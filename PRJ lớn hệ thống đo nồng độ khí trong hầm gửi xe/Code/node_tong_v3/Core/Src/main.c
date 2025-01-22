/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <time_related_node_tong.h>
#include"led.h"
#include <rs_related_node_tong.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t at_state=1;
uint16_t duty = 50;
// dữ liệu với node đo
uint8_t RS1_txBuffer[20];
uint8_t RS1_rxBuffer[30];
// dữ liệu với gateway
uint8_t RS2_txBuffer[40];
uint8_t RS2_rxBuffer[20];
uint8_t RS2_rx[20];
uint8_t rs1_temp[2];
uint8_t rs1_inx=0;
uint8_t rs2_temp[2];
uint8_t rs2_inx=0;
uint8_t node_flag=1 ;
uint8_t myID = 0x11;  // �?ịa chỉ Slave (duy nhất)

PPM ppm_data_1;
PPM ppm_data_2;
uint8_t lrc;
uint8_t received_lrc;
uint8_t receive_node1_flag=1; // c�? nhận được dữ liệu từ node 1 không
uint8_t receive_node2_flag=1; // c�? nhận được dữ liệu từ node 2 không
uint8_t connect_gw_error_flag=0; // c�? lỗi kết nối với gw
uint8_t connect_gw_miss_flag=0;
uint8_t save_ok = 0;			// lưu dữ liệu vào vùng nhớ tran xong hay không
uint8_t miss_data_node1_to_nodet_inx=0; 		// biến đếm dữ liệu thiếu node 1 đến node tổng
uint8_t miss_data_node2_to_nodet_inx=0;	// biến đếm dữ liệu thiếu node 2 dến node tổng
uint8_t miss_data_nodet_to_gw_inx=0;      // biến đếm dữ liệu thiếu gw
uint8_t miss_data_node1_to_gw_inx=0;
uint8_t miss_data_node2_to_gw_inx=0;

uint8_t send_gw_inx=0;

uint8_t send_n1_inx=0;

uint8_t send_n2_inx=0;
char string_data_tran[100][40];
char string_data_rec1[100][20];
char string_data_rec2[100][20];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// gửi lệnh đ�?c v�? node đo theo th�?i gian
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)  // Kiểm tra nếu ngắt từ Timer 2
    {
    	if(node_flag==1 && receive_node1_flag==1)
    	{
    	RS485_SendCommand(0x01, 0x02);  // Gửi lệnh "02" tới Slave 1 ( lệnh gửi 1 dữ liệu)
    	node_flag=2;
    	receive_node1_flag=0;
    	}
    	else if(node_flag==1 && receive_node1_flag==0)
    	{
    	RS485_SendCommand(0x01, 0x03);  // Gửi lệnh "03" tới Slave 1 ( lệnh gửi tất cả dữ liệu chưa gửi được)
    	node_flag=2;
    	receive_node1_flag=0;
    	}

    	else if(node_flag==2 && receive_node2_flag==1)
    	{
		RS485_SendCommand(0x02, 0x02);  // Gửi lệnh "02" tới Slave 2
		node_flag=1;
		receive_node2_flag=0;
    	}
    	else if(node_flag==2 && receive_node2_flag==0)
    	{
    		RS485_SendCommand(0x02, 0x03);  // Gửi lệnh "03" tới Slave 2
    		node_flag=1;
    		receive_node2_flag=0;
    	}
    }
    else if (htim->Instance == TIM3)  // Kiểm tra nếu ngắt từ Timer 3
    {
    	Change_Time_Save_Data();
    	connect_gw_error_flag++;
    	save_ok=0;
		 if(connect_gw_error_flag>=2 && save_ok==0)
		 {
			miss_data_nodet_to_gw_inx=connect_gw_error_flag-2;
			encode_ppm_data(ppm_data_1,(uint8_t*)string_data_tran[miss_data_nodet_to_gw_inx] , 5);
			encode_ppm_data(ppm_data_2,(uint8_t*)string_data_tran[miss_data_nodet_to_gw_inx] , 19);
			save_ok=1;
		 }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Nhận dữ liệu từ node đo
    if (huart->Instance == USART1)
    {
			// nhận chuỗi từ : đến \n
	if(rs1_temp[0] != '\n')
	{
		if(rs1_temp[0]==':')
		{
			RS1_rxBuffer[0]=rs1_temp[0];
			rs1_inx=1;
		}
		else
		{
			RS1_rxBuffer[rs1_inx++]=rs1_temp[0];
		}
	}
	else
	{
		RS1_rxBuffer[rs1_inx++]='\n';
    	if(check_lrc(&RS1_rxBuffer[1],18,(char*)&RS1_rxBuffer[19]) == 1)
    	{
			if (RS1_rxBuffer[1] == '0' && RS1_rxBuffer[2] == '1')
					{ // Kiểm tra địa chỉ
				if (RS1_rxBuffer[3] == '0' && RS1_rxBuffer[4] == '2')
				{
					HAL_TIM_Base_Start_IT(&htim2);
					decode_ppm_ver3((char*)RS1_rxBuffer,&ppm_data_1);
					receive_node1_flag=1; // c�? nhận được dữ liệu từ node1
					miss_data_node1_to_nodet_inx=0;
					Change_Time_Read_Data();
					RS485_SendCommand(0x01, 0xAA); //AA = OK
				}
				 else if(RS1_rxBuffer[3] == '0' && RS1_rxBuffer[4] == '3') // nhận được nhi�?u dữ liệu
				{
					HAL_TIM_Base_Stop_IT(&htim2);
					strcpy(&string_data_rec1[miss_data_node1_to_nodet_inx][5],(char*)&RS1_rxBuffer[5]);
					miss_data_node1_to_nodet_inx++;
					miss_data_node1_to_gw_inx = miss_data_node1_to_nodet_inx;
					RS485_SendCommand(0x01, 0xBB); // gui ve node do la da luu thanh cong
				}
			   }
			// node 2
			else if (RS1_rxBuffer[1] == '0' && RS1_rxBuffer[2] == '2')
					{ // Kiểm tra địa chỉ
				if (RS1_rxBuffer[3] == '0' && RS1_rxBuffer[4] == '2')
				{
					HAL_TIM_Base_Start_IT(&htim2);
					decode_ppm_ver3((char*)RS1_rxBuffer,&ppm_data_2);
					receive_node2_flag=1;
					miss_data_node2_to_nodet_inx=0;
					Change_Time_Read_Data();
					RS485_SendCommand(0x02, 0xAA);
				}
				else if(RS1_rxBuffer[3] == '0' && RS1_rxBuffer[4] == '3')
				{
					HAL_TIM_Base_Stop_IT(&htim2);
					strcpy(&string_data_rec2[miss_data_node2_to_nodet_inx][5],(char*)&RS1_rxBuffer[5]);
					miss_data_node2_to_nodet_inx++;
					miss_data_node2_to_gw_inx = miss_data_node2_to_nodet_inx;
					RS485_SendCommand(0x02, 0xBB);
				}
			   }
		 }
 		rs1_inx=0;
 		memset(RS1_rxBuffer,0,sizeof(RS1_rxBuffer));
    }
        HAL_UART_Receive_IT(&huart1, rs1_temp, 1);
    }


       //Nhận lệnh từ gateway
        if (huart->Instance == USART2)
        {
		// nhận bản tin từ từ : đến \n
		if(rs2_temp[0] != '\n') // nhận các kí tự đến \n
		{
			if(rs2_temp[0]==':') // kí tự bắt đầu phải là :
			{
				RS2_rxBuffer[0]=rs2_temp[0];
				rs2_inx=1;
			}
			else
			{
				RS2_rxBuffer[rs2_inx++]=rs2_temp[0];  // nhận các kí tự tiếp theo
			}
		}
		else // nhận xong 1 bản tin rồi xử lý dữ liệu
		{
			RS2_rxBuffer[rs2_inx++]='\n'; // chèn kí tự \n vào cuối bản tin
        	if(check_lrc(&RS2_rxBuffer[1],4,(char*)&RS2_rxBuffer[5]) == 1)
        	 {
				if (RS2_rxBuffer[1] == '1' && RS2_rxBuffer[2] == '1')
				{ // Kiểm tra địa chỉ
					connect_gw_error_flag=0;
					if (RS2_rxBuffer[3] == '0' && RS2_rxBuffer[4] == '2')
					{
						RS2_txBuffer[0]=':';
						// Gửi phản hồi
						hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
						RS2_txBuffer[3] = '0';
						RS2_txBuffer[4] = '2';

						// G�?i hàm để mã hóa dữ liệu ppm_data_1 vào RS2_txBuffer bắt đầu từ vị trí 5
						encode_ppm_data(ppm_data_1, RS2_txBuffer, 5);

						// G�?i hàm để mã hóa dữ liệu ppm_data_2 vào RS2_txBuffer bắt đầu từ vị trí 19
						encode_ppm_data(ppm_data_2, RS2_txBuffer, 19);

						lrc = calculate_lrc(&RS2_txBuffer[1],32);
						hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
						RS2_txBuffer[35]=0x0D ;
						RS2_txBuffer[36]=0x0A ;
						HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
					}
					else if(RS2_rxBuffer[3] == 'A' && RS2_rxBuffer[4] == 'A') // nhận lại oke
					{
						HAL_TIM_Base_Start_IT(&htim3);
						connect_gw_error_flag=0;
						miss_data_nodet_to_gw_inx=0;
						miss_data_node1_to_gw_inx=0;
						miss_data_node2_to_gw_inx=0;
						send_n1_inx=0;
						send_n2_inx=0;
						memset(string_data_rec1, 0, sizeof(string_data_rec1));
						memset(string_data_rec2, 0, sizeof(string_data_rec2));
						memset(string_data_tran, 0, sizeof(string_data_tran));
					}
					else if(RS2_rxBuffer[3] == '0' && RS2_rxBuffer[4] == '3') // th mat ket noi len gw
					{
						 if( miss_data_nodet_to_gw_inx!=0)
						 {
							 while(send_gw_inx<miss_data_nodet_to_gw_inx)
							 {
								RS2_txBuffer[0]=':';
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '0';
								RS2_txBuffer[4] = '3';
								strcpy((char*)&RS2_txBuffer[5], &string_data_tran[send_gw_inx][5]);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
								 HAL_UART_Receive(&huart2 ,RS2_rx, 10, HAL_MAX_DELAY);
								 if( RS2_rx[4] == 'B')
								 {
									 HAL_TIM_Base_Stop_IT(&htim3);
									 send_gw_inx++;
								 }
							 }
							 if(send_gw_inx==miss_data_nodet_to_gw_inx)
							 {
								RS2_txBuffer[0]=':';
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '0';
								RS2_txBuffer[4] = '2';
								strcpy((char*)&RS2_txBuffer[5], &string_data_tran[send_gw_inx][5]);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
							 }
						 }
						 else // them vao neu k chay thi xoa
						 {
							RS2_txBuffer[0]=':';
							// Gửi phản hồi
							hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
							RS2_txBuffer[3] = '0';
							RS2_txBuffer[4] = '0';
							lrc = calculate_lrc(&RS2_txBuffer[1],32);
							hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
							RS2_txBuffer[35]=0x0D ;
							RS2_txBuffer[36]=0x0A ;
							HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
						 }
					}
				   else if(RS2_rxBuffer[3] == '2' && RS2_rxBuffer[4] == '1') // nhận lệnh gửi dữ liệu bị miss từ node1 lên node tổng
				   {
					   if(miss_data_node1_to_gw_inx==0) // chưa có dữ liệu từ node 1 lên node tổng
					   {
							RS2_txBuffer[0]=':';
							// Gửi phản hồi
							hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
							RS2_txBuffer[3] = '0';
							RS2_txBuffer[4] = '2';

							// G�?i hàm để mã hóa dữ liệu ppm_data_1 vào RS2_txBuffer bắt đầu từ vị trí 5
							encode_ppm_data(ppm_data_1, RS2_txBuffer, 5);

							// G�?i hàm để mã hóa dữ liệu ppm_data_2 vào RS2_txBuffer bắt đầu từ vị trí 19
							encode_ppm_data(ppm_data_2, RS2_txBuffer, 19);

							lrc = calculate_lrc(&RS2_txBuffer[1],32);
							hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
							RS2_txBuffer[35]=0x0D ;
							RS2_txBuffer[36]=0x0A ;
							HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
					   }
					   else // đã có dữ liệu từ node đo lên node tổng
					   {
							while(send_n1_inx<miss_data_node1_to_gw_inx -1)
							{
								RS2_txBuffer[0]=':';
								// Gửi phản hồi
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '2';
								RS2_txBuffer[4] = '1';
								strncpy((char *)&RS2_txBuffer[5], &string_data_rec1[send_n1_inx][5], 14);
								memset(&RS2_txBuffer[19], 0, 14);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
								 HAL_UART_Receive(&huart2 ,RS2_rx, 10, HAL_MAX_DELAY);
								 if(RS2_rx[4] == 'C')
								 {
									 send_n1_inx++;
								 }
							}
							 if(send_n1_inx == miss_data_node1_to_gw_inx -1) // gửi dữ liệu cuối cùng của vugnf nhớ (có lệnh 0x02) để gw quay v�? thực hiện tiếp như chưa có lỗi
							 {
								RS2_txBuffer[0]=':';
								// Gửi phản hồi
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '0';
								RS2_txBuffer[4] = '2';
								strncpy((char *)&RS2_txBuffer[5], &string_data_rec1[send_n1_inx][5], 14);
								memset(&RS2_txBuffer[19], 0, 14);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
								 send_n1_inx++;
							 }
						 }
				   }
				   else if(RS2_rxBuffer[3] == '2' && RS2_rxBuffer[4] == '2')
				   {
					   if(miss_data_node2_to_gw_inx==0) // chưa có dữ liệu từ node 2 lên node tổng
					   {
							RS2_txBuffer[0]=':';
							// Gửi phản hồi
							hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
							RS2_txBuffer[3] = '0';
							RS2_txBuffer[4] = '2';

							// G�?i hàm để mã hóa dữ liệu ppm_data_1 vào RS2_txBuffer bắt đầu từ vị trí 5
							encode_ppm_data(ppm_data_1, RS2_txBuffer, 5);

							// G�?i hàm để mã hóa dữ liệu ppm_data_2 vào RS2_txBuffer bắt đầu từ vị trí 19
							encode_ppm_data(ppm_data_2, RS2_txBuffer, 19);

							lrc = calculate_lrc(&RS2_txBuffer[1],32);
							hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
							RS2_txBuffer[35]=0x0D ;
							RS2_txBuffer[36]=0x0A ;
							HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
					   }
					   else // đã có dữ liệu từ node đo lên node tổng
					   {
							 while(send_n2_inx<miss_data_node2_to_gw_inx -1)
							{
								RS2_txBuffer[0]=':';
								// Gửi phản hồi
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '2';
								RS2_txBuffer[4] = '2';
								memset(&RS2_txBuffer[5], 0, 14);
								strncpy((char *)&RS2_txBuffer[19], &string_data_rec2[send_n2_inx][5], 14);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
								 HAL_UART_Receive(&huart2 ,RS2_rx, 10, HAL_MAX_DELAY);
								 if( RS2_rx[4] == 'D')
								 {
									 send_n2_inx++;
								 }
							}
							 if(send_n2_inx == miss_data_node2_to_gw_inx -1)// gửi dữ liệu cuối cùng của vugnf nhớ (có lệnh 0x02) để gw quay v�? thực hiện tiếp như chưa có lỗi
							 {
								RS2_txBuffer[0]=':';
								// Gửi phản hồi
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '0';
								RS2_txBuffer[4] = '2';
								memset(&RS2_txBuffer[5], 0, 14);
								strncpy((char *)&RS2_txBuffer[19], &string_data_rec2[send_n2_inx][5], 14);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
							 }
					   }
				   }
				   else if(RS2_rxBuffer[3] == '2' && RS2_rxBuffer[4] == '3')
				   {
					   if(miss_data_node2_to_gw_inx==0) // chưa có dữ liệu từ 2 node lên node tổng
					   {
							RS2_txBuffer[0]=':';
							// Gửi phản hồi
							hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
							RS2_txBuffer[3] = '0';
							RS2_txBuffer[4] = '0';
							lrc = calculate_lrc(&RS2_txBuffer[1],32);
							hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
							RS2_txBuffer[35]=0x0D ;
							RS2_txBuffer[36]=0x0A ;
							 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
					   }
					   else // đã có dữ liệu từ node đo lên node tổng
					   {
							 while(send_n2_inx<miss_data_node2_to_gw_inx -1)
							{
								RS2_txBuffer[0]=':';
								// Gửi phản hồi
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '2';
								RS2_txBuffer[4] = '3';
								strncpy((char *)&RS2_txBuffer[5], &string_data_rec1[send_n2_inx][5], 14);
								strncpy((char *)&RS2_txBuffer[19], &string_data_rec2[send_n2_inx][5], 14);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
								 HAL_UART_Receive(&huart2 ,RS2_rx, 10, HAL_MAX_DELAY);
								 if( RS2_rx[4] == 'D')
								 {
									 send_n2_inx++;
								 }
							}
							 if(send_n2_inx == miss_data_node2_to_gw_inx -1)// gửi dữ liệu cuối cùng của vugnf nhớ (có lệnh 0x02) để gw quay v�? thực hiện tiếp như chưa có lỗi
							 {
								RS2_txBuffer[0]=':';
								// Gửi phản hồi
								hexdec_to_ascii(myID,(char*)&RS2_txBuffer[1]);
								RS2_txBuffer[3] = '0';
								RS2_txBuffer[4] = '2';
								strncpy((char *)&RS2_txBuffer[5], &string_data_rec1[send_n2_inx][5], 14);
								strncpy((char *)&RS2_txBuffer[19], &string_data_rec2[send_n2_inx][5], 14);
								lrc = calculate_lrc(&RS2_txBuffer[1],32);
								hexdec_to_ascii(lrc,(char*)&RS2_txBuffer[33]);
								RS2_txBuffer[35]=0x0D ;
								RS2_txBuffer[36]=0x0A ;
								 HAL_UART_Transmit(&huart2, RS2_txBuffer, 40, HAL_MAX_DELAY);
							 }
					   }
				   }
				}
        	 }
        	// nhận bản tin th�?i gian từ gw và gửi v�? các node đo
            if(RS2_rxBuffer[2] == 0x77)
			{
				 HAL_UART_Transmit(&huart1, RS2_rxBuffer, 11, HAL_MAX_DELAY); // truyen ve node do thoi gian
			}
            // reset rs2 và RS2_rxbuf để tiếp tục nhận bản tin khác
      		rs2_inx=0;
      		memset(RS2_rxBuffer,0,sizeof(RS2_rxBuffer));
       }
               HAL_UART_Receive_IT(&huart2, rs2_temp, 1);
               // nhận mỗi lần 1 kí tự
        }
}
// chương trình đk quạt = tay
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  UNUSED(GPIO_Pin);
  if(GPIO_Pin == DOWN_BUT_Pin)
  {
	  at_state=0; // c�? trạng thái đi�?u khiển quạt tự động =0
	  AT_LED_On(); // bật đèn báo hiệu đk quạt = tay
	  if(duty<=50)
	  {
		duty = 0;
	  }
	  else
	  {
		duty = duty -25;
	  }
	  TIM1->CCR1 = duty;

  }
  if(GPIO_Pin == UP_BUT_Pin)
  {
	  at_state=0;
	  AT_LED_On();
	  if(duty==100)
	  {
		duty =100;
	  }
	  else
	  {
		  if(duty==0)
		  {
			  duty=50;
		  }
		  else
		  {
		    duty = duty +25;
		  }
	  }
	  TIM1->CCR1 = duty;
  }
  if(at_state==0)
  {
	  // hiển thị led
	  if(duty<50)
	  {
		  L1_3_LED_Off();
	  }
	  else if (duty==50)
	  {
		  L1_LED_On();
	  }
	  else if (duty==75)
	  {
		  L2_LED_On();
	  }
	  else if(duty==100)
	  {
		  L3_LED_On();
	  }
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  TIM1->CCR1 = duty;
   HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  HAL_UART_Receive_IT(&huart1, rs1_temp, 1); // nhận dữ liệu từ node đo
  HAL_UART_Receive_IT(&huart2, rs2_temp, 1); // nhận dữ liệu từ gateway

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
   if(at_state==1) //dk quạt tự động
   {
	   Control_Fan_Auto();
   }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 16000-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 16000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 3000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 16000-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 6000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, L1_LED_Pin|L2_LED_Pin|L3_LED_Pin|AT_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : DOWN_BUT_Pin UP_BUT_Pin */
  GPIO_InitStruct.Pin = DOWN_BUT_Pin|UP_BUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : L1_LED_Pin L2_LED_Pin L3_LED_Pin AT_LED_Pin */
  GPIO_InitStruct.Pin = L1_LED_Pin|L2_LED_Pin|L3_LED_Pin|AT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
