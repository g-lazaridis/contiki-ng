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
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-HAL"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LR11XX_SPI_INSTANCE);  /**< SPI instance. */
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_init(void)
{
  ret_code_t ret;
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

  spi_config.ss_pin = LR1110_SPI_CS;
  spi_config.miso_pin = LR1110_SPI_MISO;
  spi_config.mosi_pin = LR1110_SPI_MOSI;
  spi_config.sck_pin = LR1110_SPI_SCK;
  spi_config.orc = LR11XX_NOP;

  ret = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);

  if(ret != NRF_SUCCESS) {
    LOG_ERR("lr11xx_hal_init: Failed to initialize spi\n")
    return LR11XX_HAL_STATUS_ERROR;
  }

  /* Set busy pin as input */
  nrf_gpio_cfg_input(LR1110_BUSY, NRF_GPIO_PIN_NOPULL);

  /* Set reset pin as output */
  nrf_gpio_cfg_input(LR1110_NRESET, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_pin_set(LR1110_NRESET);

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{

  ret_code_t ret;

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
      LOG_ERR("lr11xx_hal_write: Failed to data\n");
      return LR11XX_HAL_STATUS_ERROR;
    }
  }

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/