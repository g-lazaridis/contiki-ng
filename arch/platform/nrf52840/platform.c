/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup nrf52840dk nRF52840 Development Kit
 * @{
 */
#include "contiki.h"
#include "nordic_common.h"
#include "rtimer-arch.h"

#include "sdk_config.h"
#include "nrfx_gpiote.h"
#include "nrf.h"
#include "nrf_drv_power.h"
#include "nrfx_clock.h"
#include "contiki-net.h"
#include "leds.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"

#include "dev/serial-line.h"
#include "dev/uart0.h"
#include "usb/usb-serial.h"
#include "usb/usb-dfu-trigger.h"
#include "lpm.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Nordic semi OUI */
#define NORDIC_SEMI_VENDOR_OUI 0xF4CE36
/*---------------------------------------------------------------------------*/
#if STOP_HFCLK_ON_IDLE_ENABLE
#define HFLCK_STARTUP_TIME_US 800
#define HFLCK_STARTUP_TIME_RTIMER_TICKS  (1 + (HFLCK_STARTUP_TIME_US * RTIMER_RTC_FREQ_HZ) / (1000000LU))
#define MIN_SLEEP_TIME_RTIMER_TICKS (HFLCK_STARTUP_TIME_RTIMER_TICKS * 10)
#endif
/*---------------------------------------------------------------------------*/
static void
populate_link_address(void)
{
  uint8_t device_address[8];
  uint32_t device_address_low;

  /*
   * Populate the link address' 3 MSBs using Nordic's OUI.
   * For the remaining 5 bytes just use any 40 of the 48 FICR->DEVICEADDR
   * Those are random, so endianness is irrelevant.
   */
  device_address[0] = (NORDIC_SEMI_VENDOR_OUI) >> 16 & 0xFF;
  device_address[1] = (NORDIC_SEMI_VENDOR_OUI) >> 8 & 0xFF;
  device_address[2] = NORDIC_SEMI_VENDOR_OUI & 0xFF;
  device_address[3] = NRF_FICR->DEVICEADDR[1] & 0xFF;

  device_address_low = NRF_FICR->DEVICEADDR[0];
  memcpy(&device_address[4], &device_address_low, 4);

  memcpy(&linkaddr_node_addr, &device_address[8 - LINKADDR_SIZE],
         LINKADDR_SIZE);
}
/*---------------------------------------------------------------------------*/
void
platform_init_stage_one(void)
{
  /* Start the Xtal HFCLK */
  nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
  nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
  while(!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED));
  nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);

  gpio_hal_init();
  leds_init();
}
/*---------------------------------------------------------------------------*/
void
platform_init_stage_two(void)
{
#ifdef PLATFORM_HAS_BUTTON
  button_hal_init();
#endif

  /* Seed value is ignored since hardware RNG is used. */
  random_init(0);

#if UART0_ENABLED
  uart0_init();
  serial_line_init();
#if BUILD_WITH_SHELL
  uart0_set_input(serial_line_input_byte);
#endif
#endif

#if NRF52840_NATIVE_USB
  usb_serial_init();
  serial_line_init();
#if BUILD_WITH_SHELL
  usb_serial_set_input(serial_line_input_byte);
#endif
#endif

#if NRF52840_USB_DFU_TRIGGER
  dfu_trigger_usb_init();
#endif

  populate_link_address();
}
/*---------------------------------------------------------------------------*/
void
platform_init_stage_three(void)
{
  process_start(&sensors_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
platform_idle()
{
#if NRF_RTIMER_RTC_ENABLE && STOP_HFCLK_ON_IDLE_ENABLE
  clock_time_t next_etimer;
  rtimer_clock_t next_schedule = RTIMER_CLOCK_MAX;

  next_schedule = rtimer_arch_time_to_rtimer();
  if(next_schedule != RTIMER_CLOCK_MAX) {
    next_schedule = RTIMER_CLOCK_DIFF(next_schedule, RTIMER_NOW());
    next_etimer = etimer_next_expiration_time();
    if(next_etimer) {
      next_schedule = MIN(next_schedule,
                          RTIMER_CLOCK_DIFF((next_etimer * RTIMER_SECOND) / CLOCK_SECOND, RTIMER_NOW()));
    }
    if(next_schedule > MIN_SLEEP_TIME_RTIMER_TICKS) {
      /* Stop the HFCLK before falling to lpm and start it again when exiting from lpm*/
      nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);
      while(nrf_clock_hf_is_running(NRF_CLOCK_HFCLK_HIGH_ACCURACY));
      rtimer_arch_preschedule(RTIMER_CLOCK_DIFF(next_schedule, HFLCK_STARTUP_TIME_RTIMER_TICKS));
      lpm_drop(1);
      nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
      nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
      while(!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED));
      nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
    } else {
      lpm_drop(0);
    }
  } else {
    lpm_drop(0);
  }
#else
  lpm_drop(0);
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
