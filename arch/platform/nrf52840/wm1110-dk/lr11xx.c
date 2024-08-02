/**
 * @file lr11xx.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-07-06
 *
 * @copyright Copyright (c) 2024
 *
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
  spi_config.frequency = NRF_SPIM_FREQ_4M;
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

  lr11xx_bootloader_pin_t pin = { 0x00 };
  lr11xx_bootloader_chip_eui_t chip_eui = { 0x00 };
  lr11xx_bootloader_join_eui_t join_eui = { 0x00 };

  lr11xx_bootloader_read_pin(NULL, pin);
  lr11xx_bootloader_read_chip_eui(NULL, chip_eui);
  lr11xx_bootloader_read_join_eui(NULL, join_eui);

  LOG_PRINT("PIN is     0x%02X%02X%02X%02X\n", pin[0], pin[1], pin[2], pin[3]);
  LOG_PRINT("ChipEUI is 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", chip_eui[0], chip_eui[1], chip_eui[2], chip_eui[3],
            chip_eui[4], chip_eui[5], chip_eui[6], chip_eui[7]);
  LOG_PRINT("JoinEUI is 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", join_eui[0], join_eui[1], join_eui[2], join_eui[3],
            join_eui[4], join_eui[5], join_eui[6], join_eui[7]);

  LOG_INFO("Bootloader mode entered, erasing flash\n");
  lr11xx_bootloader_erase_flash(NULL);
  LOG_INFO("Writing new firmware...\n");
  lr11xx_bootloader_write_flash_encrypted_full(NULL, 0, fw_image, image_size);
  LOG_INFO("Firmware written, restarting...\n");
  lr11xx_wait_busy();
  lr11xx_bootloader_reboot(NULL, false);
  lr11xx_wait_busy();
  lr11xx_system_version_t version_trx = { 0x00 };
  lr11xx_system_uid_t uid = { 0x00 };

  lr11xx_system_get_version(NULL, &version_trx);
  LOG_PRINT("Chip in transceiver mode:\n");
  LOG_PRINT(" - Chip type             = 0x%02X\n", version_trx.type);
  LOG_PRINT(" - Chip hardware version = 0x%02X\n", version_trx.hw);
  LOG_PRINT(" - Chip firmware version = 0x%04X\n", version_trx.fw);

  lr11xx_system_read_uid(NULL, uid);
  LOG_INFO("Fetching version.\n");
  if(version.type == 0xdf) {
    LOG_ERR("Still in bootloader mode, fw update failed\n");
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

  irq_config.hi_accuracy = false;
  irq_config.is_watcher = false;
  irq_config.skip_gpio_setup = false;
  irq_config.pull = NRF_GPIO_PIN_PULLDOWN;
  irq_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

  nrf_drv_gpiote_init();
  nrf_drv_gpiote_in_init(LR1110_IRQ_PIN, &irq_config, gpio_irq_handler);
  nrf_drv_gpiote_in_event_enable(LR1110_IRQ_PIN, true);

  nrf_gpio_pin_set(LR1110_NRESET_PIN);
  lr11xx_spi_init();
  system_init();
}
/*---------------------------------------------------------------------------*/