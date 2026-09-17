/*
 * Copyright (c) 2024, Gavriil Lazaridis.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \addtogroup wm1110-dk
 * @{
 *
 * \file
 *        GPIO tracing of TSCH timeslot events for the Wio-WM1110 DK.
 * \author
 *        Gavriil Lazaridis
 *
 * At a 750 ms timeslot, TSCH is far too timing-sensitive to debug over the
 * serial port: printing from inside a slot desynchronises the node from its
 * time source. Instead, this header wires TSCH's timeslot hooks to three
 * GPIOs that can be captured with a logic analyser, which is how the
 * router/node slot alignment was measured during development.
 *
 * Signals (available on the DK's expansion header):
 *
 *   P0.19  high for the duration of every timeslot
 *   P0.20  toggles at each stage of a reception slot
 *   P0.21  toggles at each stage of a transmission slot
 *
 * To enable, define WM1110_CONF_TSCH_DEBUG_GPIO before including this file
 * from your project-conf.h:
 *
 *   #define WM1110_CONF_TSCH_DEBUG_GPIO 1
 *   #include "wm1110-tsch-debug.h"
 *
 * The pins are driven with direct register writes rather than the GPIO HAL
 * so that the tracing itself costs as little slot time as possible.
 */

#ifndef WM1110_TSCH_DEBUG_H_
#define WM1110_TSCH_DEBUG_H_

#if WM1110_CONF_TSCH_DEBUG_GPIO

#include "gpio-hal-arch.h"

#define WM1110_TSCH_DEBUG_SLOT_PORT  0
#define WM1110_TSCH_DEBUG_SLOT_PIN  19
#define WM1110_TSCH_DEBUG_RX_PORT    0
#define WM1110_TSCH_DEBUG_RX_PIN    20
#define WM1110_TSCH_DEBUG_TX_PORT    0
#define WM1110_TSCH_DEBUG_TX_PIN    21

#define TSCH_DEBUG_INIT() \
  do { \
    gpio_hal_arch_pin_set_output(WM1110_TSCH_DEBUG_SLOT_PORT, WM1110_TSCH_DEBUG_SLOT_PIN); \
    gpio_hal_arch_pin_set_output(WM1110_TSCH_DEBUG_RX_PORT, WM1110_TSCH_DEBUG_RX_PIN); \
    gpio_hal_arch_pin_set_output(WM1110_TSCH_DEBUG_TX_PORT, WM1110_TSCH_DEBUG_TX_PIN); \
    gpio_hal_arch_clear_pin(WM1110_TSCH_DEBUG_SLOT_PORT, WM1110_TSCH_DEBUG_SLOT_PIN); \
    gpio_hal_arch_clear_pin(WM1110_TSCH_DEBUG_RX_PORT, WM1110_TSCH_DEBUG_RX_PIN); \
    gpio_hal_arch_clear_pin(WM1110_TSCH_DEBUG_TX_PORT, WM1110_TSCH_DEBUG_TX_PIN); \
  } while(0)

#define TSCH_DEBUG_SLOT_START() \
  gpio_hal_arch_set_pin(WM1110_TSCH_DEBUG_SLOT_PORT, WM1110_TSCH_DEBUG_SLOT_PIN)
#define TSCH_DEBUG_SLOT_END() \
  gpio_hal_arch_clear_pin(WM1110_TSCH_DEBUG_SLOT_PORT, WM1110_TSCH_DEBUG_SLOT_PIN)

#define TSCH_DEBUG_RX_EVENT() \
  gpio_hal_arch_toggle_pin(WM1110_TSCH_DEBUG_RX_PORT, WM1110_TSCH_DEBUG_RX_PIN)
#define TSCH_DEBUG_RX_EVENT_OFF() \
  gpio_hal_arch_clear_pin(WM1110_TSCH_DEBUG_RX_PORT, WM1110_TSCH_DEBUG_RX_PIN)

#define TSCH_DEBUG_TX_EVENT() \
  gpio_hal_arch_toggle_pin(WM1110_TSCH_DEBUG_TX_PORT, WM1110_TSCH_DEBUG_TX_PIN)
#define TSCH_DEBUG_TX_EVENT_OFF() \
  gpio_hal_arch_clear_pin(WM1110_TSCH_DEBUG_TX_PORT, WM1110_TSCH_DEBUG_TX_PIN)

#endif /* WM1110_CONF_TSCH_DEBUG_GPIO */

#endif /* WM1110_TSCH_DEBUG_H_ */

/** @} */
