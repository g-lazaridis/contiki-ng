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
#include "lr11xx_system.h"
#include "lr11xx_system_types.h"
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
static void
print_status(lr11xx_bootloader_stat1_t *stat1,
             lr11xx_bootloader_stat2_t *stat2,
             lr11xx_bootloader_irq_mask_t *irq_status)
{
  LOG_INFO("Stat1:\n\tcommand status = %d\n\tis_interrupt_active = %d\n", stat1->command_status, stat1->is_interrupt_active);
  LOG_INFO("Stat2:\n\treset status = %d\n\tchip mode = %d\n\tis running from flash = %d\n",
           stat2->reset_status, stat2->chip_mode, stat2->is_running_from_flash);
  LOG_INFO("Irq Status = 0x%04lX\n", *irq_status);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(playground_process, ev, data)
{
  static struct etimer et;
  static lr11xx_bootloader_version_t version;
  lr11xx_bootloader_stat1_t stat1;
  lr11xx_bootloader_stat2_t stat2;
  lr11xx_bootloader_irq_mask_t irq_status;
  // static bool image_valid;
  lr11xx_status_t status;
  // lr11xx_system_errors_t errors;

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

  lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
  print_status(&stat1, &stat2, &irq_status);

  LOG_INFO("Clearing IRQs\n");
  lr11xx_system_clear_irq_status(NULL, 0xFFFF);

  lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
  print_status(&stat1, &stat2, &irq_status);

  // LOG_INFO("Validateing fw image...\n");
  // lr11xx_crypto_check_encrypted_firmware_image_full(NULL, 0, lr11xx_firmware_image, LR11XX_FIRMWARE_IMAGE_SIZE);
  // lr11xx_crypto_get_check_encrypted_firmware_image_result(NULL, &image_valid);
  // if(image_valid) {
  //   LOG_INFO("Image validation successfull!!");
  // } else {
  //   LOG_ERR("Image validation failed\n");
  // }

  // Reset and set to bootloader
  // LOG_INFO("Entering bootloader...\n");
  // lr11xx_bootloader_reboot(NULL, true);
  // //Wait for reboot
  // etimer_set(&et, CLOCK_SECOND);
  // PROCESS_WAIT_EVENT();
  // lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
  // print_status(&stat1, &stat2, &irq_status);
  // lr11xx_system_get_errors(NULL, &errors);
  // LOG_INFO("Errors=0x%02X\n", errors);

  // LOG_INFO("Resetting...\n");
  // lr11xx_hal_reset(NULL);
  // etimer_set(&et, CLOCK_SECOND);
  // PROCESS_WAIT_EVENT();
  // lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
  // print_status(&stat1, &stat2, &irq_status);
  // lr11xx_system_get_errors(NULL, &errors);
  // LOG_INFO("Errors=0x%02X\n", errors);
  // LOG_INFO("Clearing IRQs\n");
  // lr11xx_bootloader_clear_reset_status_info(NULL);
  // lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
  // print_status(&stat1, &stat2, &irq_status);
  // LOG_INFO("Entering bootloader...\n");
  // lr11xx_bootloader_reboot(NULL, true);
  // etimer_set(&et, CLOCK_SECOND);
  // PROCESS_WAIT_EVENT();
  // lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
  // print_status(&stat1, &stat2, &irq_status);

  //Check if in bootloader state
  // lr11xx_bootloader_get_version(NULL, &version);
  // if(version.type == 0xdf) {
  //   LOG_INFO("Bootloader state entered!!\n");
  // } else {
  //   LOG_ERR("Failed to enter bootloader state\n");
  // }
  // LOG_INFO("HW Version = %u\n", version.hw);
  // LOG_INFO("FW Version = %02u.%02u\n", (uint8_t)(version.fw >> 8), (uint8_t)(version.fw));
  // LOG_INFO("Type = %u\n", version.type);

  while(1) {
    etimer_reset(&et);
    PROCESS_WAIT_EVENT();
    // lr11xx_bootloader_get_status(NULL, &stat1, &stat2, &irq_status);
    // print_status(&stat1, &stat2, &irq_status);
    //   lr11xx_bootloader_get_version(NULL, &version);
    //   LOG_INFO("HW Version = %u\n", version.hw);
    //   LOG_INFO("FW Version = %02u.%02u\n", (uint8_t)(version.fw >> 8), (uint8_t)(version.fw));
    //   LOG_INFO("Type = %u\n", version.type);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
