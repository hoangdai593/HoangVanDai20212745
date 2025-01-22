/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DOWN_BUT_Pin GPIO_PIN_0
#define DOWN_BUT_GPIO_Port GPIOB
#define DOWN_BUT_EXTI_IRQn EXTI0_IRQn
#define UP_BUT_Pin GPIO_PIN_1
#define UP_BUT_GPIO_Port GPIOB
#define UP_BUT_EXTI_IRQn EXTI1_IRQn
#define L1_LED_Pin GPIO_PIN_12
#define L1_LED_GPIO_Port GPIOB
#define L2_LED_Pin GPIO_PIN_13
#define L2_LED_GPIO_Port GPIOB
#define L3_LED_Pin GPIO_PIN_14
#define L3_LED_GPIO_Port GPIOB
#define AT_LED_Pin GPIO_PIN_15
#define AT_LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t CO_ppm;
	uint16_t CO2_ppm;
}PPM;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
