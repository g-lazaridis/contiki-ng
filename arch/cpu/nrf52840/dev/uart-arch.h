/**
 * @file uart-arch.h
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef UART_ARCH_H
#define UART_ARCH_H
#include "nrf52840-board-def.h"
#include "uarte.h"
#include "nrf_uarte.h"
#include "usb-serial.h"
/*---------------------------------------------------------------------------*/
/* Configure debug interface */
#ifndef DEBUG_UART_BAUDRATE
#define DEBUG_UART_BAUDRATE     UART_RATE_115200
#endif /* DEBUG_UART_BAUDRATE */

#if defined(NRF52_DEBUG_UARTE0)
#undef NRF52_ENABLE_UARTE0
#define NRF52_ENABLE_UARTE0     1
#define DEBUG_UART              nrf52_uarte0
#ifdef  UARTE0_CONF_BAUD_RATE
#undef  UARTE0_CONF_BAUD_RATE
#define UARTE0_CONF_BAUD_RATE DEBUG_UART_BAUDRATE
#endif
#elif defined(NRF52_DEBUG_UARTE1)
#undef NRF52_ENABLE_UARTE1
#define NRF52_ENABLE_UARTE1     1
#define DEBUG_UART              nrf52_uarte1
#ifdef  UARTE1_CONF_BAUD_RATE
#undef  UARTE1_CONF_BAUD_RATE
#define UARTE1_CONF_BAUD_RATE DEBUG_UART_BAUDRATE
#endif
#endif
/*---------------------------------------------------------------------------*/
/* UART baud rates */
#define UART_RATE_1200          0
#define UART_RATE_2400          1
#define UART_RATE_4800          2
#define UART_RATE_9600          3
#define UART_RATE_14400         4
#define UART_RATE_19200         5
#define UART_RATE_28800         6
#define UART_RATE_31250         7
#define UART_RATE_38400         8
#define UART_RATE_56000         9
#define UART_RATE_57600         10
#define UART_RATE_76800         11
#define UART_RATE_115200        12
#define UART_RATE_230400        13
#define UART_RATE_250000        14
#define UART_RATE_460800        15
#define UART_RATE_921600        16
#define UART_RATE_1000000       17
/*---------------------------------------------------------------------------*/
#endif /* UART_ARCH_H */
