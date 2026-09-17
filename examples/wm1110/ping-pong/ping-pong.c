/**
 * \file
 *        Combined ping/pong radio test: whichever board hears nothing first
 *        starts pinging, and the two then alternate.
 * \author
 *        Gavriil Lazaridis
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rtimer-arch.h"
#include "netstack.h"
#include <string.h>
#include "lr11xx_system.h"
#include "lr11xx_radio.h"
#include "lr11xx_regmem.h"
#include "lr11xx-radio-conf.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "PING-PONG"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(ping_pong_process, "Ping Pong");
AUTOSTART_PROCESSES(&ping_pong_process);
/*---------------------------------------------------------------------------*/
/**
 * @brief Size of ping-pong message prefix
 *
 * Expressed in bytes
 */
#define PING_PONG_PREFIX_SIZE 4

/**
 * @brief Threshold:number of exchanges before informing user on UART that the board pair is still not synchronized
 *
 * Expressed in number of packet exchanged
 */
#define SYNC_PACKET_THRESHOLD 64

/**
 * @brief Number of exchanges are stored in the payload of the packet exchanged during this PING-PONG communication
 *        this constant indicates where in the packet the two bytes used to count are located
 *
 * Expressed in bytes
 */

#define ITERATION_INDEX (PING_PONG_PREFIX_SIZE + 1)

/**
 * @brief Duration of the wait before packet transmission to assure reception status is ready on the other side
 *
 * Expressed in milliseconds
 */
#define DELAY_BEFORE_TX_MS 20

/**
 * @brief Duration of the wait between each ping-pong activity, can be used to adjust ping-pong speed
 *
 * Expressed in milliseconds
 */
#define DELAY_PING_PONG_PACE_MS 200

#define PAYLOAD_LENGTH  7
/*---------------------------------------------------------------------------*/
static uint8_t buffer_tx[] = { 'P', 'I', 'N', 'G', 0, 0, 0 };
static uint8_t buffer_rx[PAYLOAD_LENGTH] = { 0 };

static const uint8_t ping_msg[PING_PONG_PREFIX_SIZE] = "PING";
static const uint8_t pong_msg[PING_PONG_PREFIX_SIZE] = "PONG";
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ping_pong_process, ev, data)
{
  static uint8_t iteration = 0;
  static rtimer_clock_t now;
  uint8_t packet_seen = 0;

  PROCESS_BEGIN();
  NETSTACK_RADIO.off();
  NETSTACK_RADIO.on();
  buffer_tx[PING_PONG_PREFIX_SIZE] = (uint8_t)0;
  while(1) {
    now = RTIMER_NOW();
    RTIMER_BUSYWAIT_UNTIL_ABS((packet_seen = NETSTACK_RADIO.pending_packet()),
                              now, RTIMER_SECOND);
    if(packet_seen) {
      LOG_INFO("Packet received: ");
      NETSTACK_RADIO.read(buffer_rx, sizeof(buffer_rx));
      if(memcmp(buffer_rx, ping_msg, PING_PONG_PREFIX_SIZE) == 0) {
        LOG_INFO_("PING!\n");
        memcpy(buffer_tx, pong_msg, PING_PONG_PREFIX_SIZE);
      } else if(memcmp(buffer_rx, pong_msg, PING_PONG_PREFIX_SIZE) == 0) {
        memcpy(buffer_tx, ping_msg, PING_PONG_PREFIX_SIZE);
        LOG_INFO_("PONG!\n");
      } else {
        LOG_ERR("Invalid packet received\n");
      }
      iteration = buffer_rx[ITERATION_INDEX];
      iteration++;
      buffer_tx[ITERATION_INDEX] = (uint8_t)(iteration);
      LOG_INFO("Transmitting..\n");
      NETSTACK_RADIO.send(buffer_tx, PAYLOAD_LENGTH);
    } else {
      LOG_INFO("Pinging....\n");
      buffer_tx[ITERATION_INDEX] = (uint8_t)(iteration);
      memcpy(buffer_tx, ping_msg, PING_PONG_PREFIX_SIZE);
      NETSTACK_RADIO.send(buffer_tx, PAYLOAD_LENGTH);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
