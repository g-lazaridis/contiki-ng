/**
 * \file
 *        Minimal Wio-WM1110 example, used to verify the board port and the
 *        serial console.
 * \author
 *        Gavriil Lazaridis
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "HELLO-WORLD"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND * 3);
  while(1) {
    LOG_INFO("Hello World\n");
    PROCESS_WAIT_EVENT();
    etimer_reset(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
