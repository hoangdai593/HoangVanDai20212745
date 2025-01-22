/*
 * warning.c
 *
 *  Created on: Nov 13, 2024
 *      Author: DELL
 */

#include "warning.h"


void L2_Warning_On ()
{
	HAL_GPIO_WritePin(RL1_GPIO_Port,RL1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RL2_GPIO_Port,RL2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RL3_GPIO_Port,RL3_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BUZ_GPIO_Port,BUZ_Pin, GPIO_PIN_RESET);
}
void L2_Warning_Off()
{
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_SET);
}
void L3_Warning_On ()
{
	HAL_GPIO_WritePin(RL1_GPIO_Port,RL1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RL2_GPIO_Port,RL2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RL3_GPIO_Port,RL3_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BUZ_GPIO_Port,BUZ_Pin, GPIO_PIN_SET);
}
void L3_Warning_Off()
{
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BUZ_GPIO_Port,BUZ_Pin, GPIO_PIN_RESET);
}
void L1_Warning_On()
{
	HAL_GPIO_WritePin(RL1_GPIO_Port,RL1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RL2_GPIO_Port,RL2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RL3_GPIO_Port,RL3_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L2_LED_GPIO_Port,L2_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(L3_LED_GPIO_Port,L3_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BUZ_GPIO_Port,BUZ_Pin, GPIO_PIN_RESET);
}
void L1_Warning_Off()
{
	HAL_GPIO_WritePin(L1_LED_GPIO_Port,L1_LED_Pin, GPIO_PIN_SET);
}
void Auto_Warning ()
{
	  // khi không ấn nút( cảnh báo tự đông)
	  if(CO2_ppm<=1000 && CO_ppm <= 25)
	  {
		  L1_Warning_On();
	  }
	  else if((CO2_ppm>=2000) || (CO_ppm>=50))
	  {
		  L3_Warning_On();
	  }
	  else
	  {
		  L2_Warning_On();
	  }
}
void Manual_Warning ()
{
    if(l1_state==1)
    {
      L1_Warning_On();
    }
    else if (l2_state==1)
    {
      L2_Warning_On();
    }
    else if(l3_state==1)
    {
      L3_Warning_On();
    }
}
