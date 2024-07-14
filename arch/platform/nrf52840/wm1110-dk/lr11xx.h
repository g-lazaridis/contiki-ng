/**
 * @file lr11xx.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LR11XX_H_
#define LR11XX_H_

#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "clock.h"
#include "nrf_delay.h"
/*---------------------------------------------------------------------------*/
#define USEC_BUSY_TIMEOUT     3000000
/*---------------------------------------------------------------------------*/
__STATIC_INLINE
int lr11xx_wait_busy(void);

__STATIC_INLINE
void lr11xx_wake(void);

__STATIC_INLINE
void lr11xx_reset(void);

__STATIC_INLINE
void lr11xx_spi_clear_nss(void);

__STATIC_INLINE
void lr11xx_spi_set_nss(void);

__STATIC_INLINE
void lr11xx_delay_ms(uint32_t ms_delay);

void lr11xx_spi_deinit(void);

void lr11xx_spi_init(void);

void lr11xx_init(nrfx_gpiote_evt_handler_t gpio_irq_handler);

void lr11xx_enter_bootloader_mode(void);

int lr11xx_firmware_update(const uint32_t *fw_image, uint32_t image_size);

uint16_t lr11xx_spi_in_out(const uint16_t out_data);

/*---------------------------------------------------------------------------*/
__STATIC_INLINE
int
lr11xx_wait_busy(void)
{
  uint32_t timeout_us = USEC_BUSY_TIMEOUT;
  while(nrf_gpio_pin_read(LR1110_BUSY_PIN) && timeout_us) {
    watchdog_periodic();
    clock_delay_usec(10);
    timeout_us -= 10;
  }

  return timeout_us ? 0 : -1;
}
/*---------------------------------------------------------------------------*/
__STATIC_INLINE
void
lr11xx_wake(void)
{
  nrf_gpio_pin_clear(LR1110_SPI_CS_PIN);
  clock_delay_usec(5);
  nrf_gpio_pin_set(LR1110_SPI_CS_PIN);
}
/*---------------------------------------------------------------------------*/
__STATIC_INLINE
void
lr11xx_reset(void)
{
  nrf_gpio_pin_clear(LR1110_NRESET_PIN);
  nrf_delay_ms(5);
  nrf_gpio_pin_set(LR1110_NRESET_PIN);
  nrf_delay_ms(5);
}
/*---------------------------------------------------------------------------*/
__STATIC_INLINE
void
lr11xx_spi_clear_nss(void)
{
  nrf_gpio_pin_clear(LR1110_SPI_CS_PIN);
}
/*---------------------------------------------------------------------------*/
__STATIC_INLINE
void
lr11xx_spi_set_nss(void)
{
  nrf_gpio_pin_set(LR1110_SPI_CS_PIN);
}
/*---------------------------------------------------------------------------*/
__STATIC_INLINE
void
lr11xx_delay_ms(uint32_t ms_delay)
{
  nrf_delay_ms(ms_delay);
}
/*---------------------------------------------------------------------------*/
#endif /* LR11XX_H_ */