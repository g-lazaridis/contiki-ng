/**
 * \file
 *        Orchestra rule: a slotframe for unicasts between nodes other than the
 *        root.
 *
 *        A variant of the RPL non-storing unicast rule that declines to select
 *        packets addressed to the root, leaving those to the dedicated root
 *        rule rather than shadowing it. Uses ORCHESTRA_UNICAST_PERIOD for the
 *        slotframe length. When RULE_USED_FROM_TSCH_COORDINATOR is enabled,
 *        the rule is also used for unicasts from the root to its neighbours.
 * \author
 *        Gavriil Lazaridis
 */

#include "contiki.h"
#include "orchestra.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/packetbuf.h"

static uint16_t slotframe_handle = 0;
static struct tsch_slotframe *sf_unicast;
static linkaddr_t root_addr;
static linkaddr_t time_source_addr;

#define RULE_USED_FROM_TSCH_COORDINATOR   1

#include "sys/log.h"
#define LOG_MODULE "Orchestra-non-root-unicast"
#define LOG_LEVEL  LOG_LEVEL_NONE
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_timeslot(const linkaddr_t *addr)
{
  if(addr != NULL && ORCHESTRA_UNICAST_PERIOD > 0) {
    return ORCHESTRA_LINKADDR_HASH(addr) % ORCHESTRA_UNICAST_PERIOD;
  } else {
    return 0xffff;
  }
}
/*---------------------------------------------------------------------------*/
static uint16_t
get_node_channel_offset(const linkaddr_t *addr)
{
  if(addr != NULL && ORCHESTRA_UNICAST_MAX_CHANNEL_OFFSET >= ORCHESTRA_UNICAST_MIN_CHANNEL_OFFSET) {
    return ORCHESTRA_LINKADDR_HASH(addr) % (ORCHESTRA_UNICAST_MAX_CHANNEL_OFFSET - ORCHESTRA_UNICAST_MIN_CHANNEL_OFFSET + 1)
           + ORCHESTRA_UNICAST_MIN_CHANNEL_OFFSET;
  } else {
    return 0xffff;
  }
}
/*---------------------------------------------------------------------------*/
static void
add_uc_link(const linkaddr_t *linkaddr)
{
  if(linkaddr != NULL) {
    uint16_t timeslot = get_node_timeslot(linkaddr);

    /* Add a Tx link to the neighbor; do not replace any existing links
     * at that cell. The channel offset here does not matter:
     * select_packet() always sets the right channel offset per packet. */
    tsch_schedule_add_link(sf_unicast,
                           LINK_OPTION_SHARED | LINK_OPTION_TX,
                           LINK_TYPE_NORMAL, &tsch_broadcast_address,
                           timeslot, 0, 0);
  }
}
/*---------------------------------------------------------------------------*/
static void
remove_uc_link(const linkaddr_t *linkaddr)
{
  if(linkaddr != NULL) {
    uint16_t timeslot = get_node_timeslot(linkaddr);
    tsch_schedule_remove_link_by_offsets(sf_unicast, timeslot, 0);
    tsch_queue_free_packets_to(linkaddr);
  }
}
/*---------------------------------------------------------------------------*/
static void
neighbor_updated(const linkaddr_t *linkaddr, uint8_t is_added)
{
  LOG_INFO("Neighbour updated %u\n", is_added);

#if !RULE_USED_FROM_TSCH_COORDINATOR
  if(!tsch_is_coordinator) {
#endif
  if(!linkaddr_cmp(&root_addr, linkaddr)) {
    if(is_added) {
      add_uc_link(linkaddr);
    } else {
      remove_uc_link(linkaddr);
    }
  }
#if !RULE_USED_FROM_TSCH_COORDINATOR
}
#endif
}
/*---------------------------------------------------------------------------*/
static void
root_updated(const linkaddr_t *root, uint8_t is_added)
{
  uint16_t rx_timeslot;

  LOG_INFO("Root updated %u\n", is_added);
  if(is_added) {
    /* Currently, this rule supports only one root. Consider as root node the last
       updated node. */
    if(!linkaddr_cmp(&root_addr, root)) {
      /* Update root address */
      linkaddr_copy(&root_addr, root);
      if(linkaddr_cmp(&root_addr, &linkaddr_node_addr)) {
        /* This is the root node. This rule is not used at root node,
           so remove the Rx timeslot created during the rule's initialization. */
        rx_timeslot = get_node_timeslot(&linkaddr_node_addr);
        tsch_schedule_remove_link_by_offsets(sf_unicast, rx_timeslot, 1);
      } else if(linkaddr_cmp(root, &time_source_addr)) {
        /* Remove links created during timesource update */
        remove_uc_link(root);
      }
    }
  } else {
    if(linkaddr_cmp(&root_addr, root)) {
      linkaddr_copy(&root_addr, &linkaddr_null);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot, uint16_t *channel_offset)
{
  /* Select data packets we have a unicast link to */
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  LOG_INFO("To select packet\n");
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME
#if !RULE_USED_FROM_TSCH_COORDINATOR
     && !tsch_is_coordinator
#endif
     && !linkaddr_cmp(dest, &linkaddr_null)
     && !linkaddr_cmp(dest, &root_addr)) {

    LOG_INFO("selected packet\n");
    if(slotframe != NULL) {
      *slotframe = slotframe_handle;
    }
    if(timeslot != NULL) {
      *timeslot = get_node_timeslot(dest);
    }
    /* set per-packet channel offset */
    if(channel_offset != NULL) {
      *channel_offset = get_node_channel_offset(dest);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  if(!tsch_is_coordinator) {
    if(new != old) {
      const linkaddr_t *old_addr = tsch_queue_get_nbr_address(old);
      const linkaddr_t *new_addr = tsch_queue_get_nbr_address(new);
      remove_uc_link(old_addr);
      add_uc_link(new_addr);
      if(new_addr != NULL) {
        linkaddr_copy(&time_source_addr, new_addr);
      } else {
        linkaddr_copy(&time_source_addr, &linkaddr_null);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  uint16_t rx_timeslot;
  linkaddr_t *local_addr = &linkaddr_node_addr;

  slotframe_handle = sf_handle;
  /* Slotframe for unicast transmissions */
  sf_unicast = tsch_schedule_add_slotframe(slotframe_handle, ORCHESTRA_UNICAST_PERIOD);
  rx_timeslot = get_node_timeslot(local_addr);
  /* Add a Rx link at our own timeslot. */
  tsch_schedule_add_link(sf_unicast,
                         LINK_OPTION_RX,
                         LINK_TYPE_NORMAL, &tsch_broadcast_address,
                         rx_timeslot, get_node_channel_offset(local_addr), 1);

  linkaddr_copy(&root_addr, &linkaddr_null);
}
/*---------------------------------------------------------------------------*/
struct orchestra_rule unicast_per_non_root_neighbor = {
  init,
  new_time_source,
  select_packet,
  NULL,
  NULL,
  neighbor_updated,
  root_updated,
  "unicast per non-root neighbor",
  ORCHESTRA_UNICAST_PERIOD,
};
