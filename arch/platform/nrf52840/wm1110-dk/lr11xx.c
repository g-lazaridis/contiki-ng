// /**
//  * @file lr11xx.c
//  * @author your name (you@domain.com)
//  * @brief
//  * @version 0.1
//  * @date 2024-07-06
//  *
//  * @copyright Copyright (c) 2024
//  *
//  */
// /*---------------------------------------------------------------------------*/
#include "nrf_drv_spi.h"
#include "lr11xx_hal.h"
#include "lr11xx_bootloader.h"
#include "lr11xx_crypto_engine.h"
#include "lr11xx.h"
#include "nrf_gpio.h"
#include "contiki.h"
/*---------------------------------------------------------------------------*/
#include "sys/log.h"
#define LOG_MODULE "LR11XX"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LR11XX_SPI_INSTANCE);  /**< SPI instance. */
/*---------------------------------------------------------------------------*/
static uint8_t
spi_in_out(const uint16_t out_data)
{
  uint8_t tv = 0, rv = 0;

  tv = (uint8_t)(out_data & 0xFF);

  nrf_drv_spi_xfer_desc_t xfer_desc = { .p_tx_buffer = (uint8_t *)(&tv), .tx_length = 1,
                                        .p_tx_buffer = &rv, .rx_length = 1 };
  // xfer_desc = NRF_DRV_SPI_XFER_TRX((uint8_t *)(&tv), 1, (uint8_t *)(&rv), 1);
  nrf_drv_spi_xfer(&spi, &xfer_desc, NRF_DRV_SPI_FLAG_NO_XFER_EVT_HANDLER);
  // nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX((uint8_t *)(&tv), 1, (uint8_t *)(&rv), 1);
  // nrfx_spim_xfer(&spi, &xfer_desc, NRFX_SPIM_FLAG_NO_XFER_EVT_HANDLER);

  return rv;
}
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_transfer(const void *out, uint16_t out_len, void *in, uint16_t in_len)
{
  nrf_drv_spi_transfer(&spi, out, out_len, in, in_len);
}
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_read(uint8_t *data, const uint16_t data_length)
{
  for(int i = 0; i < data_length; i++) {
    data[i] = spi_in_out(LR11XX_NOP);
  }
}
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_uninit(void)
{
  nrf_drv_spi_uninit(&spi);
}
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_init(void)
{
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

  spi_config.ss_pin = LR1110_SPI_CS_PIN;
  spi_config.miso_pin = LR1110_SPI_MISO_PIN;
  spi_config.mosi_pin = LR1110_SPI_MOSI_PIN;
  spi_config.sck_pin = LR1110_SPI_SCK_PIN;
  spi_config.orc = LR11XX_NOP;

  nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
}
/*---------------------------------------------------------------------------*/
void
lr11xx_enter_bootloader_mode(void)
{
  nrf_gpio_cfg_output(LR1110_BUSY_PIN);
  nrf_gpio_pin_clear(LR1110_BUSY_PIN);
  lr11xx_reset();

  clock_wait(CLOCK_SECOND / 2);

  /* Set-up GPIOS*/
  nrf_gpio_pin_set(LR1110_SPI_CS_PIN);
  nrf_gpio_cfg_input(LR1110_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
  lr11xx_wait_busy();
}
/*---------------------------------------------------------------------------*/
int
lr11xx_firmware_update(const uint32_t *fw_image, uint32_t image_size)
{
  static bool image_valid;

  LOG_INFO("Validateing fw image...\n");
  lr11xx_crypto_check_encrypted_firmware_image_full(NULL, 0, fw_image, image_size);
  lr11xx_crypto_get_check_encrypted_firmware_image_result(NULL, &image_valid);

  if(!image_valid) {
    LOG_ERROR("Image validation failed\n");
  }
}
/*---------------------------------------------------------------------------*/
void
lr11xx_init(nrfx_gpiote_evt_handler_t gpio_irq_handler)
{
  nrf_drv_gpiote_in_config_t irq_config;

  /* re-power up LR1110 */
  nrf_gpio_cfg_output(LR1110_NRESET_PIN);
  nrf_gpio_pin_clear(LR1110_NRESET_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_CS_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_CS_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_MISO_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_MISO_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_MOSI_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_MOSI_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_SCK_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_SCK_PIN);

  nrf_gpio_cfg_output(LR1110_BUSY_PIN);
  nrf_gpio_pin_clear(LR1110_BUSY_PIN);

  clock_wait(CLOCK_SECOND / 2);

  /* Set-up GPIOS*/
  nrf_gpio_pin_set(LR1110_SPI_CS_PIN);
  nrf_gpio_cfg_input(LR1110_BUSY_PIN, NRF_GPIO_PIN_NOPULL);

  nrf_drv_gpiote_init();
  irq_config.hi_accuracy = false;
  irq_config.is_watcher = false;
  irq_config.skip_gpio_setup = false;
  irq_config.pull = NRF_GPIO_PIN_PULLDOWN;
  irq_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

  nrf_drv_gpiote_in_init(LR1110_IRQ_PIN, &irq_config, gpio_irq_handler);

  nrf_gpio_pin_set(LR1110_NRESET_PIN);
  lr11xx_spi_init();
}
/*---------------------------------------------------------------------------*/