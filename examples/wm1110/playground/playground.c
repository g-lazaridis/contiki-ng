/**
 * @file hello-world.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-27
 *
 * @copyright Copyright (c) 2024
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lr11xx_hal.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "HELLO-WORLD"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(playground_process, "Playground");
AUTOSTART_PROCESSES(&playground_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(playground_process, ev, data)
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
