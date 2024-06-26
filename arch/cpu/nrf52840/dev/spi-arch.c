/**
 * @file spi-arch.c
 * @author glazaridis (glazaridis@f-in.eu)
 * @brief
 * @version 0.1
 * @date 2024-06-14
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <nrfx.h>
#include "nrf52840-conf.h"

#if NRFX_CHECK(NRFX_SPI_ENABLED)

#if !(NRFX_CHECK(NRFX_SPI0_ENABLED) || NRFX_CHECK(NRFX_SPI1_ENABLED) || \
      NRFX_CHECK(NRFX_SPI2_ENABLED))
#error "No enabled SPI instances."
#endif

#include "dev/spi.h"
#include "sys/mutex.h"
#include "project-arch.h"
#include "nrf_drv_spi.h"
#include <string.h>
/*---------------------------------------------------------------------------*/
typedef struct {
  mutex_t lock;
  const spi_device_t *owner;
  uint8_t id;
} spi_lock_t;
/*---------------------------------------------------------------------------*/
spi_lock_t spi_lock[] = {
#if (NRFX_CHECK(NRFX_SPI0_ENABLED))
  { MUTEX_STATUS_UNLOCKED, NULL, 0 },
#endif
#if (NRFX_CHECK(NRFX_SPI1_ENABLED))
  { MUTEX_STATUS_UNLOCKED, NULL, 1 },

#endif
#if (NRFX_CHECK(NRFX_SPI2_ENABLED))
  { MUTEX_STATUS_UNLOCKED, NULL, 2 },
#endif
};

static const uint8_t spi_lock_num = sizeof(spi_lock) / sizeof(spi_lock_t);

static const nrf_drv_spi_t spi_instance[] = {
#ifdef NRF_DRV_SPI_INSTANCE_0
  NRF_DRV_SPI_INSTANCE_0,
#else
  { 0, { { 0 } }, 0 },
#endif
#ifdef NRF_DRV_SPI_INSTANCE_1
  NRF_DRV_SPI_INSTANCE_1,
#else
  { 0, { { 0 } }, 0 },
#endif
#ifdef NRF_DRV_SPI_INSTANCE_2
  NRF_DRV_SPI_INSTANCE_2,
#else
  { 0, { { 0 } }, 0 },
#endif
};
/*---------------------------------------------------------------------------*/
static spi_lock_t *
get_lock(uint8_t instance)
{
  for(int i = 0; i < spi_lock_num; i++) {
    if(spi_lock[i].id == instance) {
      return &spi_lock[i];
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
static nrf_drv_spi_frequency_t
get_spi_frequency(uint32_t raw_freq)
{
  switch(raw_freq) {
  case 0:
    //Return the default option
    return NRF_DRV_SPI_FREQ_4M;
  case 125000:
    return NRF_DRV_SPI_FREQ_125K;
  case 250000:
    return NRF_DRV_SPI_FREQ_250K;
  case 500000:
    return NRF_DRV_SPI_FREQ_500K;
  case 1000000:
    return NRF_DRV_SPI_FREQ_1M;
  case 2000000:
    return NRF_DRV_SPI_FREQ_2M;
  case 4000000:
    return NRF_DRV_SPI_FREQ_4M;
  case 8000000:
    return NRF_DRV_SPI_FREQ_8M;
  default:
    return NRF_DRV_SPI_FREQ_4M;
  }
}
/*---------------------------------------------------------------------------*/
static nrf_drv_spi_mode_t
get_spi_mode(uint8_t phase, uint8_t polarity)
{
  if(!polarity && !phase) {
    return NRF_DRV_SPI_MODE_0;
  } else if(!polarity && phase) {
    return NRF_DRV_SPI_MODE_1;
  } else if(polarity && !phase) {
    return NRF_DRV_SPI_MODE_2;
  } else {
    return NRF_DRV_SPI_MODE_3;
  }
}
/*---------------------------------------------------------------------------*/
static void
spi_init(spi_lock_t *s_lock)
{
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

  spi_config.ss_pin = NRF_GPIO_PIN_MAP(s_lock->owner->port_spi_cs, s_lock->owner->pin_spi_cs);
  spi_config.miso_pin = NRF_GPIO_PIN_MAP(s_lock->owner->port_spi_miso, s_lock->owner->pin_spi_miso);
  spi_config.mosi_pin = NRF_GPIO_PIN_MAP(s_lock->owner->port_spi_mosi, s_lock->owner->pin_spi_mosi);
  spi_config.sck_pin = NRF_GPIO_PIN_MAP(s_lock->owner->port_spi_sck, s_lock->owner->pin_spi_sck);

  spi_config.frequency = get_spi_frequency(s_lock->owner->spi_bit_rate);
  spi_config.mode = get_spi_mode(s_lock->owner->spi_pha, s_lock->owner->spi_pol);

  nrf_drv_spi_init(&spi_instance[s_lock->id], &spi_config, NULL, NULL);
}
/*---------------------------------------------------------------------------*/
static void
spi_uninit(spi_lock_t *s_lock)
{
  nrf_drv_spi_uninit(&spi_instance[s_lock->id]);
}
/*---------------------------------------------------------------------------*/
bool
spi_arch_has_lock(const spi_device_t *dev)
{

  spi_lock_t *s_lock = get_lock(dev->spi_controller);

  if(s_lock && s_lock->owner == dev) {
    return true;
  }

  return false;
}
/*---------------------------------------------------------------------------*/
bool
spi_arch_is_bus_locked(const spi_device_t *dev)
{
  spi_lock_t *s_lock = get_lock(dev->spi_controller);
  if(s_lock && s_lock->lock == MUTEX_STATUS_LOCKED) {
    return true;
  }

  return false;
}
/*---------------------------------------------------------------------------*/
spi_status_t
spi_arch_lock_and_open(const spi_device_t *dev)
{

  spi_lock_t *s_lock = get_lock(dev->spi_controller);

  /* Lock the SPI bus */
  if(s_lock == NULL || mutex_try_lock(&s_lock->lock) == false) {
    return SPI_DEV_STATUS_BUS_LOCKED;
  }

  s_lock->owner = dev;

  spi_init(s_lock);

  return SPI_DEV_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
spi_status_t
spi_arch_close_and_unlock(const spi_device_t *dev)
{
  if(!spi_arch_has_lock(dev)) {
    return SPI_DEV_STATUS_BUS_NOT_OWNED;
  }

  spi_lock_t *s_lock = get_lock(dev->spi_controller);

  if(s_lock) {
    spi_uninit(s_lock);

    /* Unlock the SPI bus */
    s_lock->owner = NULL;
    mutex_unlock(&s_lock->lock);
  }

  return SPI_DEV_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
spi_status_t
spi_arch_transfer(const spi_device_t *dev,
                  const uint8_t *write_buf, int wlen,
                  uint8_t *inbuf, int rlen, int ignore_len)
{

  ret_code_t ret;

  if(!spi_arch_has_lock(dev)) {
    return SPI_DEV_STATUS_BUS_NOT_OWNED;
  }

  if(ignore_len) {
    uint8_t temp_buf[rlen + ignore_len];
    ret = nrf_drv_spi_transfer(&spi_instance[dev->spi_controller],
                               write_buf, wlen, temp_buf, rlen + ignore_len);
    if(rlen) {
      memcpy(inbuf, temp_buf, rlen);
    }
  } else {
    ret = nrf_drv_spi_transfer(&spi_instance[dev->spi_controller],
                               write_buf, wlen, inbuf, rlen);
  }

  return ret == NRF_SUCCESS ? SPI_DEV_STATUS_OK : SPI_DEV_STATUS_TRANSFER_ERR;
}
/*---------------------------------------------------------------------------*/
#endif