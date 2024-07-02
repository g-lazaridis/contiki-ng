/**
 * @file lr11xx_hal.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lr11xx_hal.h"
#include "nrf_drv_spi.h"
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "LR11XX-HAL"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
lr11xx_hal_status_t
lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length,
                 const uint8_t *data, const uint16_t data_length)
{
}
/*---------------------------------------------------------------------------*/