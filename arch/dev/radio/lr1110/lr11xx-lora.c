/**
 * @file lr11xx-lora.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-07-02
 *
 * @copyright Copyright (c) 2024
 *
 */

/*
 * Copyright (c) 2020, Toshiba BRIL
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup nrf52840
 * @{
 *
 * \addtogroup nrf52840-dev Device drivers
 * @{
 *
 * \defgroup nrf52840-rf-ieee nRF52840 IEEE mode driver
 *
 * @{
 *
 * \file
 * Implementation of the nRF52840 IEEE mode NETSTACK_RADIO driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/radio.h"
#include "sys/energest.h"
#include "lr11xx_hal.h"
#include "lr11xx_types.h"
#include "lr11xx_system.h"
#include "lr11xx_system_types.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_spi.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-LORA"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
static void lr11xx_irq_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{

  /* lr11xx_status_t status; */
  nrf_drv_gpiote_in_config_t irq_config;
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

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

  nrf_drv_gpiote_in_init(LR1110_IRQ_PIN, &irq_config, lr11xx_irq_callback);

  nrf_gpio_pin_set(LR1110_NRESET_PIN);

  spi_config.ss_pin = LR1110_SPI_CS_PIN;
  spi_config.miso_pin = LR1110_SPI_MISO_PIN;
  spi_config.mosi_pin = LR1110_SPI_MOSI_PIN;
  spi_config.sck_pin = LR1110_SPI_SCK_PIN;
  spi_config.orc = LR11XX_NOP;
  nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
read_frame(void *buf, unsigned short bufsize)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  return 0;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  /* The radio does not support h/w frame filtering based on addresses */
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver lr11xx_lora_driver = {
  init,
  prepare,
  transmit,
  send,
  read_frame,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object
};
/*---------------------------------------------------------------------------*/
static void
lr11xx_irq_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
}
/*---------------------------------------------------------------------------*/