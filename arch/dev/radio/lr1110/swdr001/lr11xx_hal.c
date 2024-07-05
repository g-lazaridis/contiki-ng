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
#include "nrf_drv_spi.h"
#include "watchdog.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-HAL"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
#define USEC_BUSY_TIMEOUT     3000000
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LR11XX_SPI_INSTANCE);  /**< SPI instance. */
typedef enum {
  RADIO_SLEEP,
  RADIO_AWAKE
} radio_mode_t;

static volatile radio_mode_t radio_mode = RADIO_AWAKE;
/*---------------------------------------------------------------------------*/
static inline int
wait_until_not_busy(void)
{
  uint16_t timeout_us = USEC_BUSY_TIMEOUT;
  while(nrf_gpio_pin_read(LR1110_BUSY_PIN) && timeout_us) {
    watchdog_periodic();
    clock_delay_usec(10);
    timeout_us -= 10;
  }

  return timeout_us ? 0 : -1;
}
/*---------------------------------------------------------------------------*/
static int
check_device_ready(void)
{
  if(radio_mode != RADIO_SLEEP) {
    return wait_until_not_busy();
  } else {
    hal_spi_init();

    /* Busy is HIGH in sleep mode, wake-up the device with a small glitch on NSS */
    hal_gpio_set_value(lr11xx_context->nss, 0);
    hal_gpio_set_value(lr11xx_context->nss, 1);
    lr11xx_hal_wait_on_busy(lr11xx_context->busy);
    radio_mode = RADIO_AWAKE;
  }
}
/*---------------------------------------------------------------------------*/
/* lr11xx_hal_status_t */
/* lr11xx_hal_init(void) */
/* { */
/*   ret_code_t ret; */
/*   nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG; */

/*   spi_config.ss_pin = LR1110_SPI_CS; */
/*   spi_config.miso_pin = LR1110_SPI_MISO; */
/*   spi_config.mosi_pin = LR1110_SPI_MOSI; */
/*   spi_config.sck_pin = LR1110_SPI_SCK; */
/*   spi_config.orc = LR11XX_NOP; */

/*   ret = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL); */

/*   if(ret != NRF_SUCCESS) { */
/*     LOG_ERR("lr11xx_hal_init: Failed to initialize spi\n"); */
/*     return LR11XX_HAL_STATUS_ERROR; */
/*   } */

/*   / * Set busy pin as input * / */
/*   nrf_gpio_cfg_input(LR1110_BUSY, NRF_GPIO_PIN_NOPULL); */

/*   / * Set reset pin as output, cs pin is configured from te spi driver * / */
/*   nrf_gpio_cfg_output(LR1110_NRESET); */
/*   nrf_gpio_pin_set(LR1110_NRESET); */

/*   LOG_INFO("lr11xx_hal_init: lrx11xx hal initialized successfully\n"); */

/*   return LR11XX_HAL_STATUS_OK; */
/* } */
/* / *---------------------------------------------------------------------------* / */
lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{

  ret_code_t ret;

  if(wait_until_not_busy()) {
    LOG_ERR("lr11xx_hal_write: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  if(command && command_length) {
    ret = nrf_drv_spi_transfer(&spi, command, command_length, NULL, 0);
    if(ret != NRF_SUCCESS) {
      LOG_ERR("lr11xx_hal_write: Failed to write command part\n");
      return LR11XX_HAL_STATUS_ERROR;
    }
  }

  if(data && data_length) {
    ret = nrf_drv_spi_transfer(&spi, data, data_length, NULL, 0);
    if(ret != NRF_SUCCESS) {
      LOG_ERR("lr11xx_hal_write: Failed to write data\n");
      return LR11XX_HAL_STATUS_ERROR;
    }
  }

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_read(const void *context, const uint8_t *command, const uint16_t command_length,
                uint8_t *data, const uint16_t data_length)
{
  ret_code_t ret;

  if(wait_until_not_busy()) {
    LOG_ERR("lr11xx_hal_read: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  if(command && command_length) {
    ret = nrf_drv_spi_transfer(&spi, command, command_length, NULL, 0);
    if(ret != NRF_SUCCESS) {
      LOG_ERR("lr11xx_hal_read: Failed to write command part\n");
      return LR11XX_HAL_STATUS_ERROR;
    }
    if(wait_until_not_busy()) {
      LOG_ERR("lr11xx_hal_read: Device is busy\n");
      return LR11XX_HAL_STATUS_ERROR;
    }
  }

  if(data && data_length) {
    ret = nrf_drv_spi_transfer(&spi, NULL, 0, data, data_length);
    if(ret != NRF_SUCCESS) {
      LOG_ERR("lr11xx_hal_read: Failed to read data\n");
      return LR11XX_HAL_STATUS_ERROR;
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
  nrf_gpio_pin_clear(LR1110_NRESET_PIN);
  clock_delay_usec(200);
  nrf_gpio_pin_set(LR1110_NRESET_PIN);
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_wakeup(const void *context)
{
  LOG_INFO("lr11xx_hal_wakeup: Waking up...\n");
  nrf_gpio_pin_clear(LR1110_SPI_CS_PIN);
  clock_delay_usec(200);
  nrf_gpio_pin_set(LR1110_SPI_CS_PIN);
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_abort_blocking_cmd(const void *context)
{
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/