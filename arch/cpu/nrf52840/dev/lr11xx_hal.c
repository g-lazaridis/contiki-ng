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
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-HAL"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
#define LR11XX_SPI_INSTANCE     0
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LR11XX_SPI_INSTANCE);  /**< SPI instance. */
static bool spi_initialized = false;
/*---------------------------------------------------------------------------*/
static int
hal_spi_init(void)
{
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin = LR1110_SPI_CS;
  spi_config.miso_pin = LR1110_SPI_MISO;
  spi_config.mosi_pin = LR1110_SPI_MOSI;
  spi_config.sck_pin = LR1110_SPI_SCK;
  spi_config.orc = 0x00;

  return nrf_drv_spi_init(&spi, &spi_config, NULL, NULL) == NRF_SUCCESS ? 0 : -1;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{
  int ret;
  if(!spi_initialized) {
    ret = hal_spi_init();
    if(ret) {
      LOG_ERROR("lr11xx_hal_write: Failed to initialize SPI\n");
      return LR11XX_HAL_STATUS_ERROR;
    }
    spi_initialized = true;
  }
}
/*---------------------------------------------------------------------------*/