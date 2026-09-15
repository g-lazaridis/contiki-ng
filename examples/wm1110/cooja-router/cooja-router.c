/**
 * \file
 *        TSCH-over-LoRa border router for Cooja simulations.
 * \author
 *        Gavriil Lazaridis
 */

/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rtimer-arch.h"
#include "netstack.h"
#include <string.h>
#include "tsch-const.h"
#include "tsch-types.h"

/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "BORDER-ROUTER"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(router_process, "BR");
AUTOSTART_PROCESSES(&router_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(router_process, ev, data)
{
  PROCESS_BEGIN();

  /* TSCH is started by the RPL border router service, which brings up the
   * node as PAN coordinator. Nothing else to do here. */
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
