/**
 * \file
 *        Transmits a packet and waits for a reply, bypassing the network stack.
 *
 *        Used together with the pong example to exercise the LR11xx radio
 *        driver directly.
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
#define LOG_MODULE "PING"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(ping_process, "Ping");
AUTOSTART_PROCESSES(&ping_process);
/*---------------------------------------------------------------------------*/
#define PAYLOAD_LENGTH  255
/*---------------------------------------------------------------------------*/
static uint8_t buffer_tx[] = { 'T', 'H', 'I', 'S', ' ', 'I', 'S', ' ',
                               'A', ' ', 'P', 'I', 'N', 'G', ' ', 'T', 'E', 'S', 'T', ' ',
                               'M', 'E', 'S', 'S', 'A', 'G', 'E' };
static uint8_t buffer_rx[PAYLOAD_LENGTH];
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ping_process, ev, data)
{
  static int rx_len = 0;
  static struct etimer et;
  static rtimer_clock_t now;

  PROCESS_BEGIN();

  NETSTACK_RADIO.off();

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT();
    NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 0);
    NETSTACK_RADIO.on();
    NETSTACK_RADIO.prepare(buffer_tx, sizeof(buffer_tx));
    etimer_set(&et, CLOCK_SECOND / 20);
    PROCESS_WAIT_EVENT();
    NETSTACK_RADIO.transmit(sizeof(buffer_tx));
    etimer_set(&et, CLOCK_SECOND / 10);
    PROCESS_YIELD_UNTIL(etimer_expired(&et));
    now = RTIMER_NOW();
    RTIMER_BUSYWAIT_UNTIL_ABS(NETSTACK_RADIO.receiving_packet(),
                              now, TSCH_PACKET_DURATION(45));
    if(!NETSTACK_RADIO.receiving_packet() && NETSTACK_RADIO.pending_packet()) {
      now = RTIMER_NOW();
      RTIMER_BUSYWAIT_UNTIL_ABS(!NETSTACK_RADIO.receiving_packet(),
                                now, TSCH_PACKET_DURATION(45));
      rx_len = NETSTACK_RADIO.read((void *)buffer_rx, sizeof(buffer_rx));
      LOG_INFO("Received response of len = %d\n", rx_len);
    } else {
      LOG_INFO("Not Receiving!!\n");
    }
    NETSTACK_RADIO.off();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
