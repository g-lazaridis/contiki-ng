/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
#include "nordic_common.h"

/* Radio, TSCH, RPL and Orchestra settings. Selected with
 * -DNETWORK_PROFILE_INC=<name> from the Makefile; the profiles live in
 * arch/dev/radio/lr1110/configuration-profiles. */
#ifdef NETWORK_PROFILE_INC
#include STRINGIFY(NETWORK_PROFILE_INC.h)
#endif

#include "lr11xx-radio-conf.h"
/*---------------------------------------------------------------------------*/
/* Trace TSCH timeslot events on P0.19/P0.20/P0.21 for capture with a logic
 * analyser. See arch/platform/nrf52840/wm1110-dk/wm1110-tsch-debug.h. */
/* #define WM1110_CONF_TSCH_DEBUG_GPIO 1 */
#include "wm1110-tsch-debug.h"
/*---------------------------------------------------------------------------*/
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     0
/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
