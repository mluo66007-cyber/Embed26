/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : IMX335 camera + AP3216C light sensor combined demo
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/*
 * Include path style for your current project tree:
 *   Appli/Drivers/BSP/...
 *   Appli/Middlewares/...
 *
 * Required root include paths in STM32CubeIDE:
 *   ${ProjDirPath}/Drivers/BSP
 *   ${ProjDirPath}/Middlewares
 */
#ifdef DEBUG
#include "SYS/sys.h"
#include "HyperRAM/hyperram.h"
#endif
#include "LED/led.h"
#include "BEEP/beep.h"
#include "KEY/key.h"
#include "UART/uart.h"
#include "RGBLCD/rgblcd.h"
#include "SD_NAND/sd_nand.h"
#include "SD_CARD/sd_card.h"
#include "IMX335/imx335.h"
#include "AP3216C/ap3216c.h"

#include "MALLOC/malloc.h"
#include "FATFS/FatFs/source/ff.h"
#include "FATFS/exfuns/exfuns.h"
#include "PICTURE/bmp.h"
#include "PICTURE/piclib.h"
#include "ADC/adc.h"

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DIRECTORY_PATH "0:/Camera/"

/* TIM4 clock from your standalone servo PWM test.
 * 400 MHz / 400 = 1 MHz timer counter clock, so CCR value = pulse width in us.
 */
#define TIM4_INPUT_CLOCK_MHZ          400U

/*
 * TIM4 CH4 PWM is used to drive an SG90-style shade servo.
 * Required waveform:
 *   50 Hz period = 20 ms
 *   0.5~2.5 ms high-level pulse controls angle
 * In MX_TIM4_Init(), TIM4 is configured so CCR value = pulse width in us.
 */
#define SERVO_MIN_US                  500U
#define SERVO_MID_US                  1500U
#define SERVO_MAX_US                  2500U

#define SHADE_OPEN_US                 700U
#define SHADE_CLOSE_US                2200U

#define ALS_SHADE_OPEN_THRESHOLD      600U
#define ALS_SHADE_CLOSE_THRESHOLD     1200U
#define SHADE_STATE_OPEN              0U
#define SHADE_STATE_CLOSED            1U

#define SGP30_I2C_ADDR                (0x58U << 1)
#define SGP30_CMD_INIT_AIR_QUALITY    0x2003U
#define SGP30_CMD_MEASURE_AIR_QUALITY 0x2008U
#define SGP30_I2C_TIMEOUT_MS          100U

/* PH6 GPIO output is used to drive the fan control module.
 * Current setting assumes active-low control:
 *   PH6 = RESET -> fan ON
 *   PH6 = SET   -> fan OFF
 * If your module is active-high, swap FAN_ACTIVE_LEVEL and FAN_INACTIVE_LEVEL.
 */
#define FAN_GPIO_PORT                 GPIOH
#define FAN_GPIO_PIN                  GPIO_PIN_6
#define FAN_ACTIVE_LEVEL              GPIO_PIN_RESET
#define FAN_INACTIVE_LEVEL            GPIO_PIN_SET
#define FAN_STATE_OFF                 0U
#define FAN_STATE_ON                  1U

/* PQ2 GPIO output is used to drive the water pump control module.
 * Requirement: high level starts the pump.
 *   PQ2 = SET   -> pump ON
 *   PQ2 = RESET -> pump OFF
 */
#define PUMP_GPIO_PORT                GPIOQ
#define PUMP_GPIO_PIN                 GPIO_PIN_2
#define PUMP_ACTIVE_LEVEL             GPIO_PIN_SET
#define PUMP_INACTIVE_LEVEL           GPIO_PIN_RESET
#define PUMP_STATE_OFF                0U
#define PUMP_STATE_ON                 1U

/* Fan control uses two conditions with hysteresis:
 *   eCO2 too high OR temperature too high -> fan ON.
 *   eCO2 back low AND temperature back low -> fan OFF.
 */
#define CO2_FAN_ON_THRESHOLD_PPM      1000U
#define CO2_FAN_OFF_THRESHOLD_PPM     800U
#define TEMP_FAN_ON_THRESHOLD_C_X100  3000   /* 30.00 C */
#define TEMP_FAN_OFF_THRESHOLD_C_X100 2800   /* 28.00 C */

/* Soil moisture pump control.
 * If moisture is too low, pump turns ON.
 * When moisture rises enough, pump turns OFF.
 */
#define MOISTURE_PUMP_ON_THRESHOLD_PERCENT    35U
#define MOISTURE_PUMP_OFF_THRESHOLD_PERCENT   45U


/* PQ6 is used as DS18B20 1-Wire data pin. Hardware already has a pull-up resistor. */
#define DS18B20_GPIO_PORT             GPIOQ
#define DS18B20_GPIO_PIN              GPIO_PIN_6
#define DS18B20_TEMP_INVALID_C_X100   ((int16_t)-10000)

/* Soil moisture percent calibration. Adjust these two values after real dry/wet tests.
 * Most common resistive soil modules: dry -> higher ADC raw, wet -> lower ADC raw.
 */
#define MOISTURE_DRY_RAW              3600U
#define MOISTURE_WET_RAW              1500U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DMA2D_HandleTypeDef hdma2d;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c4;

LTDC_HandleTypeDef hltdc;

SD_HandleTypeDef hsd1;
SD_HandleTypeDef hsd2;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

XSPI_HandleTypeDef hxspi1;

/* USER CODE BEGIN PV */
#ifdef DEBUG
static HyperRAM_ObjectTypeDef HyperRAMObject = {0};
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
static void MX_DMA2D_Init(void);
static void MX_LTDC_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_SDMMC2_SD_Init(void);
static void MX_I2C4_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM4_Init(void);
static void SystemIsolation_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void lcd_clear_simple(uint16_t color)
{
  uint32_t i;
  uint32_t pixels = rgblcddev.width * rgblcddev.height;
  uint16_t *p = (uint16_t *)g_ltdc_lcd_framebuf;

  for (i = 0; i < pixels; i++)
  {
    p[i] = color;
  }
}

static const char *shade_state_text(uint8_t shade_state)
{
  return (shade_state == SHADE_STATE_CLOSED) ? "CLOSED" : "OPEN";
}

static const char *fan_state_text(uint8_t fan_state)
{
  return (fan_state == FAN_STATE_ON) ? "ON" : "OFF";
}

static const char *pump_state_text(uint8_t pump_state)
{
  return (pump_state == PUMP_STATE_ON) ? "ON" : "OFF";
}

static uint8_t soil_moisture_raw_to_percent(uint32_t raw)
{
  int32_t dry = (int32_t)MOISTURE_DRY_RAW;
  int32_t wet = (int32_t)MOISTURE_WET_RAW;
  int32_t percent;

  if (dry == wet)
  {
    return 0;
  }

  /* Default formula: dry raw is high, wet raw is low. */
  percent = (dry - (int32_t)raw) * 100 / (dry - wet);

  if (percent < 0)
  {
    percent = 0;
  }
  else if (percent > 100)
  {
    percent = 100;
  }

  return (uint8_t)percent;
}

static void format_temp_c(char *buf, int16_t temp_c_x100, uint8_t temp_valid)
{
  int16_t whole;
  int16_t frac;

  if (!temp_valid)
  {
    sprintf(buf, "TEMP  : ERR");
    return;
  }

  whole = temp_c_x100 / 100;
  frac = temp_c_x100 % 100;
  if (frac < 0)
  {
    frac = -frac;
  }

  sprintf(buf, "TEMP  : %d.%02d C", whole, frac);
}

static void show_sensor_page(uint16_t ir, uint16_t ps, uint16_t als,
                             uint32_t moisture_raw, uint8_t moisture_percent,
                             int16_t temp_c_x100, uint8_t temp_valid,
                             uint16_t eco2_ppm, uint16_t tvoc_ppb, uint8_t co2_valid,
                             uint8_t fan_state, uint8_t pump_state, uint8_t shade_state)
{
  char buf[64];

  lcd_clear_simple(BLACK);

  rgblcd_show_string(30, 30, 420, 16, 16, "SENSOR DATA", RED);

  sprintf(buf, "IR    : %5u raw", ir);
  rgblcd_show_string(30, 55, 420, 16, 16, buf, BLUE);

  sprintf(buf, "PS    : %5u raw", ps);
  rgblcd_show_string(30, 75, 420, 16, 16, buf, BLUE);

  sprintf(buf, "ALS   : %5u lux", als);
  rgblcd_show_string(30, 95, 420, 16, 16, buf, BLUE);

  sprintf(buf, "MOIST : %3u %%", moisture_percent);
  rgblcd_show_string(30, 115, 420, 16, 16, buf, BLUE);

  format_temp_c(buf, temp_c_x100, temp_valid);
  rgblcd_show_string(30, 135, 420, 16, 16, buf, BLUE);

  if (co2_valid)
  {
    sprintf(buf, "eCO2  : %5u ppm", eco2_ppm);
  }
  else
  {
    sprintf(buf, "eCO2  : ERR");
  }
  rgblcd_show_string(30, 155, 420, 16, 16, buf, BLUE);

  if (co2_valid)
  {
    sprintf(buf, "TVOC  : %5u ppb", tvoc_ppb);
  }
  else
  {
    sprintf(buf, "TVOC  : ERR");
  }
  rgblcd_show_string(30, 175, 420, 16, 16, buf, BLUE);

  sprintf(buf, "FAN   : %s", fan_state_text(fan_state));
  rgblcd_show_string(30, 195, 420, 16, 16, buf, GREEN);

  sprintf(buf, "PUMP  : %s", pump_state_text(pump_state));
  rgblcd_show_string(30, 215, 420, 16, 16, buf, GREEN);

  sprintf(buf, "SHADE : %s", shade_state_text(shade_state));
  rgblcd_show_string(30, 235, 420, 16, 16, buf, GREEN);

  rgblcd_show_string(30, 255, 420, 16, 16, "KEY1: Back to Camera", GREEN);
}

static void servo_set_pulse_us(uint16_t pulse_us)
{
  if (pulse_us < SERVO_MIN_US)
  {
    pulse_us = SERVO_MIN_US;
  }
  else if (pulse_us > SERVO_MAX_US)
  {
    pulse_us = SERVO_MAX_US;
  }

  /* With TIM4 configured to 1 MHz counter clock, CCR value = pulse width in us. */
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, pulse_us);
}

static void shade_set_state(uint8_t shade_state)
{
  if (shade_state == SHADE_STATE_CLOSED)
  {
    servo_set_pulse_us(SHADE_CLOSE_US);
  }
  else
  {
    servo_set_pulse_us(SHADE_OPEN_US);
  }
}

static void shade_update_by_als(uint16_t als, uint8_t *shade_state)
{
  if ((als >= ALS_SHADE_CLOSE_THRESHOLD) && (*shade_state != SHADE_STATE_CLOSED))
  {
    *shade_state = SHADE_STATE_CLOSED;
    shade_set_state(*shade_state);
  }
  else if ((als <= ALS_SHADE_OPEN_THRESHOLD) && (*shade_state != SHADE_STATE_OPEN))
  {
    *shade_state = SHADE_STATE_OPEN;
    shade_set_state(*shade_state);
  }
}

static void fan_set_state(uint8_t fan_state)
{
  if (fan_state == FAN_STATE_ON)
  {
    HAL_GPIO_WritePin(FAN_GPIO_PORT, FAN_GPIO_PIN, FAN_ACTIVE_LEVEL);
  }
  else
  {
    HAL_GPIO_WritePin(FAN_GPIO_PORT, FAN_GPIO_PIN, FAN_INACTIVE_LEVEL);
  }
}

static void fan_update_by_air(uint16_t eco2_ppm, uint8_t co2_valid,
                              int16_t temp_c_x100, uint8_t temp_valid,
                              uint8_t *fan_state)
{
  uint8_t co2_high = 0;
  uint8_t temp_high = 0;
  uint8_t co2_low_enough = 1;
  uint8_t temp_low_enough = 1;

  /* If both readings are invalid, keep current fan state. */
  if ((!co2_valid) && (!temp_valid))
  {
    return;
  }

  if (co2_valid)
  {
    co2_high = (eco2_ppm >= CO2_FAN_ON_THRESHOLD_PPM) ? 1U : 0U;
    co2_low_enough = (eco2_ppm <= CO2_FAN_OFF_THRESHOLD_PPM) ? 1U : 0U;
  }

  if (temp_valid)
  {
    temp_high = (temp_c_x100 >= TEMP_FAN_ON_THRESHOLD_C_X100) ? 1U : 0U;
    temp_low_enough = (temp_c_x100 <= TEMP_FAN_OFF_THRESHOLD_C_X100) ? 1U : 0U;
  }

  /* OR logic for turning on: either CO2 or temperature can request the fan. */
  if ((co2_high || temp_high) && (*fan_state != FAN_STATE_ON))
  {
    *fan_state = FAN_STATE_ON;
    fan_set_state(*fan_state);
  }
  /* AND logic for turning off: both valid/available conditions must be safe. */
  else if ((co2_low_enough && temp_low_enough) && (*fan_state != FAN_STATE_OFF))
  {
    *fan_state = FAN_STATE_OFF;
    fan_set_state(*fan_state);
  }
}

static void pump_set_state(uint8_t pump_state)
{
  if (pump_state == PUMP_STATE_ON)
  {
    HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN, PUMP_ACTIVE_LEVEL);
  }
  else
  {
    HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN, PUMP_INACTIVE_LEVEL);
  }
}

static void pump_update_by_moisture(uint8_t moisture_percent, uint8_t *pump_state)
{
  if ((moisture_percent <= MOISTURE_PUMP_ON_THRESHOLD_PERCENT) && (*pump_state != PUMP_STATE_ON))
  {
    *pump_state = PUMP_STATE_ON;
    pump_set_state(*pump_state);
  }
  else if ((moisture_percent >= MOISTURE_PUMP_OFF_THRESHOLD_PERCENT) && (*pump_state != PUMP_STATE_OFF))
  {
    *pump_state = PUMP_STATE_OFF;
    pump_set_state(*pump_state);
  }
}

static uint8_t sgp30_crc8(const uint8_t *data, uint8_t len)
{
  uint8_t crc = 0xFF;
  uint8_t i;
  uint8_t bit;

  for (i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (bit = 0; bit < 8; bit++)
    {
      if (crc & 0x80U)
      {
        crc = (uint8_t)((crc << 1) ^ 0x31U);
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  return crc;
}

static HAL_StatusTypeDef sgp30_write_command(uint16_t cmd)
{
  uint8_t tx[2];

  tx[0] = (uint8_t)(cmd >> 8);
  tx[1] = (uint8_t)(cmd & 0xFFU);

  return HAL_I2C_Master_Transmit(&hi2c1, SGP30_I2C_ADDR, tx, 2, SGP30_I2C_TIMEOUT_MS);
}

static uint8_t sgp30_init(void)
{
  if (HAL_I2C_IsDeviceReady(&hi2c1, SGP30_I2C_ADDR, 3, SGP30_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return 1;
  }

  if (sgp30_write_command(SGP30_CMD_INIT_AIR_QUALITY) != HAL_OK)
  {
    return 2;
  }

  HAL_Delay(20);
  return 0;
}

static uint8_t sgp30_measure_air_quality(uint16_t *eco2_ppm, uint16_t *tvoc_ppb)
{
  uint8_t rx[6];
  uint8_t crc_data[2];

  if (sgp30_write_command(SGP30_CMD_MEASURE_AIR_QUALITY) != HAL_OK)
  {
    return 1;
  }

  HAL_Delay(15);

  if (HAL_I2C_Master_Receive(&hi2c1, SGP30_I2C_ADDR, rx, 6, SGP30_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return 2;
  }

  crc_data[0] = rx[0];
  crc_data[1] = rx[1];
  if (sgp30_crc8(crc_data, 2) != rx[2])
  {
    return 3;
  }

  crc_data[0] = rx[3];
  crc_data[1] = rx[4];
  if (sgp30_crc8(crc_data, 2) != rx[5])
  {
    return 4;
  }

  *eco2_ppm = (uint16_t)(((uint16_t)rx[0] << 8) | rx[1]);
  *tvoc_ppb = (uint16_t)(((uint16_t)rx[3] << 8) | rx[4]);

  return 0;
}

static uint32_t soil_moisture_read_raw(void)
{
  /*
   * PF6 is configured as ADC1_INP15 in CubeMX.
   * The ATK BSP adc_get_result_average() uses hadc1 and the regular rank
   * configured by MX_ADC1_Init(), so the channel argument is kept mainly
   * for readability/compatibility with the BSP API.
   */
  return adc_get_result_average(ADC_CHANNEL_15, 10);
}


static void delay_us_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = (SystemCoreClock / 1000000U) * us;

  while ((DWT->CYCCNT - start) < cycles)
  {
    __NOP();
  }
}

static void ds18b20_release_bus(void)
{
  HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);
}

static void ds18b20_pull_low(void)
{
  HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_RESET);
}

static uint8_t ds18b20_reset(void)
{
  uint8_t presence;

  ds18b20_release_bus();
  delay_us(5);

  ds18b20_pull_low();
  delay_us(480);

  ds18b20_release_bus();
  delay_us(70);

  presence = (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN) == GPIO_PIN_RESET) ? 1U : 0U;

  delay_us(410);
  return presence;
}

static void ds18b20_write_bit(uint8_t bit)
{
  if (bit)
  {
    ds18b20_pull_low();
    delay_us(6);
    ds18b20_release_bus();
    delay_us(64);
  }
  else
  {
    ds18b20_pull_low();
    delay_us(60);
    ds18b20_release_bus();
    delay_us(10);
  }
}

static uint8_t ds18b20_read_bit(void)
{
  uint8_t bit;

  ds18b20_pull_low();
  delay_us(3);
  ds18b20_release_bus();
  delay_us(10);

  bit = (HAL_GPIO_ReadPin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN) == GPIO_PIN_SET) ? 1U : 0U;

  delay_us(53);
  return bit;
}

static void ds18b20_write_byte(uint8_t data)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    ds18b20_write_bit(data & 0x01U);
    data >>= 1;
  }
}

static uint8_t ds18b20_read_byte(void)
{
  uint8_t i;
  uint8_t data = 0;

  for (i = 0; i < 8; i++)
  {
    if (ds18b20_read_bit())
    {
      data |= (uint8_t)(1U << i);
    }
  }

  return data;
}

static uint8_t ds18b20_crc8(const uint8_t *data, uint8_t len)
{
  uint8_t crc = 0;
  uint8_t i;
  uint8_t j;

  for (i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (j = 0; j < 8; j++)
    {
      if (crc & 0x01U)
      {
        crc = (uint8_t)((crc >> 1) ^ 0x8CU);
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc;
}

static uint8_t ds18b20_start_conversion(void)
{
  if (!ds18b20_reset())
  {
    return 1;
  }

  ds18b20_write_byte(0xCCU);  /* Skip ROM, single DS18B20 on the bus. */
  ds18b20_write_byte(0x44U);  /* Convert T. */
  return 0;
}

static uint8_t ds18b20_read_temperature_c_x100(int16_t *temp_c_x100)
{
  uint8_t scratchpad[9];
  uint8_t i;
  int16_t raw;
  int32_t temp;

  if (!ds18b20_reset())
  {
    return 1;
  }

  ds18b20_write_byte(0xCCU);  /* Skip ROM. */
  ds18b20_write_byte(0xBEU);  /* Read scratchpad. */

  for (i = 0; i < 9; i++)
  {
    scratchpad[i] = ds18b20_read_byte();
  }

  if (ds18b20_crc8(scratchpad, 8) != scratchpad[8])
  {
    return 2;
  }

  raw = (int16_t)(((uint16_t)scratchpad[1] << 8) | scratchpad[0]);

  /* DS18B20 raw unit is 1/16 degC at 12-bit resolution. */
  temp = ((int32_t)raw * 100) / 16;

  if (temp < -5500 || temp > 12500)
  {
    return 3;
  }

  *temp_c_x100 = (int16_t)temp;
  return 0;
}

static uint8_t ds18b20_init(void)
{
  delay_us_init();
  ds18b20_release_bus();
  HAL_Delay(10);

  if (!ds18b20_reset())
  {
    return 1;
  }

  return ds18b20_start_conversion();
}

static void ds18b20_update_temperature(int16_t *temp_c_x100, uint8_t *temp_valid)
{
  if (ds18b20_read_temperature_c_x100(temp_c_x100) == 0)
  {
    *temp_valid = 1;
  }
  else
  {
    *temp_valid = 0;
  }

  /* Start next conversion. At 12-bit resolution, read it on the next ~1 s update. */
  (void)ds18b20_start_conversion();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  FRESULT res;
  DIR directory;
  char *file_path;
  uint32_t file_index = 0;
  FIL file;
  uint8_t t = 0;
  uint8_t ap_tick = 0;
  uint8_t sgp30_sample_div = 0;
  uint8_t ds18b20_sample_div = 0;
  uint8_t key;
  uint8_t display_mode = 0;   /* 0: camera live view, 1: AP3216C data page */
  uint8_t shade_state = SHADE_STATE_OPEN;
  uint8_t fan_state = FAN_STATE_OFF;
  uint8_t pump_state = PUMP_STATE_OFF;
  uint16_t ir = 0;
  uint16_t als = 0;
  uint16_t ps = 0;
  uint32_t moisture_raw = 0;
  uint8_t moisture_percent = 0;
  int16_t temp_c_x100 = DS18B20_TEMP_INVALID_C_X100;
  uint8_t temp_valid = 0;
  uint8_t ds18b20_ok = 0;
  uint8_t sgp30_ok = 0;
  uint8_t co2_valid = 0;
  uint16_t eco2_ppm = 0;
  uint16_t tvoc_ppb = 0;

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* USER CODE BEGIN Init */


  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */
#ifdef DEBUG
  MX_XSPI1_Init();
  HyperRAM_Init(&HyperRAMObject, &hxspi1);
  HyperRAM_EnableMemoryMappedMode(&HyperRAMObject);
#endif

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA2D_Init();
  MX_LTDC_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_SDMMC1_SD_Init();
  MX_SDMMC2_SD_Init();
  MX_I2C4_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  PeriphCommonClock_Config();
  MX_TIM4_Init();
  SystemIsolation_Config();
  /* USER CODE BEGIN 2 */
  led_init();               /* 初始化LED */
  beep_init();              /* 初始化蜂鸣器 */
  key_init();               /* 初始化按键 */
  uart_init(115200);        /* 初始化串口 */
  rgblcd_init();            /* 初始化RGB LCD */
  rgblcd_display_dir(1);    /* 设置RGB LCD显示方向 */
  my_mem_init(SRAMIN);      /* 初始化AXISRAM1~2内存池 */
  my_mem_init(SRAMEX);      /* 初始化XSPI1 HyperRAM内存池 */
  exfuns_init();            /* 初始化文件系统 */
  f_mount(fs[0], "0:", 1);  /* 挂载SD卡 */
  piclib_init();            /* 初始化画图 */

  rgblcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
  rgblcd_show_string(30, 70, 200, 16, 16, "Camera", RED);
  rgblcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
  while (imx335_init())
  {
    rgblcd_show_string(30, 110, 200, 16, 16, "IMX335 Error!", RED);
    HAL_Delay(500);
    rgblcd_show_string(30, 110, 200, 16, 16, "Please Check!", RED);
    HAL_Delay(500);
    LED0_TOGGLE();
  }
  rgblcd_show_string(30, 110, 200, 16, 16, "IMX335 OK!   ", RED);

  while (ap3216c_init())
  {
      rgblcd_show_string(30, 130, 240, 16, 16, "AP3216C Error!", RED);
      HAL_Delay(500);
      rgblcd_show_string(30, 130, 240, 16, 16, "Please Check! ", RED);
      HAL_Delay(500);
      LED0_TOGGLE();
  }
  rgblcd_show_string(30, 130, 240, 16, 16, "AP3216C OK!   ", RED);

  if (sgp30_init() == 0)
  {
    sgp30_ok = 1;
    rgblcd_show_string(30, 150, 240, 16, 16, "SGP30 OK!      ", RED);
  }
  else
  {
    sgp30_ok = 0;
    co2_valid = 0;
    rgblcd_show_string(30, 150, 240, 16, 16, "SGP30 Error!   ", RED);
  }

  /* Default fan state: OFF. PH6 output should already be initialized by MX_GPIO_Init(). */
  fan_set_state(FAN_STATE_OFF);

  /* Default pump state: OFF. PQ2 is active-high, so output low means OFF. */
  pump_set_state(PUMP_STATE_OFF);

  if (ds18b20_init() == 0)
  {
    ds18b20_ok = 1;
  }
  else
  {
    ds18b20_ok = 0;
    temp_valid = 0;
  }

  /* Start SG90 PWM output */
  if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  /* Force servo to initial open position */
  shade_set_state(SHADE_STATE_OPEN);
  rgblcd_show_string(30, 170, 300, 16, 16, "KEY0: Capture", RED);
  rgblcd_show_string(30, 190, 300, 16, 16, "KEY1: Sensor Page", RED);

  res = f_opendir(&directory, DIRECTORY_PATH);
  if (res != FR_OK)
  {
    res = f_mkdir(DIRECTORY_PATH);
    while (res != FR_OK)
    {
      rgblcd_show_string(30, 130, 200, 16, 16, "Camera directory error!", RED);
      HAL_Delay(500);
      rgblcd_show_string(30, 130, 200, 16, 16, "Please Check!          ", RED);
      HAL_Delay(500);
      LED0_TOGGLE();
    }
  }
  f_closedir(&directory);

  file_path = (char *)mymalloc(SRAMEX, FF_MAX_LFN * 2 + 1);
  while (file_path == NULL)
  {
    rgblcd_show_string(30, 130, 200, 16, 16, "Output of memory!", RED);
    HAL_Delay(500);
    rgblcd_show_string(30, 130, 200, 16, 16, "Please Check!    ", RED);
    HAL_Delay(500);
    LED0_TOGGLE();
  }

  rgblcd_show_string(30, 150, 300, 16, 16, "KEY0: Capture", RED);
  rgblcd_show_string(30, 170, 300, 16, 16, "KEY1: Sensor Page", RED);

  HAL_Delay(2000);

  imx335_start_capture((uint32_t)g_ltdc_lcd_framebuf);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    for (file_index = 0; file_index < 0xFFFFFFFF; file_index++)
    {
      sprintf(file_path, "%sIMG_%lu.bmp", DIRECTORY_PATH, (unsigned long)file_index);
      res = f_open(&file, file_path, FA_READ);
      if (res == FR_NO_FILE)
      {
        break;
      }
      else if (res == FR_OK)
      {
        f_close(&file);
      }
    }

    while (1)
    {
      key = key_scan(0);

      /* KEY1: switch between camera live view and AP3216C data page */
      if (key == KEY1_PRES)
      {
        if (display_mode == 0)
        {
          display_mode = 1;
          imx335_stop_capture();

          ap3216c_read_data(&ir, &ps, &als);
          moisture_raw = soil_moisture_read_raw();
          moisture_percent = soil_moisture_raw_to_percent(moisture_raw);
          if (ds18b20_ok)
          {
            ds18b20_update_temperature(&temp_c_x100, &temp_valid);
          }
          if (sgp30_ok && (sgp30_measure_air_quality(&eco2_ppm, &tvoc_ppb) == 0))
          {
            co2_valid = 1;
          }
          else if (sgp30_ok)
          {
            co2_valid = 0;
          }
          fan_update_by_air(eco2_ppm, co2_valid, temp_c_x100, temp_valid, &fan_state);
          pump_update_by_moisture(moisture_percent, &pump_state);
          shade_update_by_als(als, &shade_state);
          show_sensor_page(ir, ps, als, moisture_raw, moisture_percent,
                           temp_c_x100, temp_valid,
                           eco2_ppm, tvoc_ppb, co2_valid,
                           fan_state, pump_state, shade_state);
        }
        else
        {
          display_mode = 0;
          lcd_clear_simple(BLACK);
          HAL_Delay(50);
          imx335_start_capture((uint32_t)g_ltdc_lcd_framebuf);
        }

        HAL_Delay(250);   /* simple key debounce */
      }

      /* KEY0: capture image, only in camera mode */
      if ((display_mode == 0) && (key == KEY0_PRES))
      {
        imx335_stop_capture();
        bmp_encode((uint8_t *)file_path, 0, 0, rgblcddev.width, rgblcddev.height, 0);
        piclib_ai_load_picfile(file_path, 0, 0, rgblcddev.width, rgblcddev.height, 0);
        rgblcd_show_string(30, 110, 200, 16, 16, file_path, RED);

        BEEP(1);
        HAL_Delay(100);
        BEEP(0);
        HAL_Delay(1000);

        imx335_start_capture((uint32_t)g_ltdc_lcd_framebuf);
        break;
      }

      /* Camera mode: keep ISP background process running */
      if (display_mode == 0)
      {
        if (imx335_isp_background_process() != 0)
        {
          LED1_TOGGLE();
        }
      }

      /* Update AP3216C data about every 500 ms */
      if (++ap_tick >= 50)
      {
        ap_tick = 0;

        ap3216c_read_data(&ir, &ps, &als);
        moisture_raw = soil_moisture_read_raw();
        moisture_percent = soil_moisture_raw_to_percent(moisture_raw);

        if (ds18b20_ok)
        {
          if (++ds18b20_sample_div >= 2)
          {
            ds18b20_sample_div = 0;
            ds18b20_update_temperature(&temp_c_x100, &temp_valid);
          }
        }

        if (sgp30_ok)
        {
          if (++sgp30_sample_div >= 2)
          {
            sgp30_sample_div = 0;
            if (sgp30_measure_air_quality(&eco2_ppm, &tvoc_ppb) == 0)
            {
              co2_valid = 1;
            }
            else
            {
              co2_valid = 0;
            }
          }
        }

        fan_update_by_air(eco2_ppm, co2_valid, temp_c_x100, temp_valid, &fan_state);
        pump_update_by_moisture(moisture_percent, &pump_state);
        shade_update_by_als(als, &shade_state);

        printf("IR:%u raw  PS:%u raw  ALS:%u lux  Moisture:%u%% raw:%lu  Temp:%d.%02d C  eCO2:%u ppm  TVOC:%u ppb  Fan:%s  Pump:%s  Shade:%s\r\n",
               ir, ps, als, moisture_percent, (unsigned long)moisture_raw,
               temp_c_x100 / 100,
               (temp_c_x100 < 0 ? -(temp_c_x100 % 100) : (temp_c_x100 % 100)),
               eco2_ppm, tvoc_ppb,
               fan_state_text(fan_state), pump_state_text(pump_state), shade_state_text(shade_state));

        if (display_mode == 1)
        {
          show_sensor_page(ir, ps, als, moisture_raw, moisture_percent,
                           temp_c_x100, temp_valid,
                           eco2_ppm, tvoc_ppb, co2_valid,
                           fan_state, pump_state, shade_state);
        }
      }

      if (++t == 20)
      {
        t = 0;
        LED0_TOGGLE();
      }

      HAL_Delay(10);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM|RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPCLKSOURCE_HSI;
  PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M_PFC;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 255;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10707DBC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10707DBC;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x10707DBC;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 0;
  hltdc.Init.VerticalSync = 0;
  hltdc.Init.AccumulatedHBP = 40;
  hltdc.Init.AccumulatedVBP = 8;
  hltdc.Init.AccumulatedActiveW = 520;
  hltdc.Init.AccumulatedActiveH = 280;
  hltdc.Init.TotalWidth = 525;
  hltdc.Init.TotalHeigh = 288;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 480;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 272;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 480;
  pLayerCfg.ImageHeight = 272;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
  static void SystemIsolation_Config(void)
{

  /* USER CODE BEGIN RIF_Init 0 */

  /* USER CODE END RIF_Init 0 */

  /* set all required IPs as secure privileged */
  __HAL_RCC_RIFSC_CLK_ENABLE();

  /*RIMC configuration*/
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DMA2D, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_SDMMC1, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_SDMMC2, &RIMC_master);

  /*RISUP configuration*/
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_SDMMC1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_SDMMC2 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_ADC12 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_NPRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DMA2D , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);

  /* RIF-Aware IPs Config */

  /* set up PWR configuration */
  HAL_PWR_ConfigAttributes(PWR_ITEM_0,PWR_SEC_NPRIV);

  /* set up GPIO configuration */
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_15,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);

  /* USER CODE BEGIN RIF_Init 1 */

  /* USER CODE END RIF_Init 1 */
  /* USER CODE BEGIN RIF_Init 2 */

  /* USER CODE END RIF_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv = 4;
  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief SDMMC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC2_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC2_Init 0 */

  /* USER CODE END SDMMC2_Init 0 */

  /* USER CODE BEGIN SDMMC2_Init 1 */

  /* USER CODE END SDMMC2_Init 1 */
  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd2.Init.ClockDiv = 4;
  if (HAL_SD_Init(&hsd2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC2_Init 2 */

  /* USER CODE END SDMMC2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = TIM4_INPUT_CLOCK_MHZ - 1U;  /* 400 MHz / 400 = 1 MHz */
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 20000U - 1U;                   /* 20 ms period = 50 Hz */
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = SERVO_MID_US;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief XSPI1 Initialization Function
  * @param None
  * @retval None
  */
void MX_XSPI1_Init(void)
{

  /* USER CODE BEGIN XSPI1_Init 0 */

  /* USER CODE END XSPI1_Init 0 */

  XSPIM_CfgTypeDef sXspiManagerCfg = {0};
  XSPI_HyperbusCfgTypeDef sHyperBusCfg = {0};

  /* USER CODE BEGIN XSPI1_Init 1 */

  /* USER CODE END XSPI1_Init 1 */
  /* XSPI1 parameter configuration*/
  hxspi1.Instance = XSPI1;
  hxspi1.Init.FifoThresholdByte = 4;
  hxspi1.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi1.Init.MemoryType = HAL_XSPI_MEMTYPE_HYPERBUS;
  hxspi1.Init.MemorySize = HAL_XSPI_SIZE_256MB;
  hxspi1.Init.ChipSelectHighTimeCycle = 2;
  hxspi1.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi1.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi1.Init.WrapSize = HAL_XSPI_WRAP_32_BYTES;
  hxspi1.Init.ClockPrescaler = 1 - 1;
  hxspi1.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi1.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
  hxspi1.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
  hxspi1.Init.MaxTran = 0;
  hxspi1.Init.Refresh = 0;
  hxspi1.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hxspi1) != HAL_OK)
  {
    Error_Handler();
  }
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort = HAL_XSPIM_IOPORT_1;
  sXspiManagerCfg.Req2AckTime = 1;
  if (HAL_XSPIM_Config(&hxspi1, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  sHyperBusCfg.RWRecoveryTimeCycle = 7;
  sHyperBusCfg.AccessTimeCycle = 7;
  sHyperBusCfg.WriteZeroLatency = HAL_XSPI_LATENCY_ON_WRITE;
  sHyperBusCfg.LatencyMode = HAL_XSPI_FIXED_LATENCY;
  if (HAL_XSPI_HyperbusCfg(&hxspi1, &sHyperBusCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN XSPI1_Init 2 */

  /* USER CODE END XSPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOQ_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FAN_GPIO_PORT, FAN_GPIO_PIN, FAN_INACTIVE_LEVEL);

  /* PQ2 pump default OFF. Requirement: high level starts pump, so low means OFF. */
  HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN, PUMP_INACTIVE_LEVEL);

  /* DS18B20 DQ idle state is released high. */
  HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_GPIO_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin : PD1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PH6 fan control */
  GPIO_InitStruct.Pin = FAN_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FAN_GPIO_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : PQ2 water pump control */
  GPIO_InitStruct.Pin = PUMP_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PUMP_GPIO_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : PQ6 DS18B20_DQ */
  GPIO_InitStruct.Pin = DS18B20_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DS18B20_GPIO_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PE10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PG6 PG4 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PG11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief   HAL库DCMIPP pipe帧事件回调函数
 * @param   pHdcmipp: DCMIPP句柄指针
 * @param   Pipe: DCMIPP pipe号
 * @retval  无
 */
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp, uint32_t Pipe)
{
  imx335_dcmipp_pipe_frame_cb(hdcmipp, Pipe);
}

/**
 * @brief   HAL库DCMIPP pipe Vsync事件回调函数
 * @param   pHdcmipp: DCMIPP句柄指针
 * @param   Pipe: DCMIPP pipe号
 * @retval  无
 */
void HAL_DCMIPP_PIPE_VsyncEventCallback(DCMIPP_HandleTypeDef *hdcmipp, uint32_t Pipe)
{
  imx335_dcmipp_pipe_vsync_cb(hdcmipp, Pipe);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
