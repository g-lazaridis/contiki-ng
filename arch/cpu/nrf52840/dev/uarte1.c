/**
 * @file uarte1.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-03
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "uart-arch.h"

#include <stdlib.h>
#include <stdint.h>

/*---------------------------------------------------------------------------*/
#if NRF52_ENABLE_UARTE1
static uarte_data_t uarte1_data = {
  .tx_pin = NRF_UARTE1_TX_PIN,
  .rx_pin = NRF_UARTE1_RX_PIN,
  .baudrate = UARTE1_CONF_BAUD_RATE,
  .enabled = 0,
  .tx_started = 0,
  .rx_started = 0,
  .peripheral = NRF_UARTE1,
  .irq_type = UARTE1_IRQn,
};
#endif
/*---------------------------------------------------------------------------*/
static int
uarte1_set_input(int (*input)(unsigned char c))
{
#if NRF52_ENABLE_UARTE1
  return uarte_set_input(input, &uarte1_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_writeb(unsigned char c)
{
#if NRF52_ENABLE_UARTE1
  return uarte_writeb(&uarte1_data, c);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_write(const unsigned char *data, int length)
{
#if NRF52_ENABLE_UARTE1
  return uarte_write(&uarte1_data, data, length);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_init(void)
{
#if NRF52_ENABLE_UARTE1
  return uarte_init(&uarte1_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_enable(void)
{
#if NRF52_ENABLE_UARTE1
  return uarte_enable(&uarte1_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_disable(void)
{
#if NRF52_ENABLE_UARTE1
  return uarte_disable(&uarte1_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_flush(void)
{
#if NRF52_ENABLE_UARTE1
  return uarte_flash(&uarte1_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_active(void)
{
#if NRF52_ENABLE_UARTE1
  return uarte_active(&uarte1_data);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
static int
uarte1_set_baudrate(int baudrate)
{
#if NRF52_ENABLE_UARTE1
  return uarte_set_baudrate(&uarte1_data, baudrate);
#else
  return -1;
#endif
}
/*---------------------------------------------------------------------------*/
#if NRF52_ENABLE_UARTE1
void
UARTE1_IRQHandler(void)
{
  if(nrf_uarte_event_check(NRF_UARTE1, NRF_UARTE_EVENT_ENDRX)) {
    nrf_uarte_event_clear(NRF_UARTE1, NRF_UARTE_EVENT_ENDRX);
    if(uarte1_data.input_byte_handler != NULL) {
      uarte1_data.input_byte_handler(uarte1_data.input_byte);
    }
  }
}
#endif
/*---------------------------------------------------------------------------*/
const struct uart_driver nrf52_uarte1 = {
  .enable = uarte1_enable,
  .disable = uarte1_disable,
  .set_baudrate = uarte1_set_baudrate,
  .set_input = uarte1_set_input,
  .init = uarte1_init,
  .active = uarte1_active,
  .writeb = uarte1_writeb,
  .write = uarte1_write,
  .flush = uarte1_flush,
};
/*---------------------------------------------------------------------------*/