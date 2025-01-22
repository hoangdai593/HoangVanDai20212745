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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CCS_RST_Pin GPIO_PIN_15
#define CCS_RST_GPIO_Port GPIOC
#define BUZ_Pin GPIO_PIN_1
#define BUZ_GPIO_Port GPIOA
#define L1_BUT_Pin GPIO_PIN_4
#define L1_BUT_GPIO_Port GPIOA
#define L1_BUT_EXTI_IRQn EXTI4_IRQn
#define RL3_Pin GPIO_PIN_5
#define RL3_GPIO_Port GPIOA
#define RL2_Pin GPIO_PIN_6
#define RL2_GPIO_Port GPIOA
#define RL1_Pin GPIO_PIN_7
#define RL1_GPIO_Port GPIOA
#define L3_BUT_Pin GPIO_PIN_0
#define L3_BUT_GPIO_Port GPIOB
#define L3_BUT_EXTI_IRQn EXTI0_IRQn
#define L2_BUT_Pin GPIO_PIN_1
#define L2_BUT_GPIO_Port GPIOB
#define L2_BUT_EXTI_IRQn EXTI1_IRQn
#define L1_LED_Pin GPIO_PIN_12
#define L1_LED_GPIO_Port GPIOB
#define L2_LED_Pin GPIO_PIN_13
#define L2_LED_GPIO_Port GPIOB
#define L3_LED_Pin GPIO_PIN_14
#define L3_LED_GPIO_Port GPIOB
#define CCS_RSTA15_Pin GPIO_PIN_15
#define CCS_RSTA15_GPIO_Port GPIOA
#define CCS811_INT_Pin GPIO_PIN_3
#define CCS811_INT_GPIO_Port GPIOB
#define CCS811_INT_EXTI_IRQn EXTI3_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
