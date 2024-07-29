/**
 * @file lr11xx-radio-conf.h
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-07-24
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef LR11XX_RADIO_CONF_H_
#define LR11XX_RADIO_CONF_H_
#include "lr11xx_radio_types.h"
/*---------------------------------------------------------------------------*/
/*!
 * @brief General parameters
 */
#ifdef  LR11XX_CONF_PACKET_TYPE
#define LR11XX_PACKET_TYPE LR11XX_CONF_PACKET_TYPE
#else
#define LR11XX_PACKET_TYPE LR11XX_RADIO_PKT_TYPE_LORA
#endif

#ifdef  LR11XX_CONF_RF_FREQ_IN_HZ
#define LR11XX_RF_FREQ_IN_HZ  LR11XX_CONF_RF_FREQ_IN_HZ
#else
#define LR11XX_RF_FREQ_IN_HZ  868000000U
#endif

#ifdef  LR11XX_CONF_TX_OUTPUT_POWER_DBM
#define LR11XX_TX_OUTPUT_POWER_DBM  LR11XX_CONF_TX_OUTPUT_POWER_DBM
#else
#define LR11XX_TX_OUTPUT_POWER_DBM 13  /* range [-17, +22] for sub-G, range [-18, 13] for 2.4G ( HF_PA ) */
#endif

#ifdef  LR11XX_CONF_PA_RAMP_TIME
#define LR11XX_PA_RAMP_TIME LR11XX_CONF_PA_RAMP_TIME
#else
#define LR11XX_PA_RAMP_TIME LR11XX_RADIO_RAMP_48_US
#endif

#ifdef  LR11XX_CONF_STANDBY_MODE
#define LR11XX_STANDBY_MODE LR11XX_CONF_STANDBY_MODE
#else
#define LR11XX_STANDBY_MODE LR11XX_SYSTEM_STANDBY_CFG_XOSC
#endif

#ifdef  LR11XX_CONF_FALLBACK_MODE
#define LR11XX_FALLBACK_MODE LR11XX_CONF_FALLBACK_MODE
#else
#define LR11XX_FALLBACK_MODE LR11XX_RADIO_FALLBACK_STDBY_XOSC
#endif

#ifdef  LR11XX_CONF_ENABLE_RX_BOOST_MODE
#define LR11XX_ENABLE_RX_BOOST_MODE LR11XX_CONF_ENABLE_RX_BOOST_MODE
#else
#define LR11XX_ENABLE_RX_BOOST_MODE false
#endif

#ifdef LR11XX_CONF_PAYLOAD_LENGTH
#define LR11XX_PAYLOAD_LENGTH   LR11XX_CONF_PAYLOAD_LENGTH
#else
#define LR11XX_PAYLOAD_LENGTH   0 /* With 0, all payloads from 0 to 255 bytes are accepted from the receiver*/
#endif

/*!
 * @brief Modulation parameters for LoRa packets
 */
#ifdef  LR11XX_CONF_LORA_SPREADING_FACTOR
#define LR11XX_LORA_SPREADING_FACTOR LR11XX_CONF_LORA_SPREADING_FACTOR
#else
#define LR11XX_LORA_SPREADING_FACTOR LR11XX_RADIO_LORA_SF7
#endif

#ifdef  LR11XX_CONF_LORA_BANDWIDTH
#define LR11XX_LORA_BANDWIDTH LR11XX_CONF_LORA_BANDWIDTH
#else
#define LR11XX_LORA_BANDWIDTH LR11XX_RADIO_LORA_BW_125
#endif

#ifdef  LR11XX_CONF_LORA_CODING_RATE
#define LR11XX_LORA_CODING_RATE LR11XX_CONF_LORA_CODING_RATE
#else
#define LR11XX_LORA_CODING_RATE LR11XX_RADIO_LORA_CR_4_5
#endif

/*!
 * @brief Packet parameters for LoRa packets
 */
#ifdef  LR11XX_CONF_LORA_PREAMBLE_LENGTH
#define LR11XX_LORA_PREAMBLE_LENGTH LR11XX_CONF_LORA_PREAMBLE_LENGTH
#else
#define LR11XX_LORA_PREAMBLE_LENGTH 8
#endif

#ifdef  LR11XX_CONF_LORA_PKT_LEN_MODE
#define LR11XX_LORA_PKT_LEN_MODE LR11XX_CONF_LORA_PKT_LEN_MODE
#else
#define LR11XX_LORA_PKT_LEN_MODE LR11XX_RADIO_LORA_PKT_EXPLICIT
#endif

#ifdef  LR11XX_CONF_LORA_IQ
#define LR11XX_LORA_IQ LR11XX_CONF_LORA_IQ
#else
#define LR11XX_LORA_IQ LR11XX_RADIO_LORA_IQ_INVERTED
#endif

#ifdef  LR11XX_CONF_LORA_CRC
#define LR11XX_LORA_CRC LR11XX_CONF_LORA_CRC
#else
#define LR11XX_LORA_CRC LR11XX_RADIO_LORA_CRC_ON
#endif

/*!
 * @brief LoRa sync word
 */
#ifdef  LR11XX_CONF_LORA_SYNCWORD
#define LR11XX_LORA_SYNCWORD  LR11XX_CONF_LORA_SYNCWORD
#else
#define LR11XX_LORA_SYNCWORD  0x12  /* 0x12 Private Network, 0x34 Public Network */
#endif

#if LR11XX_LORA_CRC == LR11XX_RADIO_LORA_CRC_ON
#define LR11XX_MAX_PAYLOAD_LEN      253
#else
#define LR11XX_MAX_PAYLOAD_LEN      255
#endif
/*---------------------------------------------------------------------------*/
typedef struct {
  int8_t power;
  lr11xx_radio_pa_cfg_t pa_config;
} lr11xx_radio_conf_pa_pwr_cfg_t;
/*---------------------------------------------------------------------------*/
const lr11xx_radio_conf_pa_pwr_cfg_t *lr11xx_radio_conf_get_pa_pwr_cfg(const uint32_t rf_freq_in_hz,
                                                                       int8_t expected_output_pwr_in_dbm);

const lr11xx_radio_rssi_calibration_table_t *lr11xx_radio_conf_get_rssi_calibration_table(const uint32_t freq_in_hz);
/*!
 * @brief A function to get the value for low data rate optimization setting
 *
 * @param [in] sf  LoRa Spreading Factor
 * @param [in] bw  LoRa Bandwidth
 */
uint8_t lr11xx_radio_conf_compute_lora_ldro(const lr11xx_radio_lora_sf_t sf, const lr11xx_radio_lora_bw_t bw);
/*---------------------------------------------------------------------------*/
#endif /* LR11XX_RADIO_CONF_H_ */