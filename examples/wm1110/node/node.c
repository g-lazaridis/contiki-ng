/**
 * \file
 *        TSCH-over-LoRa leaf node for the Wio-WM1110.
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
#include "lora-timings.h"
#include "lr11xx-tsch-timing.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "NODE"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Node");
AUTOSTART_PROCESSES(&node_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer t;
  clock_time_t next_interval;
  static bool connected = 0;

  PROCESS_BEGIN();

  /* Give the platform a moment to settle before joining the PAN. */
  next_interval = 20;
  etimer_set(&t, next_interval);
  PROCESS_WAIT_EVENT();
  NETSTACK_MAC.on();

  do {
    etimer_set(&t, next_interval);
    PROCESS_WAIT_EVENT();

    next_interval = 20;

    if(ev == PROCESS_EVENT_TIMER) {
      if(!tsch_is_associated || !NETSTACK_ROUTING.node_is_reachable()) {
        if(connected) {
          LOG_WARN("LoWPAN: lost connection, exiting\n");
        }
        connected = false;
      } else if(!connected) {
        LOG_INFO("LoWPAN: We are connected and reachable\n");
        connected = true;
      }
    }
  } while(1);
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
