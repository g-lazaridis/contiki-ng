/**
 * \file
 *        TSCH timeslot timing template for the LoRa PHY emulated in Cooja.
 * \author
 *        Gavriil Lazaridis
 */

#include "contiki.h"
#include "net/mac/tsch/tsch.h"

#define LORA_SIM_TSCH_DEFAULT_TS_CCA_OFFSET   0
#define LORA_SIM_TSCH_DEFAULT_TS_CCA    0
#define LORA_SIM_TSCH_DEFAULT_TS_TX_OFFSET  7000
#define LORA_SIM_TSCH_DEFAULT_TS_RX_OFFSET  (LORA_SIM_TSCH_DEFAULT_TS_TX_OFFSET - (TSCH_CONF_RX_WAIT / 2))
#define LORA_SIM_TSCH_DEFAULT_TS_TX_ACK_DELAY 5000
#define LORA_SIM_TSCH_DEFAULT_TS_RX_ACK_DELAY 4500
#define LORA_SIM_TSCH_DEFAULT_TS_RX_WAIT TSCH_CONF_RX_WAIT//20000
#define LORA_SIM_TSCH_DEFAULT_TS_ACK_WAIT  5000
#define LORA_SIM_TSCH_DEFAULT_TS_RX_TX 0
#define LORA_SIM_TSCH_DEFAULT_TS_MAX_ACK 28224
#define LORA_SIM_TSCH_DEFAULT_TS_MAX_TX   73024
#define LORA_SIM_TSCH_DEFAULT_TS_TIMESLOT_LENGTH  750000

const tsch_timeslot_timing_usec lora_sim_tsch_timing = {
  LORA_SIM_TSCH_DEFAULT_TS_CCA_OFFSET, /* tsch_ts_cca_offset */
  LORA_SIM_TSCH_DEFAULT_TS_CCA, /* tsch_ts_cca */
  LORA_SIM_TSCH_DEFAULT_TS_TX_OFFSET, /* tsch_ts_tx_offset */
  LORA_SIM_TSCH_DEFAULT_TS_RX_OFFSET, /* tsch_ts_rx_offset */
  LORA_SIM_TSCH_DEFAULT_TS_RX_ACK_DELAY, /* tsch_ts_rx_ack_delay */
  LORA_SIM_TSCH_DEFAULT_TS_TX_ACK_DELAY, /* tsch_ts_tx_ack_delay */
  LORA_SIM_TSCH_DEFAULT_TS_RX_WAIT, /* tsch_ts_rx_wait */
  LORA_SIM_TSCH_DEFAULT_TS_ACK_WAIT, /* tsch_ts_ack_wait */
  LORA_SIM_TSCH_DEFAULT_TS_RX_TX, /* tsch_ts_rx_tx */
  LORA_SIM_TSCH_DEFAULT_TS_MAX_ACK, /* tsch_ts_max_ack */
  LORA_SIM_TSCH_DEFAULT_TS_MAX_TX, /* tsch_ts_max_tx */
  LORA_SIM_TSCH_DEFAULT_TS_TIMESLOT_LENGTH, /* tsch_ts_timeslot_length */
};