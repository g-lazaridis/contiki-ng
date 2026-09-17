/*
 * Copyright (c) 2017, George Oikonomou - http://www.spd.gr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
#ifndef NRF52840_DEF_H_
#define NRF52840_DEF_H_
/*---------------------------------------------------------------------------*/
#include "cm4/cm4-def.h"
#if BOARD_WM1110_DK
#include "lora-timings.h"
#endif
/*---------------------------------------------------------------------------*/
/* Path to headers with implementation of mutexes, atomic and memory barriers */
#define MUTEX_CONF_ARCH_HEADER_PATH          "mutex-cortex.h"
#define ATOMIC_CONF_ARCH_HEADER_PATH         "atomic-cortex.h"
#define MEMORY_BARRIER_CONF_ARCH_HEADER_PATH "memory-barrier-cortex.h"
#if BOARD_WM1110_DK
#define TSCH_CONF_ARCH_HDR_PATH              "lr11xx-tsch-timing.h"
#endif /* BOARD_WM1110_DK */
/*---------------------------------------------------------------------------*/
/* Source the rtimer from an RTC (LFCLK) instead of TIMER0. The LoRa port needs
 * this: it lets the HFCLK stop while idle, which matters over a 750 ms
 * timeslot. The DK and Dongle keep the upstream TIMER0 source unless asked
 * otherwise. */
#ifdef NRF_RTIMER_RTC_CONF_ENABLE
#define NRF_RTIMER_RTC_ENABLE NRF_RTIMER_RTC_CONF_ENABLE
#elif BOARD_WM1110_DK
#define NRF_RTIMER_RTC_ENABLE 1
#else
#define NRF_RTIMER_RTC_ENABLE 0
#endif
/*---------------------------------------------------------------------------*/
#if NRF_RTIMER_RTC_ENABLE
#define RTIMER_ARCH_SECOND 32768
#else
#define RTIMER_ARCH_SECOND 62500
#endif
/*---------------------------------------------------------------------------*/
/* For extra power conservation, stop the HFCLK before going to idle and exit the idle mode
   HFLCK_STARTUP_TIME_US earlier to start the HFCLK again. Can be applied only when the RTC (LFCLK)
   clock is used for rtimer. */
#ifdef STOP_HFCLK_ON_IDLE_CONF_ENABLE
#define STOP_HFCLK_ON_IDLE_ENABLE STOP_HFCLK_ON_IDLE_CONF_ENABLE
#else
#define STOP_HFCLK_ON_IDLE_ENABLE  NRF_RTIMER_RTC_ENABLE
#endif
/*---------------------------------------------------------------------------*/
/* Do the math in 32bits to save precision.
 * Round to nearest integer rather than truncate. */
#define US_TO_RTIMERTICKS(US)  ((US) >= 0 ? \
                                (((int64_t)(US)*(RTIMER_ARCH_SECOND)+500000) / 1000000L) : \
                                ((int64_t)(US)*(RTIMER_ARCH_SECOND)-500000) / 1000000L)

#define RTIMERTICKS_TO_US(T)   ((T) >= 0 ? \
                                (((int64_t)(T) * 1000000L + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)) : \
                                ((int64_t)(T) * 1000000L - ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND))

/* A 64-bit version because the 32-bit one cannot handle T >= 4295 ticks.
   Intended only for positive values of T. */
#define RTIMERTICKS_TO_US_64(T)  ((uint32_t)(((uint64_t)(T) * 1000000 + ((RTIMER_ARCH_SECOND) / 2)) / (RTIMER_ARCH_SECOND)))
/*---------------------------------------------------------------------------*/
#if BOARD_WM1110_DK
/* LoRa PHY on the LR1110. Packet duration is computed from the modulation
 * parameters rather than a fixed per-byte air time, so TSCH_PACKET_DURATION()
 * is overridden and RADIO_BYTE_AIR_TIME is unused. */
#define RADIO_PHY_OVERHEAD            US_TO_RTIMERTICKS(LR11XX_LORA_PREAMBLE_LENGTH + (LR11XX_LORA_CRC ? 5 : 0))
#define RADIO_BYTE_AIR_TIME           0
#define RADIO_DELAY_BEFORE_RX         ((unsigned)US_TO_RTIMERTICKS(1000 + 43 + 50 + 39))
#define RADIO_DELAY_BEFORE_DETECT     US_TO_RTIMERTICKS(LORA_T_PREAMBLE(LR11XX_LORA_SPREADING_FACTOR, LR11XX_LORA_BANDWIDTH_HZ, LR11XX_LORA_PREAMBLE_LENGTH))

#define TSCH_PACKET_DURATION(len)     US_TO_RTIMERTICKS(LORA_T_PACKET(LR11XX_LORA_SPREADING_FACTOR, LR11XX_LORA_BANDWIDTH_HZ, LR11XX_LORA_CRC, LR11XX_LORA_PKT_LEN_MODE, LR11XX_LORA_CODING_RATE, LR11XX_LORA_PREAMBLE_LENGTH, len))

#define TSCH_CONF_HW_FRAME_FILTERING  0
/* Power the radio down between the phases of a timeslot. Worth doing over a
 * 750 ms slot, at the cost of a longer turn-on allowance before Tx. */
#ifndef TSCH_CONF_RADIO_ON_DURING_TIMESLOT
#define TSCH_CONF_RADIO_ON_DURING_TIMESLOT 0
#endif

#if TSCH_CONF_RADIO_ON_DURING_TIMESLOT
#define RADIO_DELAY_BEFORE_TX         US_TO_RTIMERTICKS(220)
#else
#define RADIO_DELAY_BEFORE_TX         US_TO_RTIMERTICKS(2606)
#endif

#define TSCH_CONF_RX_WAIT             7000
#define TSCH_CONF_DEFAULT_TIMESLOT_TIMING tsch_timing_lr11xx

#else /* BOARD_WM1110_DK */

/* 802.15.4 radio in the nRF52840 itself. */
#define RADIO_PHY_OVERHEAD            3
#define RADIO_BYTE_AIR_TIME          32
#define RADIO_SHR_LEN                 5 /* Synch word + SFD */
#define RADIO_DELAY_BEFORE_TX         \
  ((unsigned)US_TO_RTIMERTICKS(RADIO_SHR_LEN * RADIO_BYTE_AIR_TIME))
/* Very conservative value moved over from CC2538 */
#define RADIO_DELAY_BEFORE_RX         ((unsigned)US_TO_RTIMERTICKS(250))
#define RADIO_DELAY_BEFORE_DETECT     0

#define TSCH_CONF_HW_FRAME_FILTERING  0
#define TSCH_CONF_RADIO_ON_DURING_TIMESLOT 1

#endif /* BOARD_WM1110_DK */

/* Use hardware timestamps.
 * Disabling this greatly impacts TSCH sync, especially on preview devkits.
 */
#ifndef TSCH_CONF_RESYNC_WITH_SFD_TIMESTAMPS
#define TSCH_CONF_RESYNC_WITH_SFD_TIMESTAMPS 1
#define TSCH_CONF_TIMESYNC_REMOVE_JITTER 0
#endif

#ifndef TSCH_CONF_BASE_DRIFT_PPM
/*
 * The drift compared to "true" 10ms slots.
 * Enable adaptive sync to enable compensation for this.
 * Slot length 10000 usec
 * 1000000 / 62500 = 16 usec / rtimer tick
 * 10 ms is 625 ticks, exactly
 * Real slot duration 10000 usec
 */
#define TSCH_CONF_BASE_DRIFT_PPM    0
#endif
/*---------------------------------------------------------------------------*/
/* Enable S/W ACKs with CSMA */
#define CSMA_CONF_SEND_SOFT_ACK       1
/*---------------------------------------------------------------------------*/
#define GPIO_HAL_CONF_ARCH_HDR_PATH          "dev/gpio-hal-arch.h"
/*---------------------------------------------------------------------------*/
#define GPIO_HAL_CONF_ARCH_SW_TOGGLE 0
/*---------------------------------------------------------------------------*/
#endif /* NRF52840_DEF_H_ */
/*---------------------------------------------------------------------------*/
