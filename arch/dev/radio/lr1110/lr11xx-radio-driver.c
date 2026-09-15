/**
 * \file
 *        Contiki-NG radio driver for the Semtech LR11xx transceiver.
 * \author
 *        Gavriil Lazaridis
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
#include "netstack.h"
#include "dev/radio.h"
#include "sys/energest.h"
#include "net/packetbuf.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"
#include "lr11xx_hal.h"
#include "lr11xx_types.h"
#include "lr11xx_radio.h"
#include "lr11xx_system.h"
#include "lr11xx_system_types.h"
#include "lr11xx_regmem.h"
#include "lr11xx-radio-conf.h"
#include "lr11xx_radio_timings.h"
#if MAC_CONF_WITH_TSCH
#include "lr11xx-tsch-timing.h"
#include "tsch-const.h"
#endif
#include "lr11xx.h"
#include "gpio-hal-arch.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
// #include "tsch-log.h"
#define LOG_MODULE "LR11XX-RADIO"
#define LOG_LEVEL LOG_LEVEL_NONE
/*---------------------------------------------------------------------------*/
#define IRQ_MASK (LR11XX_SYSTEM_IRQ_PREAMBLE_DETECTED)
/*---------------------------------------------------------------------------*/
#define SFD_TIMER NRF_TIMER0
/*---------------------------------------------------------------------------*/
#define SFD_TIMER_FREQUENCY NRF_TIMER_FREQ_62500Hz
#define SFD_TIMER_TICKS_SECOND (16000000 >> NRF_TIMER_FREQ_62500Hz)
/* (Micro seconds * 100) per tick for the SFD timer */
static const uint32_t sfd_timer_tick_us_100 = 100000000 / SFD_TIMER_TICKS_SECOND;
/* (Micro seconds * 100) per tick for the rtimer */
static const uint32_t rtimer_tick_us_100 = 100000000 / RTIMER_ARCH_SECOND;
/* Holds the rtc rtimer value on radio on. Used for converting
   the NRF_TIMER0 value to the corresponding value of the RTC rtimer.*/
rtimer_clock_t rtimer_ts = 0;
/*---------------------------------------------------------------------------*/
uint32_t pending_channel_id, cur_channel_id = RADIO_CHANNEL_0;
int tx_output_power = LR11XX_TX_OUTPUT_POWER_DBM;
int last_rssi = 1, last_lqi = 1, last_packet_len;
rtimer_clock_t last_rx_ts;
uint8_t last_rx_ts_is_calculated = 0;
/*---------------------------------------------------------------------------*/
static lr11xx_radio_mod_params_lora_t lora_mod_params = {
  .sf = LR11XX_LORA_SPREADING_FACTOR,
  .bw = LR11XX_LORA_BANDWIDTH,
  .cr = LR11XX_LORA_CODING_RATE,
  .ldro = 0    // Will be initialized in radio init
};

static lr11xx_radio_pkt_params_lora_t lora_pkt_params = {
  .preamble_len_in_symb = LR11XX_LORA_PREAMBLE_LENGTH,
  .header_type = LR11XX_LORA_PKT_LEN_MODE,
  .pld_len_in_bytes = 0,
  .crc = LR11XX_LORA_CRC,
  .iq = LR11XX_LORA_IQ,
};

/*---------------------------------------------------------------------------*/
static volatile uint8_t pack_pending = 0;
static volatile uint8_t pack_receiving = 0;
static uint8_t radio_off = 1;
/* Signal propagation delay */
static uint32_t rx_signal_delay;
/*---------------------------------------------------------------------------*/
static void
set_freq_and_tx_power(uint32_t ch_id, int tx_power)
{
  const lr11xx_radio_conf_pa_pwr_cfg_t *pa_pwr_cfg;
  uint32_t rf_freq = get_channel_frequency(LR11XX_RF_FREQ_IN_HZ, ch_id,
                                           LR11XX_LORA_CHANNEL_GAP, LR11XX_LORA_BANDWIDTH);

  LOG_DBG("set_freq_and_tx_power: rf_freq = %lu\n", rf_freq);
  pa_pwr_cfg = lr11xx_radio_conf_get_pa_pwr_cfg(rf_freq, tx_output_power);
  if(pa_pwr_cfg == NULL) {
    LOG_ERR("Invalid target frequency or power level\n");
    return;
  }

  lr11xx_radio_set_rf_freq(NULL, rf_freq);
  lr11xx_radio_set_rssi_calibration(NULL, lr11xx_radio_conf_get_rssi_calibration_table(rf_freq));
  lr11xx_radio_set_pa_cfg(NULL, &(pa_pwr_cfg->pa_config));
  lr11xx_radio_set_tx_params(NULL, pa_pwr_cfg->power, LR11XX_PA_RAMP_TIME);
}
/*---------------------------------------------------------------------------*/
static void
radio_params_init(void)
{

  if(LR11XX_PACKET_TYPE != LR11XX_RADIO_PKT_TYPE_LORA) {
    LOG_ERR("Only lora packet type is supported for now\n");
    return;
  }

  set_freq_and_tx_power(cur_channel_id, tx_output_power);

  /* Set-up general parameters */
  lr11xx_radio_set_pkt_type(NULL, LR11XX_PACKET_TYPE);
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

  rx_signal_delay =
    US_TO_RTIMERTICKS(lr11xx_radio_timings_get_delay_between_last_bit_sent_and_rx_done_in_us(&lora_mod_params));
}
/*---------------------------------------------------------------------------*/
static void
enter_rx(void)
{
  // Accept all payload lengths..
  if(lora_pkt_params.pld_len_in_bytes) {
    lora_pkt_params.pld_len_in_bytes = 0;
    lr11xx_radio_set_lora_pkt_params(NULL, &lora_pkt_params);
  }
  LOG_INFO("Enter Rx\n");
  lr11xx_regmem_clear_rxbuffer(NULL);
  lr11xx_radio_set_rx_with_timeout_in_rtc_step(NULL, 0xFFFFFF);
}
/*---------------------------------------------------------------------------*/
static void
rx_timestamp_calc(rtimer_clock_t *rx_ts)
{
  rtimer_clock_t rtimer_ts;
  uint64_t d;

  nrf_timer_task_trigger(SFD_TIMER, NRF_TIMER_TASK_CAPTURE1);
  rtimer_ts = RTIMER_NOW();
  *rx_ts = nrf_timer_cc_read(SFD_TIMER, NRF_TIMER_CC_CHANNEL1);
  //Need to convert the above values in rtimer rtc ticks
  d = ((uint64_t)*rx_ts * (uint64_t)sfd_timer_tick_us_100) / rtimer_tick_us_100;
  *rx_ts = RTIMER_CLOCK_DIFF(rtimer_ts, d);

  /* Remove the signal propagation delay from the timestamp */
  *rx_ts = RTIMER_CLOCK_DIFF(*rx_ts, rx_signal_delay);
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  if(!radio_off) {
    return 0;
  }

  nrf_timer_task_trigger(SFD_TIMER, NRF_TIMER_TASK_START);

  LOG_DBG("Radio on\n");
  radio_off = 0;
  //Apply channel given channel id
  if(pending_channel_id != cur_channel_id) {
    cur_channel_id = pending_channel_id;
    set_freq_and_tx_power(cur_channel_id, tx_output_power);
  }
  enter_rx();
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  lr11xx_system_sleep_cfg_t radio_sleep_cfg;

  LOG_DBG("Radio off\n");
  radio_sleep_cfg.is_warm_start = 1;
  radio_sleep_cfg.is_rtc_timeout = 1;

  lr11xx_system_clear_irq_status(NULL, LR11XX_SYSTEM_IRQ_ALL_MASK);
  lr11xx_system_set_sleep(NULL, radio_sleep_cfg, 0xffffffff);
  pack_receiving = 0;

  /* Calculate timestamp if needed, before turning off */
  if(pack_pending && !last_rx_ts_is_calculated) {
    rx_timestamp_calc(&last_rx_ts);
    last_rx_ts_is_calculated = 1;
  }
  /* Stop Timer*/
  nrf_timer_task_trigger(SFD_TIMER, NRF_TIMER_TASK_SHUTDOWN);

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  radio_off = 1;

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
  nrfx_gpiote_in_config_t config_in = NRFX_GPIOTE_RAW_CONFIG_IN_SENSE_LOTOHI(true);

  /* Initialize system parameters (spi, gpio)*/
  lr11xx_init(NULL);
  /* Initialize radio parameters */
  radio_params_init();
  /* Set up pin IRQ conditions */
  lr11xx_system_set_dio_irq_params(NULL, IRQ_MASK, 0);
  /* NRF_TIMER0 is not enabled from rtimer module since an RTC instance is
     used as an RTIMER instead. NRF_TIMER0 used from this radio driver for timestamping
     through PPI channels so we need to configure it here. */
  nrf_timer_event_clear(SFD_TIMER, NRF_TIMER_EVENT_COMPARE0);
  nrf_timer_frequency_set(SFD_TIMER, SFD_TIMER_FREQUENCY);
  nrf_timer_bit_width_set(SFD_TIMER, NRF_TIMER_BIT_WIDTH_32);
  nrf_timer_mode_set(SFD_TIMER, NRF_TIMER_MODE_TIMER);

  /* Set up PPI for timestamping */
  config_in.pull = NRF_GPIO_PIN_PULLDOWN;
  nrf_drv_gpiote_in_init(LR1110_IRQ_PIN, &config_in, NULL);
  nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL0,
                                 (uint32_t)nrf_drv_gpiote_in_event_addr_get(LR1110_IRQ_PIN),
                                 (uint32_t)nrf_timer_task_address_get(SFD_TIMER, NRF_TIMER_TASK_CLEAR));

  nrf_ppi_channel_enable(NRF_PPI_CHANNEL0);
  nrf_drv_gpiote_in_event_enable(LR1110_IRQ_PIN, false);
  enter_rx();
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  radio_off = 0;

  nrf_timer_task_trigger(SFD_TIMER, NRF_TIMER_TASK_START);

  LOG_DBG("Init done\n");
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  if(payload_len > LR11XX_MAX_PAYLOAD_LEN) {
    LOG_ERR("prepare: Invalid payload size, %d\n", payload_len);
    return RADIO_TX_ERR;
  }
  LOG_DBG("Writing %d bytes to tx buffer\n", payload_len);
  lr11xx_regmem_write_buffer8(NULL, payload, payload_len);
  if(radio_off) {
    off();
  }
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  lr11xx_system_irq_mask_t irq_status;

  if(transmit_len > LR11XX_MAX_PAYLOAD_LEN) {
    LOG_ERR("transmit: Invalid payload size, %d\n", transmit_len);
    return RADIO_TX_ERR;
  }
  /* No CCA before transmitting. TSCH is configured with a zero-length CCA
   * window, and the schedule is what keeps nodes off each other's slots. The
   * LR1110 does support channel activity detection; wiring it up would mean
   * budgeting time for it inside an already long timeslot. */

  /* Set system to standby to prepare for TX */
  lr11xx_system_set_standby(NULL, LR11XX_SYSTEM_STANDBY_CFG_XOSC);
  /* Set payload len */
  lora_pkt_params.pld_len_in_bytes = transmit_len;
  lr11xx_radio_set_lora_pkt_params(NULL, &lora_pkt_params);
  /* Start the transmission */
  lr11xx_system_clear_irq_status(NULL, LR11XX_SYSTEM_IRQ_TX_DONE);
  lr11xx_radio_set_tx(NULL, 0);
  ENERGEST_SWITCH(ENERGEST_TYPE_LISTEN, ENERGEST_TYPE_TRANSMIT);
  do {
    lr11xx_system_get_irq_status(NULL, &irq_status);
  } while(!(irq_status & LR11XX_SYSTEM_IRQ_TX_DONE));
  LOG_DBG("Transmission complete\n");
  /* We are now in RX */
  enter_rx();
  ENERGEST_SWITCH(ENERGEST_TYPE_TRANSMIT, ENERGEST_TYPE_LISTEN);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
read_frame(void *buf, unsigned short bufsize)
{
  lr11xx_radio_rx_buffer_status_t rx_buffer_status;

  if(!pack_pending) {
    LOG_WARN("read_frame: No packet pending!\n");
    return 0;
  }

  if(!last_rx_ts_is_calculated) {
    rx_timestamp_calc(&last_rx_ts);
    last_rx_ts_is_calculated = 1;
  }

  LOG_DBG("read_frame: Packet pending\n");
  lr11xx_radio_get_rx_buffer_status(NULL, &rx_buffer_status);
  if(rx_buffer_status.pld_len_in_bytes > bufsize) {
    LOG_ERR("read_frame: frame of size %d can't fit to buffer of size %d",
            rx_buffer_status.pld_len_in_bytes, bufsize);
    return 0;
  }

  LOG_DBG("read_frame: Bytes pending, %u\n", rx_buffer_status.pld_len_in_bytes);

  lr11xx_regmem_read_buffer8(NULL, buf, rx_buffer_status.buffer_start_pointer,
                             rx_buffer_status.pld_len_in_bytes);

  if(radio_off) {
    off();
  } else {
    enter_rx();
  }

  pack_pending = 0;
  last_packet_len = rx_buffer_status.pld_len_in_bytes;

  return last_packet_len;
}
/*---------------------------------------------------------------------------*/
static void
check_status(void)
{
  lr11xx_system_irq_mask_t irq_status;
  lr11xx_radio_pkt_status_lora_t pkt_status_lora;

  lr11xx_system_get_and_clear_irq_status(NULL, &irq_status);

  if(irq_status & LR11XX_SYSTEM_IRQ_PREAMBLE_DETECTED) {
    /* Preamble detected, a packet is being received */
    pack_receiving = 1;
  }

  if(irq_status & LR11XX_SYSTEM_IRQ_RX_DONE) {
    /* Packet reception has been completed successfuly */
    pack_pending = 1;
    pack_receiving = 0;
    last_rx_ts_is_calculated = 0;
    /* Get packet reception stats */
    lr11xx_radio_get_lora_pkt_status(NULL, &pkt_status_lora);
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, pkt_status_lora.rssi_pkt_in_dbm);
    last_rssi = pkt_status_lora.rssi_pkt_in_dbm;
    last_lqi = pkt_status_lora.snr_pkt_in_db;
  }

  if(irq_status & (LR11XX_SYSTEM_IRQ_CRC_ERROR | LR11XX_SYSTEM_IRQ_HEADER_ERROR)) {
    /* Packet reception has failed */
    pack_receiving = 0;
    pack_pending = 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  check_status();
  return pack_receiving;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  check_status();
  return pack_pending;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    *value = radio_off ? RADIO_POWER_MODE_OFF : RADIO_POWER_MODE_ON;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = cur_channel_id;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    *value = RADIO_RX_MODE_POLL_MODE;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    /* RADIO_TX_MODE_SEND_ON_CCA is not offered: the driver does not do CCA.
     * See transmit(). */
    *value = 0;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    *value = tx_output_power;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    /*
     * Clear channel assessment threshold in dBm. This threshold
     * determines the minimum RSSI level at which the radio will assume
     * that there is a packet in the air.
     */
    return RADIO_RESULT_NOT_SUPPORTED;
  case RADIO_PARAM_RSSI:
    /* Return the RSSI value in dBm */
    return RADIO_RESULT_NOT_SUPPORTED;
  case RADIO_PARAM_LAST_RSSI:
    /* RSSI of the last packet received */
    *value = last_rssi;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_LAST_LINK_QUALITY:
    /* LQI of the last packet received */
    *value = last_lqi;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = RADIO_CHANNEL_0;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = RADIO_CHANNEL_2;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = LR11XX_TX_OUTPUT_POWER_DBM_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = LR11XX_TX_OUTPUT_POWER_DBM_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_MAX_PAYLOAD_LEN:
    *value = (radio_value_t)(PACKET_MAX_LEN);
    return RADIO_RESULT_OK;
  case RADIO_CONST_PHY_OVERHEAD:
    *value = RADIO_PHY_OVERHEAD;
    return RADIO_RESULT_OK;
  case RADIO_CONST_BYTE_AIR_TIME:
    *value = RADIO_BYTE_AIR_TIME;
    return RADIO_RESULT_OK;
  case RADIO_CONST_DELAY_BEFORE_TX:
    *value = RADIO_DELAY_BEFORE_TX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_DELAY_BEFORE_RX:
    *value = RADIO_DELAY_BEFORE_RX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_DELAY_BEFORE_DETECT:
    *value = US_TO_RTIMERTICKS(100);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }

  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    if(value == RADIO_POWER_MODE_ON) {
      on();
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_OFF) {
      off();
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_CARRIER_ON ||
       value == RADIO_POWER_MODE_CARRIER_OFF) {
      return RADIO_RESULT_NOT_SUPPORTED;
    }
    return RADIO_RESULT_INVALID_VALUE;
  case RADIO_PARAM_CHANNEL:
    LOG_INFO("set_value: setting RADIO_PARAM_CHANNEL at %u\n", value);
    if(pending_channel_id != value) {
      pending_channel_id = value;
      if(!radio_off) {
        cur_channel_id = pending_channel_id;
        set_freq_and_tx_power(cur_channel_id, tx_output_power);
      }
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    return RADIO_RESULT_OK;
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER | RADIO_RX_MODE_AUTOACK | RADIO_RX_MODE_POLL_MODE)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    /* RN2483_DEV.radio.tx_cca = (value & RADIO_TX_MODE_SEND_ON_CCA) != 0; */
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    LOG_INFO("set_value: setting RADIO_PARAM_TXPOWER at %u\n", value);
    if(value < LR11XX_TX_OUTPUT_POWER_DBM_MIN || value > LR11XX_TX_OUTPUT_POWER_DBM_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    if(tx_output_power != value) {
      if(!radio_off) {
        // Radio is in RX or TX mode. Put it to standby.
        lr11xx_system_set_standby(NULL, LR11XX_STANDBY_MODE);
      }
      tx_output_power = value;
      set_freq_and_tx_power(cur_channel_id, tx_output_power);
      if(radio_off) {
        off();
      } else {
        on();
      }
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    /*
     * Clear channel assessment threshold in dBm. This threshold
     * determines the minimum RSSI level at which the radio will assume
     * that there is a packet in the air.
     *
     * The CCA threshold must be set to a level above the noise floor of
     * the deployment. Otherwise mechanisms such as send-on-CCA and
     * low-power-listening duty cycling protocols may not work
     * correctly. Hence, the default value of the system may not be
     * optimal for any given deployment.
     */
    return RADIO_RESULT_NOT_SUPPORTED;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
  return RADIO_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  switch(param) {
  case RADIO_PARAM_LAST_PACKET_TIMESTAMP:
    if(size != sizeof(rtimer_clock_t) || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    *(rtimer_clock_t *)dest = last_rx_ts;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TSCH_TIMING:
    if(size != sizeof(uint32_t *) || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }
#if MAC_CONF_WITH_TSCH
    if(param == RADIO_CONST_TSCH_TIMING) {
      if(size != sizeof(uint16_t *) || !dest) {
        return RADIO_RESULT_INVALID_VALUE;
      }
      /* Assigned value: a pointer to the TSCH timing in usec */
      *(const tsch_timing_t **)dest = tsch_timing_lr11xx;
      return RADIO_RESULT_OK;
    }
#endif /* MAC_CONF_WITH_TSCH */
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }

  return RADIO_RESULT_OK;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  /* The radio does not support h/w frame filtering based on addresses */
  return RADIO_RESULT_OK;
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