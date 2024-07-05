/**
 * @file uarte.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-05
 *
 * @copyright Copyright (c) 2024
 *
 */
/*---------------------------------------------------------------------------*/
#include "nrf.h"
#include "uarte.h"
#include "nrf_gpio.h"
#include "nrf_uarte.h"
#include "uart-arch.h"
/*---------------------------------------------------------------------------*/
static int
send(uarte_data_t *uarte_d)
{
  bool endtx;
  bool txstopped;

  nrf_uarte_event_clear(uarte_d->peripheral, NRF_UARTE_EVENT_ENDTX);
  nrf_uarte_event_clear(uarte_d->peripheral, NRF_UARTE_EVENT_TXSTOPPED);
  nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STARTTX);
  uarte_d->tx_started = 1;

  do {
    endtx = nrf_uarte_event_check(uarte_d->peripheral, NRF_UARTE_EVENT_ENDTX);
    txstopped = nrf_uarte_event_check(uarte_d->peripheral, NRF_UARTE_EVENT_TXSTOPPED);
  } while ((!endtx) && (!txstopped));

  // Transmitter has to be stopped by triggering the STOPTX task to achieve
  // the lowest possible level of the UARTE power consumption.
  nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STOPTX);
  while(!nrf_uarte_event_check(uarte_d->peripheral, NRF_UARTE_EVENT_TXSTOPPED)) {
  }

  uarte_d->tx_started = 0;

  if(txstopped) {
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static nrf_uarte_baudrate_t
get_nrf_uarte_baudrate(uint8_t baudrate)
{
  switch(baudrate) {
  case UART_RATE_1200:
    return NRF_UARTE_BAUDRATE_1200;
  case UART_RATE_2400:
    return NRF_UARTE_BAUDRATE_2400;
  case UART_RATE_4800:
    return NRF_UARTE_BAUDRATE_4800;
  case UART_RATE_9600:
    return NRF_UARTE_BAUDRATE_9600;
  case UART_RATE_14400:
    return NRF_UARTE_BAUDRATE_14400;
  case UART_RATE_19200:
    return NRF_UARTE_BAUDRATE_19200;
  case UART_RATE_28800:
    return NRF_UARTE_BAUDRATE_28800;
  case UART_RATE_31250:
    return NRF_UARTE_BAUDRATE_31250;
  case UART_RATE_38400:
    return NRF_UARTE_BAUDRATE_38400;
  case UART_RATE_56000:
    return NRF_UARTE_BAUDRATE_56000;
  case UART_RATE_57600:
    return NRF_UARTE_BAUDRATE_57600;
  case UART_RATE_76800:
    return NRF_UARTE_BAUDRATE_76800;
  case UART_RATE_115200:
    return NRF_UARTE_BAUDRATE_115200;
  case UART_RATE_230400:
    return NRF_UARTE_BAUDRATE_230400;
  case UART_RATE_250000:
    return NRF_UARTE_BAUDRATE_250000;
  case UART_RATE_460800:
    return NRF_UARTE_BAUDRATE_460800;
  case UART_RATE_921600:
    return NRF_UARTE_BAUDRATE_921600;
  case UART_RATE_1000000:
    return NRF_UARTE_BAUDRATE_1000000;
  default:
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static void
rx_trigger(uarte_data_t *uarte_d, uint8_t trigger)
{
  if(trigger) {
    nrf_uarte_int_enable(uarte_d->peripheral, NRF_UARTE_INT_ENDRX_MASK);
    nrf_uarte_rx_buffer_set(uarte_d->peripheral, &uarte_d->input_byte, 1);
    nrf_uarte_shorts_enable(uarte_d->peripheral, NRF_UARTE_SHORT_ENDRX_STARTRX);
    nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STARTRX);
    NRFX_IRQ_PRIORITY_SET(nrfx_get_irq_number(uarte_d->peripheral),
                          NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY);
    NRFX_IRQ_ENABLE(nrfx_get_irq_number((void *)uarte_d->peripheral));
    uarte_d->rx_started = 1;
  } else {
    nrf_uarte_shorts_disable(uarte_d->peripheral, NRF_UARTE_SHORT_ENDRX_STARTRX);
    nrf_uarte_int_disable(uarte_d->peripheral, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
    nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STOPRX);
    NRFX_IRQ_DISABLE(nrfx_get_irq_number((void *)uarte_d->peripheral));
    uarte_d->rx_started = 0;
  }
}
/*---------------------------------------------------------------------------*/
int
uarte_set_input(int (*input)(unsigned char c), uarte_data_t *uarte_d)
{
  uarte_d->input_byte_handler = input;

  if(input != NULL) {
    if(uarte_d->enabled) {
      rx_trigger(uarte_d, 1);
    }
  } else {
    rx_trigger(uarte_d, 0);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
uarte_write(uarte_data_t *uarte_d, uint8_t const *p_data, int length)
{
  unsigned char ch;
  int len = length;

  if(!uarte_d->enabled) {
    return 1;
  }

  if(p_data == NULL || !(length > 0)) {
    return -1;
  }

  // EasyDMA requires that transfer buffers are placed in DataRAM. If data not in ram,
  // write each byte individually
  if(!nrfx_is_in_ram(p_data)) {
    while(len--) {
      ch = *p_data++;
      nrf_uarte_tx_buffer_set(uarte_d->peripheral, &ch, 1);
      if(send(uarte_d)) {
        return length - len;
      }
    }
    return length;
  } else {
    nrf_uarte_tx_buffer_set(uarte_d->peripheral, p_data, length);
    return send(uarte_d) == 0 ? length : 0;
  }
}
/*---------------------------------------------------------------------------*/
int
uarte_writeb(uarte_data_t *uarte_d, uint8_t data)
{
  if(!uarte_d->enabled) {
    return 1;
  }
  return uarte_write(uarte_d, &data, 1) != 0 ? 0 : -1;
}
/*---------------------------------------------------------------------------*/
int
uarte_init(uarte_data_t *uarte_d)
{
  if(uarte_d->enabled) {
    nrf_uarte_disable(uarte_d->peripheral);
    nrf_gpio_cfg_output(uarte_d->tx_pin);
    nrf_gpio_pin_set(uarte_d->tx_pin);
    nrf_gpio_cfg_input(uarte_d->rx_pin, NRF_GPIO_PIN_NOPULL);
    nrf_uarte_baudrate_set(uarte_d->peripheral, get_nrf_uarte_baudrate(uarte_d->baudrate));
    nrf_uarte_configure(uarte_d->peripheral, NRF_UARTE_PARITY_EXCLUDED,
                        NRF_UARTE_HWFC_DISABLED);
    nrf_uarte_txrx_pins_set(uarte_d->peripheral, uarte_d->tx_pin, uarte_d->rx_pin);
    nrf_uarte_enable(uarte_d->peripheral);
    if(uarte_d->input_byte_handler) {
      rx_trigger(uarte_d, 1);
    }
    return 0;
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
int
uarte_enable(uarte_data_t *uarte_d)
{
  if(!uarte_d->enabled) {
    uarte_d->enabled = 1;
    return uarte_init(uarte_d);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
uarte_disable(uarte_data_t *uarte_d)
{
  if(uarte_d->enabled) {
    if(uarte_d->tx_started) {
      nrf_uarte_event_clear(uarte_d->peripheral, NRF_UARTE_EVENT_TXSTOPPED);
      nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STOPTX);
      uarte_d->tx_started = 0;
    }
    if(uarte_d->rx_started) {
      nrf_uarte_event_clear(uarte_d->peripheral, NRF_UARTE_EVENT_RXTO);
      nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STOPRX);
      uarte_d->rx_started = 0;
    }
    nrf_uarte_int_disable(uarte_d->peripheral,
                          NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_RXSTARTED_MASK | NRF_UARTE_INT_TXSTARTED_MASK);
    nrf_uarte_disable(uarte_d->peripheral);

    nrf_uarte_txrx_pins_disconnect(uarte_d->peripheral);
    nrf_gpio_cfg_default(uarte_d->tx_pin);
    nrf_gpio_cfg_default(uarte_d->rx_pin);
    uarte_d->enabled = 0;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
int
uarte_active(uarte_data_t *uarte_d)
{
  if(!uarte_d->enabled) {
    return 0;
  }

  return uarte_d->tx_started;
}
/*---------------------------------------------------------------------------*/
int
uarte_flash(uarte_data_t *uarte_d)
{
  if(!uarte_d->enabled) {
    return 0;
  }

  if(uarte_d->rx_started) {
    nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_FLUSHRX);
  }

  if(uarte_d->tx_started) {
    nrf_uarte_event_clear(uarte_d->peripheral, NRF_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_task_trigger(uarte_d->peripheral, NRF_UARTE_TASK_STOPTX);
    while(!nrf_uarte_event_check(uarte_d->peripheral, NRF_UARTE_EVENT_TXSTOPPED)) {
    }
    //Reset Tx buffer to defaults
    uarte_d->tx_started = 0;
    nrf_uarte_tx_buffer_set(uarte_d->peripheral, 0, 0);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
uarte_set_baudrate(uarte_data_t *uarte_d, uint8_t baudrate)
{
  uarte_d->baudrate = baudrate;
  if(uarte_d->enabled) {
    nrf_uarte_baudrate_set(uarte_d->peripheral, get_nrf_uarte_baudrate(uarte_d->baudrate));
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
