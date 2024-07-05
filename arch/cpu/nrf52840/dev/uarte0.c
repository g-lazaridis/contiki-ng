/**
 * @file uarte0.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-03
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "nrf.h"
#include "uart-arch.h"

#include <stdlib.h>
#include <stdint.h>

/*---------------------------------------------------------------------------*/
#if NRF52_ENABLE_UARTE0
static uarte_data_t uarte0_data = {
  .tx_pin = NRF_UARTE0_TX_PIN,
  .rx_pin = NRF_UARTE0_RX_PIN,
  .baudrate = UARTE0_CONF_BAUD_RATE,
  .enabled = 0,
  .tx_started = 0,
  .rx_started = 0,
  .peripheral = NRF_UARTE0,
  .irq_type = UARTE0_UART0_IRQn,
};
#endif
/*---------------------------------------------------------------------------*/
static int
uarte0_set_input(int (*input)(unsigned char c))
{
#if NRF52_ENABLE_UARTE0
  return uarte_set_input(input, &uarte0_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_writeb(unsigned char c)
{
#if NRF52_ENABLE_UARTE0
  return uarte_writeb(&uarte0_data, c);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_write(const unsigned char *data, int length)
{
#if NRF52_ENABLE_UARTE0
  return uarte_write(&uarte0_data, data, length);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_init(void)
{
#if NRF52_ENABLE_UARTE0
  return uarte_init(&uarte0_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_enable(void)
{
#if NRF52_ENABLE_UARTE0
  return uarte_enable(&uarte0_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_disable(void)
{
#if NRF52_ENABLE_UARTE0
  return uarte_disable(&uarte0_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_flush(void)
{
#if NRF52_ENABLE_UARTE0
  return uarte_flash(&uarte0_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_active(void)
{
#if NRF52_ENABLE_UARTE0
  return uarte_active(&uarte0_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte0_set_baudrate(int baudrate)
{
#if NRF52_ENABLE_UARTE0
  return uarte_set_baudrate(&uarte0_data, baudrate);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
#if NRF52_ENABLE_UARTE0
void
UARTE0_UART0_IRQHandler(void)
{
  if(nrf_uarte_event_check(NRF_UARTE0, NRF_UARTE_EVENT_ENDRX)) {
    nrf_uarte_event_clear(NRF_UARTE0, NRF_UARTE_EVENT_ENDRX);
    if(uarte0_data.input_byte_handler != NULL) {
      uarte0_data.input_byte_handler(uarte0_data.input_byte);
    }
  }
}
#endif
/*---------------------------------------------------------------------------*/
const struct uart_driver nrf52_uarte0 = {
  .enable = uarte0_enable,
  .disable = uarte0_disable,
  .set_baudrate = uarte0_set_baudrate,
  .set_input = uarte0_set_input,
  .init = uarte0_init,
  .active = uarte0_active,
  .writeb = uarte0_writeb,
  .write = uarte0_write,
  .flush = uarte0_flush,
};
/*---------------------------------------------------------------------------*/