/**
 * @file lr11xx-radio.c
 * @author glazaridis (gabriel.laz@hotmail.com)
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
#include "contiki.h"
#include "dev/radio.h"
#include "sys/energest.h"
#include "lr11xx_hal.h"
#include "lr11xx_types.h"
#include "lr11xx_radio.h"
#include "lr11xx_system.h"
#include "lr11xx_system_types.h"
#include "lr11xx_regmem.h"
#include "lr11xx-radio-conf.h"
#include "lr11xx.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-LORA"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
static lr11xx_radio_mod_params_lora_t lora_mod_params = {
  .sf = LR11XX_LORA_SPREADING_FACTOR,
  .bw = LR11XX_LORA_BANDWIDTH,
  .cr = LR11XX_LORA_CODING_RATE,
  .ldro = 0    // Will be initialized in radio init
};

static const lr11xx_radio_pkt_params_lora_t lora_pkt_params = {
  .preamble_len_in_symb = LR11XX_LORA_PREAMBLE_LENGTH,
  .header_type = LR11XX_LORA_PKT_LEN_MODE,
  .pld_len_in_bytes = LR11XX_PAYLOAD_LENGTH,
  .crc = LR11XX_LORA_CRC,
  .iq = LR11XX_LORA_IQ,
};
/*---------------------------------------------------------------------------*/
static void lr11xx_irq_callback(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
/*---------------------------------------------------------------------------*/
static void
radio_params_init(void)
{
  const lr11xx_radio_conf_pa_pwr_cfg_t *pa_pwr_cfg;

  pa_pwr_cfg = lr11xx_radio_conf_get_pa_pwr_cfg(LR11XX_RF_FREQ_IN_HZ, LR11XX_TX_OUTPUT_POWER_DBM);

  if(pa_pwr_cfg == NULL) {
    LOG_ERR("Invalid target frequency or power level\n");
    return;
  }

  if(LR11XX_PACKET_TYPE != LR11XX_RADIO_PKT_TYPE_LORA) {
    LOG_ERR("Only lora packet type is supported for now\n");
    return;
  }

  /* Set-up general parameters */
  lr11xx_radio_set_pkt_type(NULL, LR11XX_PACKET_TYPE);
  lr11xx_radio_set_rf_freq(NULL, LR11XX_RF_FREQ_IN_HZ);
  lr11xx_radio_set_rssi_calibration(NULL, lr11xx_radio_conf_get_rssi_calibration_table(LR11XX_RF_FREQ_IN_HZ));
  lr11xx_radio_set_pa_cfg(NULL, &(pa_pwr_cfg->pa_config));
  lr11xx_radio_set_tx_params(NULL, pa_pwr_cfg->power, LR11XX_PA_RAMP_TIME);
  lr11xx_system_set_standby(NULL, LR11XX_STANDBY_MODE);
  lr11xx_radio_set_rx_tx_fallback_mode(NULL, LR11XX_FALLBACK_MODE);
  lr11xx_radio_cfg_rx_boosted(NULL, LR11XX_ENABLE_RX_BOOST_MODE);

  /* Set-up modulation parameters */
  lora_mod_params.ldro = lr11xx_radio_conf_compute_lora_ldro(LR11XX_LORA_SPREADING_FACTOR, LR11XX_LORA_BANDWIDTH);
  lr11xx_radio_set_lora_mod_params(NULL, &lora_mod_params);

  /* Set-up packet parameters */
  lr11xx_radio_set_lora_pkt_params(NULL, &lora_pkt_params);
  /* Set-up sync word */
  lr11xx_radio_set_lora_sync_word(NULL, LR11XX_LORA_SYNCWORD);
}
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
  /* Initialize system parameters (spi, gpio)*/
  lr11xx_init(lr11xx_irq_callback);
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  lr11xx_regmem_write_buffer8(NULL, payload, payload_len);
  radio_params_init();
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
const struct radio_driver lr11xx_radio_driver = {
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