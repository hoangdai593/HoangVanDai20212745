/*
 * led.c
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */

#include"led.h"
void L2_LED_On ()
{
	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_SET);
}
void L3_LED_On ()
{
	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_RESET);
}
void L1_LED_On()
{
	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_SET);
}
void L1_3_LED_Off()
{
	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_SET);
}
void AT_LED_On()
{
	HAL_GPIO_WritePin(AT_LED_GPIO_Port,AT_LED_Pin, GPIO_PIN_RESET);
}
void AT_LED_Off()
{
	HAL_GPIO_WritePin(AT_LED_GPIO_Port,AT_LED_Pin, GPIO_PIN_SET);
}
