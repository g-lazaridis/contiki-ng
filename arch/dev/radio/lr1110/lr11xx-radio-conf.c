/**
 * \file
 *        Configuration of the LR11xx transceiver.
 * \author
 *        Gavriil Lazaridis
 */

#include "lr11xx_radio_types.h"
#include "lr11xx-radio-conf.h"
#include "lr11xx_radio.h"
#include <stddef.h>

#define LR1110_SUBGHZ_FREQ_MIN 150000000
#define LR1110_SUBGHZ_FREQ_MAX 960000000

#define LR1110_MIN_PWR -17
#define LR1110_MAX_PWR 22

#define LR1110_MIN_PWR_PA_HF -18
#define LR1110_MAX_PWR_PA_HF 13
/*---------------------------------------------------------------------------*/
// PA config table
const lr11xx_radio_conf_pa_pwr_cfg_t pa_cfg_table[LR1110_MAX_PWR - LR1110_MIN_PWR + 1] = {
  { // Expected output power = -17dBm
    .power = -15,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -16dBm
    .power = -14,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -15dBm
    .power = -13,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -14dBm
    .power = -12,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -13dBm
    .power = -11,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -12dBm
    .power = -9,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -11dBm
    .power = -8,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -10dBm
    .power = -7,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -9dBm
    .power = -6,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -8dBm
    .power = -5,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -7dBm
    .power = -4,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -6dBm
    .power = -3,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -5dBm
    .power = -2,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -4dBm
    .power = -1,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -3dBm
    .power = 0,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -2dBm
    .power = 1,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = -1dBm
    .power = 2,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 0dBm
    .power = 3,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 1dBm
    .power = 3,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x01,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 2dBm
    .power = 4,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x01,
      .pa_hp_sel = 0x00,
    },
  },
  {    // Expected output power = 3dBm
    .power = 7,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {    // Expected output power = 4dBm
    .power = 8,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {    // Expected output power = 5dBm
    .power = 9,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {    // Expected output power = 6dBm
    .power = 10,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {    // Expected output power = 7dBm
    .power = 12,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 8dBm
    .power = 13,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 9dBm
    .power = 14,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x00,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 10dBm
    .power = 13,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x01,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 11dBm
    .power = 13,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x02,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 12dBm
    .power = 14,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x02,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 13dBm
    .power = 14,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x03,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 14dBm
    .power = 14,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x04,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 15dBm
    .power = 14,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VREG,
      .pa_duty_cycle = 0x07,
      .pa_hp_sel = 0x00,
    },
  },
  {   // Expected output power = 16dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x01,
      .pa_hp_sel = 0x04,
    },
  },
  {   // Expected output power = 17dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x02,
      .pa_hp_sel = 0x04,
    },
  },
  {   // Expected output power = 18dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x01,
      .pa_hp_sel = 0x06,
    },
  },
  {   // Expected output power = 19dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x03,
      .pa_hp_sel = 0x05,
    },
  },
  {   // Expected output power = 20dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x04,
      .pa_hp_sel = 0x07,
    },
  },
  {   // Expected output power = 21dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x05,
      .pa_hp_sel = 0x07,
    },
  },
  {   // Expected output power = 22dBm
    .power = 22,
    .pa_config = {
      .pa_sel = LR11XX_RADIO_PA_SEL_HP,
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT,
      .pa_duty_cycle = 0x06,
      .pa_hp_sel = 0x07,
    },
  },
};
/*---------------------------------------------------------------------------*/
static const lr11xx_radio_rssi_calibration_table_t rssi_calibration_table_below_600mhz = {
  .gain_offset = 0,
  .gain_tune = { .g4 = 12,
                 .g5 = 12,
                 .g6 = 14,
                 .g7 = 0,
                 .g8 = 1,
                 .g9 = 3,
                 .g10 = 4,
                 .g11 = 4,
                 .g12 = 3,
                 .g13 = 6,
                 .g13hp1 = 6,
                 .g13hp2 = 6,
                 .g13hp3 = 6,
                 .g13hp4 = 6,
                 .g13hp5 = 6,
                 .g13hp6 = 6,
                 .g13hp7 = 6 },
};

static const lr11xx_radio_rssi_calibration_table_t rssi_calibration_table_from_600mhz_to_2ghz = {
  .gain_offset = 0,
  .gain_tune = { .g4 = 2,
                 .g5 = 2,
                 .g6 = 2,
                 .g7 = 3,
                 .g8 = 3,
                 .g9 = 4,
                 .g10 = 5,
                 .g11 = 4,
                 .g12 = 4,
                 .g13 = 6,
                 .g13hp1 = 5,
                 .g13hp2 = 5,
                 .g13hp3 = 6,
                 .g13hp4 = 6,
                 .g13hp5 = 6,
                 .g13hp6 = 7,
                 .g13hp7 = 6 },
};

static const lr11xx_radio_rssi_calibration_table_t rssi_calibration_table_above_2ghz = {
  .gain_offset = 2030,
  .gain_tune = { .g4 = 6,
                 .g5 = 7,
                 .g6 = 6,
                 .g7 = 4,
                 .g8 = 3,
                 .g9 = 4,
                 .g10 = 14,
                 .g11 = 12,
                 .g12 = 14,
                 .g13 = 12,
                 .g13hp1 = 12,
                 .g13hp2 = 12,
                 .g13hp3 = 12,
                 .g13hp4 = 8,
                 .g13hp5 = 8,
                 .g13hp6 = 9,
                 .g13hp7 = 9 },
};
/*---------------------------------------------------------------------------*/
const lr11xx_radio_conf_pa_pwr_cfg_t *
lr11xx_radio_conf_get_pa_pwr_cfg(const uint32_t rf_freq_in_hz,
                                 int8_t expected_output_pwr_in_dbm)
{
  if((LR1110_SUBGHZ_FREQ_MIN <= rf_freq_in_hz) && (rf_freq_in_hz <= LR1110_SUBGHZ_FREQ_MAX)) {
    if((LR1110_MIN_PWR <= expected_output_pwr_in_dbm) &&
       (expected_output_pwr_in_dbm <= LR1110_MAX_PWR)) {
      return &(pa_cfg_table[expected_output_pwr_in_dbm - LR1110_MIN_PWR]);
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
const lr11xx_radio_rssi_calibration_table_t *
lr11xx_radio_conf_get_rssi_calibration_table(const uint32_t freq_in_hz)
{
  if(freq_in_hz < 600000000) {
    return &rssi_calibration_table_below_600mhz;
  } else if((600000000 <= freq_in_hz) && (freq_in_hz <= 2000000000)) {
    return &rssi_calibration_table_from_600mhz_to_2ghz;
  } else if((2000000000 < freq_in_hz) && (freq_in_hz <= 2500000000)) {
    return &rssi_calibration_table_above_2ghz;
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
uint8_t
lr11xx_radio_conf_compute_lora_ldro(const lr11xx_radio_lora_sf_t sf, const lr11xx_radio_lora_bw_t bw)
{
  switch(bw) {
  case LR11XX_RADIO_LORA_BW_500:
    return 0;

  case LR11XX_RADIO_LORA_BW_250:
    if(sf == LR11XX_RADIO_LORA_SF12) {
      return 1;
    } else {
      return 0;
    }

  case LR11XX_RADIO_LORA_BW_800:
  case LR11XX_RADIO_LORA_BW_400:
  case LR11XX_RADIO_LORA_BW_200:
  case LR11XX_RADIO_LORA_BW_125:
    if((sf == LR11XX_RADIO_LORA_SF12) || (sf == LR11XX_RADIO_LORA_SF11)) {
      return 1;
    } else {
      return 0;
    }

  case LR11XX_RADIO_LORA_BW_62:
    if((sf == LR11XX_RADIO_LORA_SF12) || (sf == LR11XX_RADIO_LORA_SF11) || (sf == LR11XX_RADIO_LORA_SF10)) {
      return 1;
    } else {
      return 0;
    }

  case LR11XX_RADIO_LORA_BW_41:
    if((sf == LR11XX_RADIO_LORA_SF12) || (sf == LR11XX_RADIO_LORA_SF11) || (sf == LR11XX_RADIO_LORA_SF10) ||
       (sf == LR11XX_RADIO_LORA_SF9)) {
      return 1;
    } else {
      return 0;
    }

  case LR11XX_RADIO_LORA_BW_31:
  case LR11XX_RADIO_LORA_BW_20:
  case LR11XX_RADIO_LORA_BW_15:
  case LR11XX_RADIO_LORA_BW_10:
    return 1;

  default:
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
uint32_t
get_channel_frequency(uint32_t base_freq_in_hz, radio_channel channel_id,
                      uint32_t channel_gap, lr11xx_radio_lora_bw_t bw)
{

  uint32_t bw_hz = lr11xx_radio_get_lora_bw_in_hz(bw);

  return base_freq_in_hz + (bw_hz + channel_gap) * channel_id;
}
/*---------------------------------------------------------------------------*/