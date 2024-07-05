/**
 * @file uarte.h
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-05
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef UARTE_H_
#define UARTE_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include "nrf52840.h"
#include "uart.h"
/*---------------------------------------------------------------------------*/
typedef struct {
  uint32_t tx_pin;                              // TX pin number.
  uint32_t rx_pin;                              // RX pin number.
  uint8_t baudrate;                             // Baud rate.
  uint8_t enabled;                              // True when enabled.
  uint8_t tx_started;                           // True when transmission is ongoing.
  uint8_t rx_started;                           // True when the uarte interface is listening for input bytes.
  uint8_t input_byte;                           // Where input is stored.
  NRF_UARTE_Type *peripheral;                   // Instance of UARTE interface.
  IRQn_Type irq_type;                           // Interrupt type of UARTE interface.
  int (*input_byte_handler)(unsigned char c);   // Rx single byte input handler.
} uarte_data_t;
/*---------------------------------------------------------------------------*/
int uarte_set_input(int (*input)(unsigned char c), uarte_data_t *uarte_d);
int uarte_write(uarte_data_t *uarte_d, uint8_t const *p_data, int length);
int uarte_writeb(uarte_data_t *uarte_d, uint8_t data);
int uarte_init(uarte_data_t *uarte_d);
int uarte_enable(uarte_data_t *uarte_d);
int uarte_disable(uarte_data_t *uarte_d);
int uarte_active(uarte_data_t *uarte_d);
int uarte_flash(uarte_data_t *uarte_d);
int uarte_set_baudrate(uarte_data_t *uarte_d, uint8_t baudrate);
/*---------------------------------------------------------------------------*/
extern const struct uart_driver nrf52_uarte0;
/*---------------------------------------------------------------------------*/
extern const struct uart_driver nrf52_uarte1;
/*---------------------------------------------------------------------------*/
#endif