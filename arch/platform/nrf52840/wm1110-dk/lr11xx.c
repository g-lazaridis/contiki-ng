/**
 * \file
 *        Board-level control of the LR1110 transceiver on the Wio-WM1110.
 *
 *        Implements the SPI transport, reset/wakeup/busy handling and the
 *        bootloader firmware update path that the Semtech SWDR001 driver
 *        builds on.
 * \author
 *        Gavriil Lazaridis
 */
/*---------------------------------------------------------------------------*/
#include "nrfx_spim.h"
#include "lr11xx_hal.h"
#include "lr11xx_bootloader.h"
#include "lr11xx_bootloader_types.h"
#include "lr11xx_system.h"
#include "lr11xx_system_types.h"
#include "lr11xx_crypto_engine.h"
#include "lr11xx.h"
#include "nrf_gpio.h"
#include "contiki.h"
/*---------------------------------------------------------------------------*/
#include "sys/log.h"
#define LOG_MODULE "LR11XX"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(LR11XX_SPI_INSTANCE);
/*---------------------------------------------------------------------------*/
static void
system_init(void)
{
  lr11xx_system_rfswitch_cfg_t rfswitch_cfg = {
    .enable = LR11XX_SYSTEM_RFSW0_HIGH | LR11XX_SYSTEM_RFSW1_HIGH,
    .standby = 0,
    .rx = LR11XX_SYSTEM_RFSW0_HIGH,
    .tx = LR11XX_SYSTEM_RFSW0_HIGH | LR11XX_SYSTEM_RFSW1_HIGH,
    .tx_hp = LR11XX_SYSTEM_RFSW1_HIGH,
    .tx_hf = 0,
    .gnss = 0,
    .wifi = 0,
  };

  // Configure the regulators
  lr11xx_system_set_reg_mode(NULL, LR11XX_SYSTEM_REG_MODE_DCDC);
  lr11xx_system_set_tcxo_mode(NULL, LR11XX_SYSTEM_TCXO_CTRL_3_3V, 50);

  lr11xx_system_cfg_lfclk(NULL, LR11XX_SYSTEM_LFCLK_XTAL, 1);
  lr11xx_system_set_dio_as_rf_switch(NULL, &rfswitch_cfg);
  lr11xx_system_drive_dio_in_sleep_mode(NULL, true);
  lr11xx_system_clear_errors(NULL);
  lr11xx_system_calibrate(NULL, LR11XX_SYSTEM_CALIB_LF_RC_MASK | LR11XX_SYSTEM_CALIB_HF_RC_MASK | LR11XX_SYSTEM_CALIB_PLL_MASK |
                          LR11XX_SYSTEM_CALIB_ADC_MASK | LR11XX_SYSTEM_CALIB_IMG_MASK |
                          LR11XX_SYSTEM_CALIB_PLL_TX_MASK);
  uint16_t errors;
  lr11xx_system_get_errors(NULL, &errors);
  if(errors) {
    LOG_ERR("System errors on system init : 0x%04X\n", errors);
  }
  lr11xx_system_clear_errors(NULL);
  lr11xx_system_clear_irq_status(NULL, LR11XX_SYSTEM_IRQ_ALL_MASK);
}
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_deinit(void)
{
  nrfx_spim_uninit(&spi);
}
/*---------------------------------------------------------------------------*/
uint16_t
lr11xx_spi_in_out(const uint16_t out_data)
{
  uint8_t tv = 0, rv = 0;
  tv = (uint8_t)(out_data & 0xFF);
  nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX((uint8_t *)(&tv), 1, (uint8_t *)(&rv), 1);
  nrfx_spim_xfer(&spi, &xfer_desc, NRFX_SPIM_FLAG_NO_XFER_EVT_HANDLER);
  return rv;
}
/*---------------------------------------------------------------------------*/
void
lr11xx_spi_init(void)
{
  nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
  spi_config.frequency = NRF_SPIM_FREQ_8M;
  spi_config.mode = NRF_SPIM_MODE_0;
  spi_config.bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST;
  spi_config.miso_pin = LR1110_SPI_MISO_PIN;
  spi_config.mosi_pin = LR1110_SPI_MOSI_PIN;
  spi_config.sck_pin = LR1110_SPI_SCK_PIN;
  spi_config.use_hw_ss = false;
  spi_config.ss_active_high = false;
  nrfx_spim_init(&spi, &spi_config, NULL, NULL);
}
/*---------------------------------------------------------------------------*/
void
lr11xx_enter_bootloader_mode(void)
{
  nrf_gpio_cfg_output(LR1110_BUSY_PIN);
  nrf_gpio_pin_clear(LR1110_BUSY_PIN);
  lr11xx_reset();

  nrf_delay_ms(500);

  /* Set-up GPIOS*/
  nrf_gpio_cfg_input(LR1110_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
  nrf_delay_ms(100);

  lr11xx_wait_busy();
}
/*---------------------------------------------------------------------------*/
int
lr11xx_firmware_update(const uint32_t *fw_image, uint32_t image_size)
{
  lr11xx_bootloader_version_t version;
  lr11xx_enter_bootloader_mode();

  lr11xx_bootloader_get_version(NULL, &version);
  if(version.type != 0xdf) {
    LOG_ERR("Failed to enter bootloader mode\n");
    return -1;
  }

  lr11xx_system_clear_errors(NULL);

  LOG_INFO("Entering bootloader...\n");
  lr11xx_enter_bootloader_mode();

  lr11xx_bootloader_get_version(NULL, &version);

  if(version.type == 0xdf) {
    LOG_INFO("Entered bootloader state!\n");
    LOG_INFO("Erasing flash\n");
    lr11xx_bootloader_erase_flash(NULL);
    LOG_INFO("Writing new firmware...\n");
    lr11xx_bootloader_write_flash_encrypted_full(NULL, 0, fw_image, image_size);
    lr11xx_bootloader_reboot(NULL, false);
    lr11xx_bootloader_get_version(NULL, &version);
    LOG_INFO("Fetching version.\n");
    LOG_INFO("HW Version = %u\n", version.hw);
    LOG_INFO("FW Version = %02u.%02u\n", (uint8_t)(version.fw >> 8), (uint8_t)(version.fw));
    LOG_INFO("Type = %u\n", version.type);
    if(version.type == 0xdf) {
      LOG_ERR("Still in bootloader mode, fw update failed\n");
      return -1;
    }
  } else {
    LOG_INFO("Failed to enter bootloader mode, type = %d\n", version.type);
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
lr11xx_print_status(void)
{
  lr11xx_system_stat1_t stat1;
  lr11xx_system_stat2_t stat2;
  lr11xx_system_irq_mask_t irq_status;

  lr11xx_system_get_status(NULL, &stat1, &stat2, &irq_status);

  LOG_INFO("Command status = %u\n", stat1.command_status);
  LOG_INFO("Irq Active = %u\n", stat1.is_interrupt_active);
  LOG_INFO("Reset status = %u\n", stat2.reset_status);
  LOG_INFO("Chip mode = %u\n", stat2.chip_mode);
  LOG_INFO("Running from flash = %u\n", stat2.is_running_from_flash);
  LOG_INFO("IRQ status = 0x%08lX\n", irq_status);
}
/*---------------------------------------------------------------------------*/
void
lr11xx_init(nrfx_gpiote_evt_handler_t gpio_irq_handler)
{
  nrf_drv_gpiote_in_config_t irq_config;

  /* re-power up LR1110 */
  nrf_gpio_cfg_output(LR1110_NRESET_PIN);
  nrf_gpio_pin_clear(LR1110_NRESET_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_CS_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_CS_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_MISO_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_MISO_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_MOSI_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_MOSI_PIN);

  nrf_gpio_cfg_output(LR1110_SPI_SCK_PIN);
  nrf_gpio_pin_clear(LR1110_SPI_SCK_PIN);

  nrf_gpio_cfg_output(LR1110_BUSY_PIN);
  nrf_gpio_pin_clear(LR1110_BUSY_PIN);

  nrf_delay_ms(500);

  /* Set-up GPIOS*/
  nrf_gpio_pin_set(LR1110_SPI_CS_PIN);
  nrf_gpio_cfg_input(LR1110_BUSY_PIN, NRF_GPIO_PIN_NOPULL);

  if(gpio_irq_handler) {
    irq_config.hi_accuracy = true;
    irq_config.is_watcher = false;
    irq_config.skip_gpio_setup = false;
    irq_config.pull = NRF_GPIO_PIN_PULLDOWN;
    irq_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

    nrf_drv_gpiote_init();
    nrf_drv_gpiote_in_init(LR1110_IRQ_PIN, &irq_config, gpio_irq_handler);
    nrf_drv_gpiote_in_event_enable(LR1110_IRQ_PIN, true);
  }

  nrf_gpio_pin_set(LR1110_NRESET_PIN);
  lr11xx_spi_init();
  system_init();
}
/*---------------------------------------------------------------------------*/