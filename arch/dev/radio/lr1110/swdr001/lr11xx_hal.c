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
static int
lr11xx_hal_check_device_ready(void)
{
  if(radio_mode != RADIO_SLEEP) {
    return lr11xx_wait_busy();
  } else {
    lr11xx_spi_init();
    lr11xx_wake();
    radio_mode = RADIO_AWAKE;
    return lr11xx_wait_busy();
  }
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{

  int_master_status_t status;

  if(lr11xx_hal_check_device_ready()) {
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

  // LR11XX_SYSTEM_SET_SLEEP_OC=0x011B opcode. In sleep mode the radio busy line is held at 1 => do not test it
  if((command[0] == 0x01) && (command[1] == 0x1B)) {
    radio_mode = RADIO_SLEEP;

    // add a incompressible delay to prevent trying to wake the radio before it is full asleep
    clock_delay_usec(500);

    lr11xx_spi_uninit();
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
  uint8_t rx_data[data_length + 1];

  if(lr11xx_hal_check_device_ready()) {
    LOG_ERR("lr11xx_hal_read: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  status = critical_enter();

  if(command && command_length) {
    lr11xx_spi_transfer(command, command_length, NULL, 0);
    if(lr11xx_hal_check_device_ready()) {
      LOG_ERR("lr11xx_hal_read: Device is busy\n");
      critical_exit(status);
      return LR11XX_HAL_STATUS_ERROR;
    }
  }

  if(data && data_length) {
    if(command && command_length) {
      lr11xx_spi_transfer(NULL, 0, &rx_data, data_length + 1);
      critical_exit(status);
      LOG_INFO("lr11xx_hal_read: Status byte = 0x%x\n", rx_data[0]);
      memcpy(data, &rx_data[1], data_length);
    } else {
      lr11xx_spi_transfer(NULL, 0, data, data_length);
      critical_exit(status);
    }
  }

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
  radio_mode = RADIO_AWAKE;
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