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
/**
 * \addtogroup nrf52840
 * @{
 *
 * \file
 *         Architecture dependent rtimer implementation header file.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "nrf.h"
#include <stdint.h>
#include <stddef.h>

#if NRF_RTIMER_RTC_ENABLE
/*---------------------------------------------------------------------------*/
/*
 * Use Timer RTC 1 at 32768Hz. Generates 1 tick per 30,515 usecs.
 */
/*---------------------------------------------------------------------------*/
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#define RTIMER_RTC_IRQ_PRIORITY  6
#define RTC_MAX 0xFFFFFFUL
static const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(1); //Use RTC1 for RTIMER
static volatile bool has_next = false;
static volatile rtimer_clock_t scheduled_time = 0, m_base_counter = 0;
#define SCHEDULE_COMPARE_CHANNEL NRFX_RTC_INT_COMPARE0
#define PRE_SCHEDULE_COMPARE_CHANNEL NRFX_RTC_INT_COMPARE1
#else
/*---------------------------------------------------------------------------*/
/*
 * Use Timer RTIMER_TIMER at 62500Hz. Generates 1 tick per exactly 16 usecs,
 * which is exactly 1 .15.4 symbol period.
 */
/*---------------------------------------------------------------------------*/
#include "nrf_timer.h"
#define TIMER_INSTANCE NRF_TIMER0
#endif
/*---------------------------------------------------------------------------*/
#if NRF_RTIMER_RTC_ENABLE
/*---------------------------------------------------------------------------*/
/**
 * @brief Return current rtimer_clock_t timestamp
 */
static rtimer_clock_t
get_now(void)
{
  static uint32_t last_counter_val = 0;
  uint32_t rtc_counter;

  rtc_counter = nrf_drv_rtc_counter_get(&rtc);

  // Overflow occured
  if(rtc_counter < last_counter_val) {
    m_base_counter += (RTC_MAX + 1);
  }

  last_counter_val = rtc_counter;

  return (rtimer_clock_t)(m_base_counter + rtc_counter);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Function for handling the RTC1 interrupts
 * \param int_type Type of interrupt to be handled
 */
static void
rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
  if(int_type == NRFX_RTC_INT_COMPARE0) {
    has_next = false;
    rtimer_run_next();
  }
}
/*---------------------------------------------------------------------------*/
static void
rtc_schedule(rtimer_clock_t t, int channel)
{
  nrf_drv_rtc_cc_set(&rtc, channel, t % (RTC_MAX + 1), 1);
}
#endif
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
#if NRF_RTIMER_RTC_ENABLE
  nrf_drv_rtc_config_t config;
  /* Before starting an RTC instance the LFCLK must be started.
     Although the LFCLK is started before from clock_init(), we also
     do the procedure here */
  nrf_drv_clock_init();
  nrf_drv_clock_lfclk_request(NULL);

  config.prescaler = RTC_FREQ_TO_PRESCALER(RTIMER_RTC_FREQ_HZ);
  config.interrupt_priority = RTIMER_RTC_IRQ_PRIORITY;
  nrf_drv_rtc_init(&rtc, &config, rtc_handler);
  /* Power on RTC instance */
  nrf_drv_rtc_enable(&rtc);
#else
  nrf_timer_event_clear(TIMER_INSTANCE, NRF_TIMER_EVENT_COMPARE0);
  nrf_timer_frequency_set(TIMER_INSTANCE, NRF_TIMER_FREQ_62500Hz);
  nrf_timer_bit_width_set(TIMER_INSTANCE, NRF_TIMER_BIT_WIDTH_32);
  nrf_timer_mode_set(TIMER_INSTANCE, NRF_TIMER_MODE_TIMER);
  nrf_timer_int_enable(TIMER_INSTANCE, NRF_TIMER_INT_COMPARE0_MASK);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);
  nrf_timer_task_trigger(TIMER_INSTANCE, NRF_TIMER_TASK_START);
#endif
}
/*---------------------------------------------------------------------------*/
/**
 *
 * This function schedules a one-shot event with the nRF RTC.
 */
void
rtimer_arch_schedule(rtimer_clock_t t)
{
#if NRF_RTIMER_RTC_ENABLE
  rtc_schedule(t, SCHEDULE_COMPARE_CHANNEL);
  scheduled_time = t;
  has_next = true;
#else
  nrf_timer_cc_write(TIMER_INSTANCE, NRF_TIMER_CC_CHANNEL0, t);
#endif
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now()
{
#if NRF_RTIMER_RTC_ENABLE
  return get_now();
#else
  nrf_timer_task_trigger(TIMER_INSTANCE, NRF_TIMER_TASK_CAPTURE1);
  return nrf_timer_cc_read(TIMER_INSTANCE, NRF_TIMER_CC_CHANNEL1);
#endif
}
/*---------------------------------------------------------------------------*/
#if !NRF_RTIMER_RTC_ENABLE
void
TIMER0_IRQHandler(void)
{

  if(nrf_timer_event_check(TIMER_INSTANCE, NRF_TIMER_EVENT_COMPARE0)) {
    nrf_timer_event_clear(TIMER_INSTANCE, NRF_TIMER_EVENT_COMPARE0);
    rtimer_run_next();
  }
}
#endif
#if NRF_RTIMER_RTC_ENABLE
/*---------------------------------------------------------------------------*/
void
rtimer_arch_preschedule(rtimer_clock_t t)
{
  rtc_schedule(t, PRE_SCHEDULE_COMPARE_CHANNEL);
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_time_to_rtimer(void)
{
  if(has_next) {
    return scheduled_time;
  }
  /* if no wakeup is scheduled yet return maximum time */
  return RTIMER_CLOCK_MAX;
}
#endif /* NRF_RTIMER_RTC_ENABLE */
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
