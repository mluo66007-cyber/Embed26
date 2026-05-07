/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32n6xx_hal_msp.c
  * @brief        HAL MSP file for TIM4_CH4 PWM on PF0 only
  ******************************************************************************
  * Use case:
  *   Servo-only test project.
  *
  * TIM4_CH4 PWM output:
  *   PF0  ------> TIM4_CH4
  *
  * Important:
  *   This MSP file is intentionally minimal. Do NOT use it to replace the MSP
  *   file in the full camera/sensor project, because it does not include
  *   DCMIPP, CSI, I2C, ADC, LTDC, SDMMC, UART, etc.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  /*
   * STM32N6 RIF clock is needed if pin attributes are configured here.
   * If your main.c already calls SystemIsolation_Config(), this is still safe.
   */
  __HAL_RCC_RIFSC_CLK_ENABLE();

  /*
   * Give PF0 secure/non-privileged access for the servo PWM pin.
   * This helps when using a minimal servo-only main.c on STM32N6.
   */
  HAL_GPIO_ConfigPinAttributes(GPIOF, GPIO_PIN_0, GPIO_PIN_SEC | GPIO_PIN_NPRIV);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief TIM_Base MSP Initialization
  *        This function configures the hardware resources used by TIM4.
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if (htim_base->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspInit 0 */

    /* USER CODE END TIM4_MspInit 0 */

    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* USER CODE BEGIN TIM4_MspInit 1 */

    /* USER CODE END TIM4_MspInit 1 */
  }
}

/**
  * @brief TIM_PWM MSP Initialization
  *        Kept here so TIM4 clock is also enabled if the project calls
  *        HAL_TIM_PWM_Init() directly.
  * @param htim_pwm: TIM_PWM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
  if (htim_pwm->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_PWM_MspInit 0 */

    /* USER CODE END TIM4_PWM_MspInit 0 */

    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* USER CODE BEGIN TIM4_PWM_MspInit 1 */

    /* USER CODE END TIM4_PWM_MspInit 1 */
  }
}

/**
  * @brief TIM MSP Post Initialization
  *        This function configures PF0 as TIM4_CH4 PWM output.
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (htim->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspPostInit 0 */

    /* USER CODE END TIM4_MspPostInit 0 */

    __HAL_RCC_GPIOF_CLK_ENABLE();

    /*
     * TIM4 GPIO Configuration
     * PF0  ------> TIM4_CH4
     */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM4_MspPostInit 1 */

    /* USER CODE END TIM4_MspPostInit 1 */
  }
}

/**
  * @brief TIM_Base MSP De-Initialization
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if (htim_base->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspDeInit 0 */

    /* USER CODE END TIM4_MspDeInit 0 */

    __HAL_RCC_TIM4_CLK_DISABLE();

    /* USER CODE BEGIN TIM4_MspDeInit 1 */

    /* USER CODE END TIM4_MspDeInit 1 */
  }
}

/**
  * @brief TIM_PWM MSP De-Initialization
  * @param htim_pwm: TIM_PWM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
  if (htim_pwm->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_PWM_MspDeInit 0 */

    /* USER CODE END TIM4_PWM_MspDeInit 0 */

    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0);

    /* USER CODE BEGIN TIM4_PWM_MspDeInit 1 */

    /* USER CODE END TIM4_PWM_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
