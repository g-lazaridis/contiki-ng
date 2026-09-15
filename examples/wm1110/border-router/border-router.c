/**
 * \file
 *        RPL border router for the Wio-WM1110, running TSCH over LoRa.
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
#include "lr11xx-tsch-timing.h"
#include "tsch-const.h"
#include "tsch-types.h"

/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "BORDER-ROUTER"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(router_process, "Border router");
AUTOSTART_PROCESSES(&router_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(router_process, ev, data)
{
  static struct etimer et;
  static int radio_last_rssi;

  PROCESS_BEGIN();

  /* TSCH is started by the RPL border router service, which brings up the
   * node as PAN coordinator. */
  while(1) {
    etimer_set(&et, CLOCK_SECOND * 60);
    NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &radio_last_rssi);
    LOG_INFO("Last RSSI=%d\n", radio_last_rssi);
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
