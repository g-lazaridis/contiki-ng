#ifndef PER_CENT_DUTY_CYCLE_10_H_
#define PER_CENT_DUTY_CYCLE_10_H_

/* LR11XX configuration */
#define LR11XX_CONF_RF_FREQ_IN_HZ  869450000U
#define LR11XX_CONF_TX_OUTPUT_POWER_DBM 13
#define LR11XX_CONF_LORA_BANDWIDTH   LR11XX_RADIO_LORA_BW_125
#define LR11XX_CONF_LORA_BANDWIDTH_HZ 125000
#define LR11XX_CONF_LORA_CHANNEL_GAP   0
#define LR11XX_CONF_LORA_SPREADING_FACTOR   LR11XX_RADIO_LORA_SF5
#define LR11XX_CONF_LORA_PREAMBLE_LENGTH 12

#define QUEUEBUF_CONF_NUM                     16
#define NBR_TABLE_CONF_MAX_NEIGHBORS          16
#define NETSTACK_MAX_ROUTE_ENTRIES            16
/*******************************************************/
/*************        TSCH                **************/
/*******************************************************/
#define TSCH_CONF_MAX_INCOMING_PACKETS        8
#define TSCH_SCHEDULE_CONF_MAX_LINKS          64
#define TSCH_CONF_EXTEND_TS_SIZE 1
/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0x81a5

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#define TSCH_CONF_AUTOSTART 0

#if WITH_SECURITY
/* Enable security */
#define LLSEC802154_CONF_ENABLED 1
#endif /* WITH_SECURITY */

#define TSCH_WAIT_EB RTIMER_SECOND / 2

#define TSCH_CONF_ADAPTIVE_TIMESYNC 1

#define TSCH_CONF_CHANNEL_SCAN_DURATION CLOCK_SECOND

/* Max time before sending a unicast keep-alive message to the time source */
// #define TSCH_CONF_KEEPALIVE_TIMEOUT (48 * CLOCK_SECOND)

/* With TSCH_ADAPTIVE_TIMESYNC enabled: keep-alive timeout used after reaching
 * accurate drift compensation. */
#define TSCH_CONF_MAX_KEEPALIVE_TIMEOUT (60 * CLOCK_SECOND)

/* Max time without synchronization before leaving the PAN */
// #define TSCH_CONF_DESYNC_THRESHOLD (4 * TSCH_CONF_KEEPALIVE_TIMEOUT)

/* Period between two consecutive EBs */
#define TSCH_CONF_EB_PERIOD (30 * CLOCK_SECOND)
#define TSCH_CONF_MAX_EB_PERIOD  (55 * CLOCK_SECOND)

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE    (uint8_t[]){ 0 }
#define TSCH_CONF_JOIN_HOPPING_SEQUENCE       (uint8_t[]){ 0 }

/*******************************************************/
/************* RPL                        **************/
/*******************************************************/

#define RPL_CONF_DIS_INTERVAL (120 * CLOCK_SECOND)
#define RPL_CONF_DAO_RETRANSMISSION_TIMEOUT (150 * CLOCK_SECOND)
#define RPL_CONF_DELAY_BEFORE_LEAVING (300 * CLOCK_SECOND)
// #define RPL_CONF_DAO_MAX_RETRANSMISSIONS 15
#define RPL_CONF_DIO_INTERVAL_MIN 15
#define RPL_CONF_DIO_INTERVAL_DOUBLINGS 3

/*******************************************************/
/************* Orchestra                  **************/
/*******************************************************/

#define ORCHESTRA_CONF_EBSF_PERIOD            29//17
#define ORCHESTRA_CONF_COMMON_SHARED_PERIOD   21//13/*43 */
#define ORCHESTRA_CONF_UNICAST_PERIOD         11//11/*31 */

#define ORCHESTRA_CONF_RULES      { &eb_per_time_source, \
                                    &unicast_per_non_root_neighbor, \
                                    &special_for_root_always_on, \
                                    &default_common }
/* Our "hash" is collision-free */
#define ORCHESTRA_CONF_COLLISION_FREE_HASH    1

#endif /* PER_CENT_DUTY_CYCLE_10_H_ */