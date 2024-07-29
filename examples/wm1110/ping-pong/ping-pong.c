/**
 * @file ping-pong.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-07-29
 *
 * @copyright Copyright (c) 2024
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rtimer-arch.h"
#include "netstack.h"
#include <string.h>
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
static uint8_t buffer_tx[PAYLOAD_LENGTH];
static uint8_t buffer_rx[PAYLOAD_LENGTH];

static const uint8_t ping_msg[PING_PONG_PREFIX_SIZE] = "PING";
static const uint8_t pong_msg[PING_PONG_PREFIX_SIZE] = "PONG";
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(ping_pong_process, ev, data)
{
  static struct etimer et;
  static uint8_t iteration;
  uint8_t packet_seen;

  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND * 3);
  NETSTACK_RADIO.init();
  NETSTACK_RADIO.on();
  buffer_tx[PING_PONG_PREFIX_SIZE] = (uint8_t)0;
  while(1) {
    RTIMER_BUSYWAIT_UNTIL_ABS((packet_seen = NETSTACK_RADIO.pending_packet()),
                              RTIMER_NOW(), RTIMER_NOW() + RTIMER_ARCH_SECOND);
    if(packet_seen) {
      LOG_INFO("Packet received: ");
      NETSTACK_RADIO.read(buffer_rx, sizeof(buffer_rx));
      if(memcmp(buffer_rx, ping_msg, PING_PONG_PREFIX_SIZE)) {
        LOG_INFO("PING!\n");
        memcpy(buffer_tx, pong_msg, PING_PONG_PREFIX_SIZE);
      } else if(memcmp(buffer_rx, pong_msg, PING_PONG_PREFIX_SIZE)) {
        memcpy(buffer_tx, ping_msg, PING_PONG_PREFIX_SIZE);
        LOG_INFO("PONG!\n");
      }
      iteration = buffer_rx[ITERATION_INDEX];
      iteration++;
      buffer_tx[ITERATION_INDEX] = (uint8_t)(iteration);
      etimer_set(&et,
                 (CLOCK_SECOND * (DELAY_PING_PONG_PACE_MS + DELAY_BEFORE_TX_MS)) / 1000);
      PROCESS_WAIT_EVENT();
      LOG_INFO("Transmitting..\n");
      NETSTACK_RADIO.send(buffer_tx, PAYLOAD_LENGTH);
    } else {
      LOG_INFO("Waiting for packet\n");
      // PROCESS_PAUSE();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
