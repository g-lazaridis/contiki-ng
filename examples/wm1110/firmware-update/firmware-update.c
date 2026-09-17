/**
 * \file
 *        Flashes a bundled firmware image onto the LR1110 transceiver.
 * \author
 *        Gavriil Lazaridis
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lr11xx_hal.h"
#include "lr11xx_bootloader_types.h"
#include "lr11xx_bootloader.h"
#include "lr1110_transceiver_0401.h"
#include "lr11xx_crypto_engine.h"
#include "lr11xx_system.h"
#include "lr11xx_system_types.h"
#include "lr11xx.h"
#include "netstack.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "FW-UPDATE"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(fw_update_process, "LR11xx firmware update");
AUTOSTART_PROCESSES(&fw_update_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(fw_update_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND * 3);
  PROCESS_WAIT_EVENT();
  /* No radio IRQ handler needed: this only drives the bootloader. */
  lr11xx_init(NULL);
  lr11xx_firmware_update(lr11xx_firmware_image, LR11XX_FIRMWARE_IMAGE_SIZE);

  while(1) {
    etimer_reset(&et);
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
