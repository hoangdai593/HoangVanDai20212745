/*
 * led.h
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */

#ifndef INC_LED_H_
#define INC_LED_H_
#include "stm32f1xx_hal.h"  // Include STM32 HAL library for GPIO definitions
#include "main.h"
#define L1_LED_GPIO_Port GPIOB
#define L1_LED_Pin GPIO_PIN_12

#define L2_LED_GPIO_Port GPIOB
#define L2_LED_Pin GPIO_PIN_13

#define L3_LED_GPIO_Port GPIOB
#define L3_LED_Pin GPIO_PIN_14

#define AT_LED_Pin GPIO_PIN_15
#define AT_LED_GPIO_Port GPIOB

// Prototype các hàm điều khiển LED
void L2_LED_On(void);
void L3_LED_On(void);
void L1_LED_On(void);
void L1_3_LED_Off(void);
void AT_LED_On(void);
void AT_LED_Off(void);

#endif /* INC_LED_H_ */
