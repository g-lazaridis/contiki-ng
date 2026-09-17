/**
 * \file
 *        Orchestra rule: a slotframe of size 1 for unicasts from the root's
 *        neighbours to the root.
 *
 *        Because the slotframe holds a single slot, the root is available to
 *        receive from its neighbours in every timeslot, unless a transmission
 *        from another slotframe is scheduled for that slot. Neighbours get an
 *        immediate upward transmit opportunity instead of waiting for a
 *        dedicated slot in a longer slotframe.
 *
 *        Intended to be paired with unicast_per_non_root_neighbor, which
 *        leaves traffic addressed to the root to this rule.
 * \author
 *        Gavriil Lazaridis
 */

#include "contiki.h"
#include "orchestra.h"
#include "tsch.h"

#include "sys/log.h"
#define LOG_MODULE "Orchestra-root-always-on"
#define LOG_LEVEL  LOG_LEVEL_NONE

/* True when the root node is also our time source */
static uint8_t root_is_ts = 0;
static uint16_t slotframe_handle = 0;
static struct tsch_slotframe *sf;
static linkaddr_t root_addr;
static linkaddr_t time_source_addr;
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
  if(is_added) {
    tsch_schedule_add_link(sf,
                           LINK_OPTION_SHARED | (tsch_is_coordinator ? LINK_OPTION_RX : LINK_OPTION_TX),
                           LINK_TYPE_NORMAL, &tsch_broadcast_address,
                           0, 0, 0);
  } else {
    tsch_schedule_remove_link_by_offsets(sf, 0, 0);
  }
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot, uint16_t *channel_offset)
{
  /* Select data packets we have a unicast link to */
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  if(!tsch_is_coordinator) {
    if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME
       && !linkaddr_cmp(dest, &linkaddr_null)
       && linkaddr_cmp(dest, &root_addr)) {

      if(slotframe != NULL) {
        *slotframe = slotframe_handle;
      }
      if(timeslot != NULL) {
        *timeslot = 0;
      }
      if(channel_offset != NULL) {
        *channel_offset = get_node_channel_offset(dest);
      }
      return 1;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
root_node_updated(const linkaddr_t *root, uint8_t is_added)
{
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
  } else if(linkaddr_cmp(&root_addr, root)) {
    linkaddr_copy(&root_addr, &linkaddr_null);
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

  sf = tsch_schedule_add_slotframe(slotframe_handle, 1);
  if(sf == NULL) {
    LOG_ERR("failed to add root slotframe\n");
  }

  linkaddr_copy(&root_addr, &linkaddr_null);
}
/*---------------------------------------------------------------------------*/
struct orchestra_rule special_for_root_always_on = {
  init,
  new_time_source,
  select_packet,
  NULL,
  NULL,
  NULL,
  root_node_updated,
  "special for root always on",
  ORCHESTRA_ROOT_PERIOD,
};