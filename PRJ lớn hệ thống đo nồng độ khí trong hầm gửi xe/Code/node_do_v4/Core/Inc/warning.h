/*
 * warning.h
 *
 *  Created on: Nov 13, 2024
 *      Author: DELL
 */

#ifndef INC_WARNING_H_
#define INC_WARNING_H_

#include "stm32f1xx_hal.h"  // Include STM32 HAL library for GPIO definitions
#include "main.h"

#define RL1_GPIO_Port GPIOA
#define RL1_Pin GPIO_PIN_7

#define RL2_GPIO_Port GPIOA
#define RL2_Pin GPIO_PIN_6

#define RL3_GPIO_Port GPIOA
#define RL3_Pin GPIO_PIN_5

#define L1_LED_GPIO_Port GPIOB
#define L1_LED_Pin GPIO_PIN_12

#define L2_LED_GPIO_Port GPIOB
#define L2_LED_Pin GPIO_PIN_13

#define L3_LED_GPIO_Port GPIOB
#define L3_LED_Pin GPIO_PIN_14

#define BUZ_GPIO_Port GPIOA
#define BUZ_Pin GPIO_PIN_1

extern uint16_t CO_ppm;
extern uint16_t CO2_ppm;
extern uint8_t l1_state;
extern uint8_t l2_state;
extern uint8_t l3_state;

void L2_Warning_On();
void L2_Warning_Off();
void L3_Warning_On();
void L3_Warning_Off();
void L1_Warning_On();
void L1_Warning_Off();
void Auto_Warning ();
void Manual_Warning ();
#endif /* INC_WARNING_H_ */
