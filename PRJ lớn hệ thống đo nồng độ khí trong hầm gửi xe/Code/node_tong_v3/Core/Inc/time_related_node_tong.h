/*
 * time_related.h
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */

#ifndef INC_TIME_RELATED_NODE_TONG_H_
#define INC_TIME_RELATED_NODE_TONG_H_

#include "main.h"
#include "led.h"
// Khai báo các biến toàn cục nếu cần thiết, ví dụ như ppm_data_1, ppm_data_2, v.v.
extern PPM ppm_data_1;
extern PPM ppm_data_2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
#define T3 6000
#define T2 3000
#define T1 1000
// Khai báo các hàm sử dụng trong time_related.c
void Change_Time_Read_Data(void);
void Change_Time_Save_Data(void);
void Control_Fan_Auto(void);



#endif /* INC_TIME_RELATED_NODE_TONG_H_ */
