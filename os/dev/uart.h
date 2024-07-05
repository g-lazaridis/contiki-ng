/**
 * @file uart.h
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-07-05
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef DEV_UART_H_
#define DEV_UART_H_

/*---------------------------------------------------------------------------*/
struct uart_driver {
  /* Enables and initializes the UART. Returns 0 on success, -1 on failure */
  int
  (*enable)(void);
  /* Disables and powers down (if supported) the UART. Returns 0 on success,
   * -1 on failure */
  int
  (*disable)(void);
  /* Sets the baudrate of the UART. Returns 0 on success, -1 on failure */
  int
  (*set_baudrate)(int baudrate);
  /* Sets the input callback of the UART. Returns 0 on success, -1 on failure */
  int
  (*set_input)(int
               (*input)(unsigned char c));
  /* Initializes the UART. Returns 0 if the UART got initialized, 1 if the
   * UART is disabled or -1 on error */
  int
  (*init)(void);
  /* Returns true if the UART is currently transmitting, false if the UART
   * is either not transmitting or is disabled and -1 on error */
  int
  (*active)(void);
  /* Writes byte c to the UART. Returns 0 on success, 1 if the UART is disabled
   * or -1 on error. */
  int
  (*writeb)(unsigned char c);
  /* Writes a stream of bytes the UART. Returns 0 on success, 1 if the UART is disabled
   * or -1 on error. */
  int
  (*write)(const unsigned char *data, int length);
  /* Flushes both reception and transmission buffers of the UART. If the UART is
   * not enabled, flush will take place once enabled. Returns 0 on success
   * or -1 on error. */
  int
  (*flush)(void);
};
/*---------------------------------------------------------------------------*/
#endif /* DEV_UART_H_ */
