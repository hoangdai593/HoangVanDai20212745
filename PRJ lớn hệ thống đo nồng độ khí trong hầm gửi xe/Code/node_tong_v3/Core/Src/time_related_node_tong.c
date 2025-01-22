/*
 * time_related.c
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */
#include <time_related_node_tong.h>

void Change_Time_Read_Data()
{
	if(ppm_data_1.CO2_ppm <=1000 && ppm_data_1.CO_ppm <=25 && ppm_data_2.CO2_ppm <=1000 && ppm_data_2.CO_ppm <=25)
	{
    	HAL_TIM_Base_Stop_IT(&htim2);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim2, 3000);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim2);                 // Khởi động lại timer
	}
	else if(ppm_data_1.CO2_ppm >=2000 || ppm_data_1.CO_ppm >=50 || ppm_data_2.CO2_ppm >=2000 || ppm_data_2.CO_ppm >=50)
	{
    	HAL_TIM_Base_Stop_IT(&htim2);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim2, 500);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim2);                 // Khởi động lại timer
	}
	else
	{
    	HAL_TIM_Base_Stop_IT(&htim2);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim2, 1500);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim2);                 // Khởi động lại timer
	}
}
void Change_Time_Save_Data()
{
	if(ppm_data_1.CO2_ppm <=1000 && ppm_data_1.CO_ppm <=25 && ppm_data_2.CO2_ppm <=1000 && ppm_data_2.CO_ppm <=25)
	{
    	HAL_TIM_Base_Stop_IT(&htim3);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim3, T3);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim3);                 // Khởi động lại timer
	}
	else if(ppm_data_1.CO2_ppm >=2000 || ppm_data_1.CO_ppm >=50 || ppm_data_2.CO2_ppm >=2000 || ppm_data_2.CO_ppm >=50)
	{
    	HAL_TIM_Base_Stop_IT(&htim3);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim3, T2);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim3);                 // Khởi động lại timer
	}
	else
	{
    	HAL_TIM_Base_Stop_IT(&htim3);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim3, T1);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim3);                 // Khởi động lại timer
	}
}

void Control_Fan_Auto()
{
	if(ppm_data_1.CO2_ppm <=1000 && ppm_data_1.CO_ppm <=25 && ppm_data_2.CO2_ppm <=1000 && ppm_data_2.CO_ppm <=25)
	{
		TIM1->CCR1 = 50;
		L1_LED_On();
	}
	else if(ppm_data_1.CO2_ppm >=2000 || ppm_data_1.CO_ppm >=50 || ppm_data_2.CO2_ppm >=2000 || ppm_data_2.CO_ppm >=50)
	{
		TIM1->CCR1 = 100;
		L3_LED_On();
	}
	else
	{
		TIM1->CCR1 = 75;
		L2_LED_On();
	}
}

