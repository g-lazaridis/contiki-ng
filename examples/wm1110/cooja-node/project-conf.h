/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
/* Emulate a LoRa PHY in Cooja: LoRa air times plus the long TSCH timeslot
 * template defined in lora-sim-tsch-timing.c. */
#define COOJA_CONF_LORA_PHY
/*---------------------------------------------------------------------------*/
/* Radio, TSCH, RPL and Orchestra settings shared with the hardware examples.
 * Provided by arch/dev/radio/lr1110/configuration-profiles. */
#include "10-per-cent-duty-cycle.h"
/*---------------------------------------------------------------------------*/
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_WARN
#define TSCH_LOG_CONF_PER_SLOT                     1
/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
