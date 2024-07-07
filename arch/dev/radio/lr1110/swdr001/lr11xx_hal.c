/**
 * @file lr11xx_hal.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lr11xx_hal.h"
#include "lr11xx.h"
#include "sys/critical.h"
#include "watchdog.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-HAL"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/

typedef enum {
  RADIO_SLEEP,
  RADIO_AWAKE
} radio_mode_t;

static volatile radio_mode_t radio_mode = RADIO_AWAKE;
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{

  int_master_status_t status;

  if(lr11xx_wait_busy()) {
    LOG_ERR("lr11xx_hal_write: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  status = critical_enter();

  lr11xx_spi_transfer(command, command_length, NULL, 0);

  if(command && command_length) {
    lr11xx_spi_transfer(command, command_length, NULL, 0);
  }

  if(data && data_length) {
    lr11xx_spi_transfer(data, data_length, NULL, 0);
  }

  critical_exit(status);

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_read(const void *context, const uint8_t *command, const uint16_t command_length,
                uint8_t *data, const uint16_t data_length)
{
  int_master_status_t status;
  uint8_t st1;
  uint8_t rx_data[data_length]

  if(lr11xx_wait_busy()) {
    LOG_ERR("lr11xx_hal_read: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  status = critical_enter();

  if(command && command_length) {
    lr11xx_spi_transfer(command, command_length, NULL, 0);
    if(lr11xx_wait_busy()) {
      LOG_ERR("lr11xx_hal_read: Device is busy\n");
      critical_exit(status);
      return LR11XX_HAL_STATUS_ERROR;
    }
  }

  if(data && data_length) {
    if(command && command_length) {
      // Get status byte
      lr11xx_spi_transfer(NULL, 0, &st1, 1);
    }
    lr11xx_spi_transfer(NULL, 0, data, data_length);
  }

  critical_exit(status);
  LOG_INFO("lr11xx_hal_read: Status byte = 0x%x\n", st1);
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_direct_read(const void *context, uint8_t *data, const uint16_t data_length)
{
  return lr11xx_hal_read(context, NULL, 0, data, data_length);
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_reset(const void *context)
{
  LOG_INFO("lr11xx_hal_reset: Reseting...\n");
  lr11xx_reset();
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_wakeup(const void *context)
{
  LOG_INFO("lr11xx_hal_wakeup: Waking up...\n");
  lr11xx_wake();
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_abort_blocking_cmd(const void *context)
{
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/