/*!
 * \file      lr11xx_hal.c
 *
 * \brief     Implements the lr11xx radio HAL functions
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include "lr11xx.h"
#include "lr11xx_hal.h"
#include "sys/critical.h"
#include "watchdog.h"
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-HAL"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/

typedef enum {
  RADIO_SLEEP,
  RADIO_AWAKE
} radio_mode_t;

static volatile radio_mode_t radio_mode = RADIO_AWAKE;
/*---------------------------------------------------------------------------*/
static int
lr11xx_hal_check_device_ready(void)
{
  if(radio_mode != RADIO_SLEEP) {
    return lr11xx_wait_busy();
  } else {
    lr11xx_spi_init();
    lr11xx_wake();
    radio_mode = RADIO_AWAKE;
    return lr11xx_wait_busy();
  }
}
/*---------------------------------------------------------------------------*/

lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{

  int_master_status_t status;

#if defined(USE_LR11XX_CRC_OVER_SPI)
  // Compute the CRC over command array first and over data array then
  uint8_t cmd_crc = lr11xx_hal_compute_crc(0xFF, command, command_length);
  cmd_crc = lr11xx_hal_compute_crc(cmd_crc, data, data_length);
#endif

  if(lr11xx_hal_check_device_ready()) {
    LOG_ERR("lr11xx_hal_write: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  status = critical_enter();

  // Put NSS low to start spi transaction
  lr11xx_spi_clear_nss();
  for(uint16_t i = 0; i < command_length; i++) {
    lr11xx_spi_in_out(command[i]);
  }
  for(uint16_t i = 0; i < data_length; i++) {
    lr11xx_spi_in_out(data[i]);
  }

#if defined(USE_LR11XX_CRC_OVER_SPI)
  // Send the CRC byte at the end of the transaction
  lr11xx_spi_in_out(cmd_crc);
#endif

  // Put NSS high as the spi transaction is finished
  lr11xx_spi_set_nss();

  // LR11XX_SYSTEM_SET_SLEEP_OC=0x011B opcode. In sleep mode the radio busy line is held at 1 => do not test it
  if((command[0] == 0x01) && (command[1] == 0x1B)) {
    radio_mode = RADIO_SLEEP;

    // add a incompressible delay to prevent trying to wake the radio before it is full asleep
    clock_delay_usec(500);
  }

  critical_exit(status);

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_read(const void *context, const uint8_t *command, const uint16_t command_length,
                uint8_t *data, const uint16_t data_length)
{
  int_master_status_t status;

#if defined(USE_LR11XX_CRC_OVER_SPI)
  // Compute the CRC over command array
  uint8_t cmd_crc = lr11xx_hal_compute_crc(0xFF, command, command_length);
#endif

  if(lr11xx_hal_check_device_ready()) {
    LOG_ERR("lr11xx_hal_read: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  status = critical_enter();

  // Put NSS low to start spi transaction
  lr11xx_spi_clear_nss();

  for(uint16_t i = 0; i < command_length; i++) {
    lr11xx_spi_in_out(command[i]);
  }

#if defined(USE_LR11XX_CRC_OVER_SPI)
  // Send the CRC byte at the end of the transaction
  lr11xx_spi_in_out(cmd_crc);
#endif

  lr11xx_spi_set_nss();

  if(data_length > 0) {
    lr11xx_hal_check_device_ready();
    lr11xx_spi_clear_nss();

    // dummy read
#if defined(USE_LR11XX_CRC_OVER_SPI)
    // save dummy for crc calculation
    const uint8_t dummy = lr11xx_spi_in_out(LR11XX_NOP);
#else
    lr11xx_spi_in_out(LR11XX_NOP);
#endif

    for(uint16_t i = 0; i < data_length; i++) {
      data[i] = lr11xx_spi_in_out(LR11XX_NOP);
    }

#if defined(USE_LR11XX_CRC_OVER_SPI)
    // read crc sent by lr11xx at the end of the transaction
    const uint8_t rx_crc = lr11xx_spi_in_out(LR11XX_NOP);
#endif

    // Put NSS high as the spi transaction is finished
    lr11xx_spi_set_nss();

#if defined(USE_LR11XX_CRC_OVER_SPI)
    // Check CRC value
    uint8_t computed_crc = lr11xx_hal_compute_crc(0xFF, &dummy, 1);
    computed_crc = lr11xx_hal_compute_crc(computed_crc, data, data_length);
    if(rx_crc != computed_crc) {
      critical_exit(status);
      return LR11XX_HAL_STATUS_ERROR;
    }
#endif
  }

  critical_exit(status);

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_direct_read(const void *radio, uint8_t *data, const uint16_t data_length)
{
  int_master_status_t status;

  if(lr11xx_hal_check_device_ready()) {
    LOG_ERR("lr11xx_hal_direct_read: Device is busy\n");
    return LR11XX_HAL_STATUS_ERROR;
  }

  status = critical_enter();

  // Put NSS low to start spi transaction
  lr11xx_spi_clear_nss();

  for(uint16_t i = 0; i < data_length; i++) {
    data[i] = lr11xx_spi_in_out(LR11XX_NOP);
  }

#if defined(USE_LR11XX_CRC_OVER_SPI)
  // read crc sent by lr11xx by sending one more NOP
  const uint8_t rx_crc = lr11xx_spi_in_out(LR11XX_NOP);
#endif

  lr11xx_spi_set_nss();

#if defined(USE_LR11XX_CRC_OVER_SPI)
  // check crc value
  uint8_t computed_crc = lr11xx_hal_compute_crc(0xFF, data, data_length);
  if(rx_crc != computed_crc) {
    critical_exit(status);
    return LR11XX_HAL_STATUS_ERROR;
  }
#endif

  critical_exit(status);

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_reset(const void *context)
{

  lr11xx_reset();

  // Wait until internal lr11xx fw is ready
  lr11xx_delay_ms(250);

  radio_mode = RADIO_AWAKE;

  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_wakeup(const void *context)
{
  lr11xx_hal_check_device_ready();
  return LR11XX_HAL_STATUS_OK;
}
/*---------------------------------------------------------------------------*/