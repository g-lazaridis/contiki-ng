/**
 * \file
 *        Reads the hardware and firmware version of the LR1110 transceiver.
 *
 *        A minimal bring-up check for the Wio-WM1110: it exercises the SPI
 *        link and the LR11xx HAL without involving the network stack, so it
 *        is the first thing to run on a new board. Compare the reported
 *        firmware version against the images in the firmware-update example.
 * \author
 *        Gavriil Lazaridis
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lr11xx_hal.h"
#include "lr11xx_bootloader_types.h"
#include "lr11xx_bootloader.h"
#include "netstack.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-VERSION"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(version_process, "LR11xx version");
AUTOSTART_PROCESSES(&version_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(version_process, ev, data)
{
  static struct etimer et;
  static lr11xx_bootloader_version_t version;
  lr11xx_status_t status;

  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND * 3);
  PROCESS_WAIT_EVENT();
  NETSTACK_RADIO.init();
  LOG_INFO("Reading version\n");
  status = lr11xx_bootloader_get_version(NULL, &version);
  if(status == LR11XX_STATUS_OK) {
    LOG_INFO("HW Version = %u\n", version.hw);
    LOG_INFO("FW Version = %02u.%02u\n", (uint8_t)(version.fw >> 8), (uint8_t)(version.fw));
    LOG_INFO("Type = %u\n", version.type);
  } else {
    LOG_ERR("Failed to get version\n");
  }

  while(1) {
    PROCESS_WAIT_EVENT();
    etimer_reset(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
