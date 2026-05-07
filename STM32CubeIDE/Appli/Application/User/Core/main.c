/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : TIM4_CH4 PF0 SG90 servo only test
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;
UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/
static void SystemIsolation_Config(void);
static void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void servo_set_pulse_us(uint16_t pulse_us);
void Error_Handler(void);

/* Servo PWM setting
 * SG90 standard control:
 *   PWM frequency: 50 Hz
 *   Period: 20 ms
 *   Pulse width: about 500 us ~ 2500 us
 *
 * According to your current CubeMX clock tree:
 *   CPU clock  = 600 MHz
 *   TIM clock  = 400 MHz
 *
 * Prescaler = 400 - 1 gives 1 MHz timer counter clock.
 * So 1 counter tick = 1 us.
 */
#define TIM4_INPUT_CLOCK_MHZ          400U
#define SERVO_MIN_US                  500U
#define SERVO_MID_US                  1500U
#define SERVO_MAX_US                  2500U
#define SERVO_LEFT_US                 700U
#define SERVO_RIGHT_US                2300U
#define SERVO_DELAY_MS                1500U

int main(void)
{
  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();

  /* STM32N6 RIF / GPIO attribute config. Keep this before peripheral init. */
  SystemIsolation_Config();

  /* Configure TIM kernel clock source. */
  PeriphCommonClock_Config();

  MX_GPIO_Init();
  MX_TIM4_Init();

  if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  /* Move to middle first. */
  servo_set_pulse_us(SERVO_MID_US);
  HAL_Delay(1000);

  while (1)
  {
    /* Turn to one side. */
    servo_set_pulse_us(SERVO_LEFT_US);
    HAL_Delay(SERVO_DELAY_MS);

    /* Back to middle. */
    servo_set_pulse_us(SERVO_MID_US);
    HAL_Delay(SERVO_DELAY_MS);

    /* Turn to the other side. */
    servo_set_pulse_us(SERVO_RIGHT_US);
    HAL_Delay(SERVO_DELAY_MS);

    /* Back to middle. */
    servo_set_pulse_us(SERVO_MID_US);
    HAL_Delay(SERVO_DELAY_MS);
  }
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

  /* Because TIM4 counter clock is configured as 1 MHz, CCR value = us. */
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, pulse_us);
}

static void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM | RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPCLKSOURCE_HSI;
  PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_DIV1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

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

  /* This function must exist in stm32n6xx_hal_msp.c.
   * It should configure PF0 as TIM4_CH4, AF2_TIM4, GPIO_MODE_AF_PP.
   */
  HAL_TIM_MspPostInit(&htim4);
}

static void MX_GPIO_Init(void)
{
  /* GPIOF clock is needed because PF0 is TIM4_CH4 output. */
  __HAL_RCC_GPIOF_CLK_ENABLE();
}

static void SystemIsolation_Config(void)
{
  __HAL_RCC_RIFSC_CLK_ENABLE();

  /* PF0 is used as TIM4_CH4 PWM output for servo signal. */
  HAL_GPIO_ConfigPinAttributes(GPIOF, GPIO_PIN_0, GPIO_PIN_SEC | GPIO_PIN_NPRIV);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif
