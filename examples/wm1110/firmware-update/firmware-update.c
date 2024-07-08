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
#include "lr11xx_bootloader_types.h"
#include "lr11xx_bootloader.h"
#include "lr1110_transceiver_0401.h"
#include "lr11xx_crypto_engine.h"
#include "netstack.h"
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
  static lr11xx_bootloader_version_t version;
  static bool image_valid;
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

  LOG_INFO("Validateing fw image...\n");
  lr11xx_crypto_check_encrypted_firmware_image_full(NULL, 0, lr11xx_firmware_image, LR11XX_FIRMWARE_IMAGE_SIZE);
  lr11xx_crypto_get_check_encrypted_firmware_image_result(NULL, &image_valid);
  if(image_valid) {
    LOG_INFO("Image validation successfull!!");
  } else {
    LOG_ERR("Image validation failed\n");
  }
  // //Reset and set to bootloader
  // lr11xx_bootloader_reboot(NULL, true);
  // //Wait for reboot
  // etimer_set(&et, CLOCK_SECOND * 1);
  // PROCESS_WAIT_EVENT();
  // //Check if in bootloader state
  // lr11xx_bootloader_get_version(NULL, &version);
  // if(version.type == 0xdf) {
  //   LOG_INFO("Bootloader state entered!!\n");
  // } else {
  //   LOG_ERR("Failed to enter bootloader state\n");
  // }

  while(1) {
    PROCESS_WAIT_EVENT();
    etimer_reset(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
