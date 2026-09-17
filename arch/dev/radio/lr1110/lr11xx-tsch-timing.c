/**
 * \file
 *        TSCH timeslot timing template for a LoRa PHY on the LR11xx.
 * \author
 *        Gavriil Lazaridis
 */
#include "contiki.h"
#if MAC_CONF_WITH_TSCH
#include "lr11xx-tsch-timing.h"
#include "lr11xx-radio-conf.h"
#include "lora-timings.h"

/**
 * \brief TSCH timing attributes and description. All timings are in usec.
 *
 * CCAOffset   -> time between the beginning of timeslot and start of CCA
 * CCA         -> duration of CCA (CCA is NOT ENABLED by default)
 * TxOffset    -> time between beginning of the timeslot and start of frame TX (end of SFD)
 * RxOffset    -> beginning of the timeslot to when the receiver shall be listening
 * RxAckDelay  -> end of frame to when the transmitter shall listen for ACK
 * TxAckDelay  -> end of frame to the start of ACK tx
 * RxWait      -> time to wait for start of frame (Guard time)
 * AckWait     -> min time to wait for start of an ACK frame
 * RxTx        -> receive-to-transmit switch time (NOT USED)
 * MaxAck      -> TX time to send a max length ACK
 * MaxTx       -> TX time to send the max length frame
 *
 * The TSCH timeslot structure is described in the IEEE 802.15.4-2015 standard,
 * in particular in the Figure 6-30.
 *
 * The default timeslot timing in the standard is a guard time of
 * 2200 us, a Tx offset of 2120 us and a Rx offset of 1120 us.
 * As a result, the listening device has a guard time not centered
 * on the expected Tx time. This is to be fixed in the next iteration
 * of the standard. This can be enabled with:
 * TxOffset: 2120
 * RxOffset: 1120
 * RxWait:   2200
 *
 * Instead, we align the Rx guard time on expected Tx time. The Rx
 * guard time is user-configurable with TSCH_CONF_RX_WAIT.
 * (TxOffset - (RxWait / 2)) instead
 */

#define LR11XX_TSCH_DEFAULT_TS_MAX_TX \
  (LORA_T_PACKET(LR11XX_LORA_SPREADING_FACTOR, LR11XX_LORA_BANDWIDTH_HZ, \
                 LR11XX_LORA_CRC, LR11XX_LORA_PKT_LEN_MODE, \
                 LR11XX_LORA_CODING_RATE, LR11XX_LORA_PREAMBLE_LENGTH, PACKET_MAX_LEN))

#define LR11XX_TSCH_DEFAULT_TS_CCA_OFFSET 0
#define LR11XX_TSCH_DEFAULT_TS_CCA 0
#define LR11XX_TSCH_DEFAULT_TS_TX_OFFSET 7000
#define LR11XX_TSCH_DEFAULT_TS_RX_OFFSET (LR11XX_TSCH_DEFAULT_TS_TX_OFFSET - (TSCH_CONF_RX_WAIT / 2))
#define LR11XX_TSCH_DEFAULT_TS_TX_ACK_DELAY 8800
#define LR11XX_TSCH_DEFAULT_TS_RX_ACK_DELAY 8300
#define LR11XX_TSCH_DEFAULT_TS_RX_WAIT TSCH_CONF_RX_WAIT//20000
#define LR11XX_TSCH_DEFAULT_TS_ACK_WAIT  5000
#define LR11XX_TSCH_DEFAULT_TS_RX_TX 0
#define LR11XX_TSCH_DEFAULT_TS_MAX_ACK (LORA_T_PACKET(LR11XX_LORA_SPREADING_FACTOR, LR11XX_LORA_BANDWIDTH_HZ, LR11XX_LORA_CRC, \
                                                      LR11XX_LORA_PKT_LEN_MODE, LR11XX_LORA_CODING_RATE, \
                                                      LR11XX_LORA_PREAMBLE_LENGTH, 40))

#define LR11XX_TSCH_DEFAULT_TS_TIMESLOT_LENGTH 750000

/* TSCH timeslot timing (microseconds) */
const tsch_timeslot_timing_usec tsch_timing_lr11xx = {
  LR11XX_TSCH_DEFAULT_TS_CCA_OFFSET, /* tsch_ts_cca_offset */
  LR11XX_TSCH_DEFAULT_TS_CCA, /* tsch_ts_cca */
  LR11XX_TSCH_DEFAULT_TS_TX_OFFSET, /* tsch_ts_tx_offset */
  LR11XX_TSCH_DEFAULT_TS_RX_OFFSET, /* tsch_ts_rx_offset */
  LR11XX_TSCH_DEFAULT_TS_RX_ACK_DELAY, /* tsch_ts_rx_ack_delay */
  LR11XX_TSCH_DEFAULT_TS_TX_ACK_DELAY, /* tsch_ts_tx_ack_delay */
  LR11XX_TSCH_DEFAULT_TS_RX_WAIT, /* tsch_ts_rx_wait */
  LR11XX_TSCH_DEFAULT_TS_ACK_WAIT, /* tsch_ts_ack_wait */
  LR11XX_TSCH_DEFAULT_TS_RX_TX, /* tsch_ts_rx_tx */
  LR11XX_TSCH_DEFAULT_TS_MAX_ACK, /* tsch_ts_max_ack */
  LR11XX_TSCH_DEFAULT_TS_MAX_TX, /* tsch_ts_max_tx */
  LR11XX_TSCH_DEFAULT_TS_TIMESLOT_LENGTH, /* tsch_ts_timeslot_length */
};
#endif