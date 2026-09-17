/**
 * \file
 *        Replies to packets sent by the ping example, bypassing the network
 *        stack.
 * \author
 *        Gavriil Lazaridis
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
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
PROCESS(pong, "Pong");
AUTOSTART_PROCESSES(&pong);
/*---------------------------------------------------------------------------*/
#define PAYLOAD_LENGTH  255
/*---------------------------------------------------------------------------*/
static uint8_t buffer_tx[] = { 'T', 'H', 'I', 'S', ' ', 'I', 'S', ' ',
                               'A', ' ', 'P', 'O', 'N', 'G', ' ', 'T', 'E', 'S', 'T', ' ',
                               'M', 'E', 'S', 'S', 'A', 'G', 'E' };
static uint8_t buffer_rx[PAYLOAD_LENGTH];
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pong, ev, data)
{
  static int rx_len = 0;
  static rtimer_clock_t now;

  PROCESS_BEGIN();

  NETSTACK_RADIO.off();
  NETSTACK_RADIO.on();
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 0);

  while(1) {
    now = RTIMER_NOW();
    RTIMER_BUSYWAIT_UNTIL_ABS(NETSTACK_RADIO.receiving_packet(),
                              now, RTIMER_SECOND);
    if(NETSTACK_RADIO.receiving_packet()) {
      now = RTIMER_NOW();
      RTIMER_BUSYWAIT_UNTIL_ABS(!NETSTACK_RADIO.receiving_packet(),
                                now, RTIMER_SECOND);
      if(!NETSTACK_RADIO.receiving_packet() && NETSTACK_RADIO.pending_packet()) {
        rx_len = NETSTACK_RADIO.read((void *)buffer_rx, sizeof(buffer_rx));
        NETSTACK_RADIO.prepare(buffer_tx, sizeof(buffer_tx));
        NETSTACK_RADIO.transmit(sizeof(buffer_tx));
        LOG_INFO("Received packet of len = %d. Reply sent\n", rx_len);
      } else {
        LOG_INFO("Reception error.\n");
      }
      NETSTACK_RADIO.off();
      NETSTACK_RADIO.on();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
