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
#include "lr11xx.h"
#include "nrf_gpio.h"
#include "contiki.h"
/*---------------------------------------------------------------------------*/
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LR11XX_SPI_INSTANCE);  /**< SPI instance. */
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_transfer(const void *out, uint16_t out_len, void *in, uint16_t in_len)
{
  nrf_drv_spi_transfer(&spi, out, out_len, in, in_len);
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
// /*---------------------------------------------------------------------------*/
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