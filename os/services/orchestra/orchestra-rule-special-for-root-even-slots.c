/**
 * \file
 *        Orchestra rule: a slotframe for unicasts between the root node and
 *        its children.
 *
 *        The root can receive from every child in all even timeslots, and a
 *        child can receive from the root in one specific odd timeslot. A
 *        lower-duty-cycle alternative to special_for_root_always_on, which
 *        leaves the odd slots free for the other rules.
 * \author
 *        Gavriil Lazaridis
 */

#include "contiki.h"
#include "orchestra.h"
#include "tsch.h"

#include "sys/log.h"
#define LOG_MODULE "Orchestra-root-even-slots"
#define LOG_LEVEL  LOG_LEVEL_NONE

/* True when the root node is also our time source */
static uint8_t root_is_ts = 0;
static uint16_t slotframe_handle = 0;
static struct tsch_slotframe *sf_root;
static linkaddr_t root_addr;
static linkaddr_t time_source_addr;
/*---------------------------------------------------------------------------*/
static uint16_t
get_root_to_node_timeslot(const linkaddr_t *addr)
{
  uint16_t timeslot;

  if(addr != NULL && ORCHESTRA_ROOT_PERIOD > 0) {
    timeslot = (ORCHESTRA_LINKADDR_HASH(addr) % ORCHESTRA_ROOT_PERIOD);
    // Tx timeslot must be an even number
    if(!(timeslot % 2)) {
      timeslot++;
      if(timeslot == ORCHESTRA_ROOT_PERIOD) {
        timeslot = 1;
      }
    }
    return timeslot;
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
create_links_to_root(uint8_t is_added)
{
  uint16_t timeslot;

  if(is_added) {
    for(int i = 0; i < ORCHESTRA_ROOT_PERIOD; i += 2) {
      tsch_schedule_add_link(sf_root,
                             LINK_OPTION_SHARED | (tsch_is_coordinator ? LINK_OPTION_RX : LINK_OPTION_TX),
                             LINK_TYPE_NORMAL, &tsch_broadcast_address,
                             i, 0, 0);
    }
    if(!tsch_is_coordinator) {
      timeslot = get_root_to_node_timeslot(&linkaddr_node_addr);
      tsch_schedule_add_link(sf_root, LINK_OPTION_SHARED | LINK_OPTION_RX,
                             LINK_TYPE_NORMAL, &tsch_broadcast_address,
                             timeslot, 0, 0);
    }
  } else {
    for(int i = 0; i < ORCHESTRA_ROOT_PERIOD; i += 2) {
      tsch_schedule_remove_link_by_offsets(sf_root, i, 0);
    }
    if(!tsch_is_coordinator) {
      /* Remove also the Root-to-Node Rx link */
      timeslot = get_root_to_node_timeslot(&linkaddr_node_addr);
      tsch_schedule_remove_link_by_offsets(sf_root, timeslot, 0);
      tsch_queue_free_packets_to(&root_addr);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
neighbor_updated(const linkaddr_t *linkaddr, uint8_t is_added)
{
  uint16_t timeslot;
  LOG_INFO("Neighbour updated %u\n", is_added);

  if(tsch_is_coordinator) {
    timeslot = get_root_to_node_timeslot(linkaddr);
    if(is_added) {
      tsch_schedule_add_link(sf_root, LINK_OPTION_SHARED | LINK_OPTION_TX,
                             LINK_TYPE_NORMAL, &tsch_broadcast_address,
                             timeslot, 0, 0);
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot, uint16_t *channel_offset)
{
  /* Select data packets we have a unicast link to */
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  struct tsch_asn_t asn;

  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME
     && !linkaddr_cmp(dest, &linkaddr_null)) {

    if(tsch_is_coordinator) {
      if(slotframe != NULL) {
        *slotframe = slotframe_handle;
      }
      if(timeslot != NULL) {
        *timeslot = get_root_to_node_timeslot(dest);
      }
      /* set per-packet channel offset */
      if(channel_offset != NULL) {
        *channel_offset = get_node_channel_offset(dest);
      }
    } else if(tsch_roots_is_root(dest)) {
      if(slotframe != NULL) {
        *slotframe = slotframe_handle;
      }
      /* set per-packet channel offset */
      if(channel_offset != NULL) {
        *channel_offset = get_node_channel_offset(dest);
      }

      asn = tsch_current_asn;
      TSCH_ASN_INC(asn, 1);
      *timeslot = TSCH_ASN_MOD(asn, sf_root->size);
      if((*timeslot % 2)) {
        *timeslot = *timeslot == (ORCHESTRA_ROOT_PERIOD - 1) ? 0 : *timeslot + 1;
      }
    }

    LOG_INFO("Selected packet, ts = %u, cur ts = %u\n", *timeslot, TSCH_ASN_MOD(tsch_current_asn, sf_root->size));
    LOG_INFO(" {asn cur %02x.%08" PRIx32 " \n",
             tsch_current_asn.ms1b, tsch_current_asn.ls4b);

    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
root_node_updated(const linkaddr_t *root, uint8_t is_added)
{
  LOG_INFO("root node updated %u\n", is_added);

  if(is_added) {
    /* Currently, this rule supports only one root. Consider as root node the last
       updated node. */
    if(!linkaddr_cmp(&root_addr, root)) {
      linkaddr_copy(&root_addr, root);
      if(linkaddr_cmp(root, &linkaddr_node_addr)) {
        create_links_to_root(is_added);
      } else if(linkaddr_cmp(root, &time_source_addr)) {
        root_is_ts = 1;
        create_links_to_root(is_added);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  if(new != old) {
    const linkaddr_t *new_addr = tsch_queue_get_nbr_address(new);
    if(new_addr != NULL) {
      linkaddr_copy(&time_source_addr, new_addr);
      if(linkaddr_cmp(&root_addr, new_addr)) {
        create_links_to_root(1);
        root_is_ts = 1;
      }
    } else {
      linkaddr_copy(&time_source_addr, &linkaddr_null);
      if(root_is_ts) {
        /* New time source is NULL, which means that we lost connection with the
           timesource. Remove all links if the timesource was also the root node. */
        create_links_to_root(0);
        root_is_ts = 0;
        /* Since our timesource was also the root node, clear root address entry. */
        linkaddr_copy(&root_addr, &linkaddr_null);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  /* signal that the root rule is used */
  slotframe_handle = sf_handle;

  sf_root = tsch_schedule_add_slotframe(slotframe_handle, ORCHESTRA_ROOT_PERIOD);
  if(sf_root == NULL) {
    LOG_ERR("failed to add root slotframe\n");
  }

  linkaddr_copy(&root_addr, &linkaddr_null);
}
/*---------------------------------------------------------------------------*/
struct orchestra_rule special_for_root_even_rx_slots = {
  init,
  new_time_source,
  select_packet,
  NULL,
  NULL,
  neighbor_updated,
  root_node_updated,
  "special for root even slots",
  ORCHESTRA_ROOT_PERIOD,
};