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
#include "math.h"
#include "i2c-lcd.h"
#include "warning.h"
#include <stdio.h>
#include "ccs811.h"
#include <stdlib.h>
#include <string.h>
#include "rs_related_node_do.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define T3 6000
#define T2 3000
#define T1 1000
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RS_rxBuffer[20];
uint8_t RS_rx[20];
uint8_t RS_txBuffer[30];
uint8_t rs_temp[2];
uint8_t rs_inx=0;

uint8_t ppm_data[20];
uint8_t myID = 0x01;  // �?ịa chỉ Slave (duy nhất)
//uint8_t myID = 0x02;  // �?ịa chỉ Slave (duy nhất)

uint8_t l1_state=0;
uint8_t l2_state=0;
uint8_t l3_state=0;
uint8_t at_state=1;
uint8_t co2_flag=0;
uint16_t eCO2 = 0;
uint16_t CO2_ppm;
uint32_t CO_adc_val;

float CO_vol;
uint16_t CO_ppm;
char CO_ppm_buf[6];
char CO2_ppm_buf[6];

uint8_t miss_inx=0;
uint8_t send_inx=0;

uint8_t connect_error_flag=0;
uint8_t connect_miss_flag=0;
uint8_t save_ok = 0;
char string_data[100][20];
char data_saved[100][30]; // Mảng con tr�? để lưu chuỗi
uint8_t lrc;

RTC_TimeTypeDef gTime;



void set_time (uint8_t hr, uint8_t min, uint8_t sec)
{
	RTC_TimeTypeDef sTime = {0};

	sTime.Hours = hr;
	sTime.Minutes = min;
	sTime.Seconds = sec;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x2345);  // backup register
}

void get_time()
{
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
}

int CO_ppm_convert(float CO_vol) {
    float a = ((5.0 - CO_vol) / CO_vol) * (10/5);
    return (int)round(100 * pow(a, -1.559));
}

void Change_Timer_Period(TIM_HandleTypeDef *htim)
{
if (CO2_ppm <= 1000 && CO_ppm <= 25)
   {
       HAL_TIM_Base_Stop_IT(htim);                  // Dừng timer
       __HAL_TIM_SET_AUTORELOAD(htim, T3);          // Cập nhật chu kỳ
       HAL_TIM_Base_Start_IT(htim);                 // Khởi động lại timer
   }
   else if (CO2_ppm >= 2000 || CO_ppm >= 50)
   {
       HAL_TIM_Base_Stop_IT(htim);                  // Dừng timer
       __HAL_TIM_SET_AUTORELOAD(htim, T1);          // Cập nhật chu kỳ
       HAL_TIM_Base_Start_IT(htim);                 // Khởi động lại timer
   }
   else
   {
       HAL_TIM_Base_Stop_IT(htim);                  // Dừng timer
       __HAL_TIM_SET_AUTORELOAD(htim, T2);          // Cập nhật chu kỳ
       HAL_TIM_Base_Start_IT(htim);                 // Khởi động lại timer
   }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)  // Kiểm tra nếu ngắt từ Timer 2
    {
    	HAL_ADC_Start_IT(&hadc1);
    }
    else if (htim->Instance == TIM3)  // Kiểm tra nếu ngắt từ Timer 3
    {
    	Change_Timer_Period(&htim3);
    	connect_error_flag++;
    	save_ok=0;
   	 if(connect_error_flag>=2 && save_ok==0)
   	 {
   		miss_inx=connect_error_flag-2;
   		get_time();
   		hexdec_to_ascii(gTime.Hours,&data_saved[miss_inx][5]);
		hexdec_to_ascii(gTime.Minutes,&data_saved[miss_inx][7]);
		hexdec_to_ascii(gTime.Seconds,&data_saved[miss_inx][9]);
		hexdec_to_ascii((uint8_t)(CO_ppm>>8),&data_saved[miss_inx][11]);
		hexdec_to_ascii((uint8_t)(CO_ppm),&data_saved[miss_inx][13]);
		hexdec_to_ascii((uint8_t)(CO2_ppm>>8),&data_saved[miss_inx][15]);
		hexdec_to_ascii((uint8_t)(CO2_ppm),&data_saved[miss_inx][17]);
   		save_ok=1;
   	 }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc -> Instance == ADC1)
    CO_adc_val = HAL_ADC_GetValue(&hadc1);
	CO_vol = ((float)CO_adc_val / 4095.0) * 5;
	CO_ppm = (uint16_t)(CO_ppm_convert(CO_vol));
	sprintf(CO_ppm_buf, "%d",(int) CO_ppm);
	// Xóa vị trí hiện tại trên LCD trước khi ghi chuỗi mới
	lcd_put_cur(1, 4);
	lcd_send_string("    ");  // Ghi đè 4f khoảng trắng tại vị trí đó

	lcd_put_cur(1, 4);
	lcd_send_string (CO_ppm_buf);
	if(CO_ppm > 9999)
	{
	   lcd_put_cur(1, 8);
	   lcd_send_string("        ");  // Ghi đè 8 khoảng trắng tại vị trí đó
	   lcd_put_cur(1, 8);
	   lcd_send_string ("ppm|AUTO");
	}
	// đo và hiển thị LCD cho CO2
	CCS811_Read_Co2(&eCO2);
    CO2_ppm = (eCO2);
	sprintf(CO2_ppm_buf, "%d",(int) CO2_ppm);
	// Xóa vị trí hiện tại trên LCD trước khi ghi chuỗi mới
	lcd_put_cur(0, 4);
	lcd_send_string("    ");  // Ghi đè 4 khoảng trắng tại vị trí đó
	// ghi vào vị trí này
	lcd_put_cur(0, 4);
	lcd_send_string (CO2_ppm_buf);

	if(CO2_ppm > 9999)
	{
	   lcd_put_cur(0, 8);
	   lcd_send_string("        ");
	   lcd_put_cur(0, 8);
	   lcd_send_string ("ppm|MODE");
	}
	if(at_state==1)
	{
		Auto_Warning();
	}
	Change_Timer_Period(&htim2);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	UNUSED(GPIO_Pin);
     if (GPIO_Pin == L1_BUT_Pin)
    {
        while(HAL_GPIO_ReadPin(L1_BUT_GPIO_Port,L1_BUT_Pin)==0)
           {};
     l1_state = 1;
     l2_state = 0;
     l3_state = 0;
     at_state = 0;
	lcd_put_cur(1, 12);
	lcd_send_string ("HAND");
    }
    else if (GPIO_Pin == L2_BUT_Pin)
    {
      while(HAL_GPIO_ReadPin(L2_BUT_GPIO_Port,L2_BUT_Pin)==0)
        {};
     l1_state = 0;
     l2_state = 1;
     l3_state = 0;
     at_state = 0;
	lcd_put_cur(1, 12);
	lcd_send_string ("HAND");
    }

    else if (GPIO_Pin == L3_BUT_Pin)
    {
     while(HAL_GPIO_ReadPin(L3_BUT_GPIO_Port,L3_BUT_Pin)==0)
        {};
     l1_state = 0;
     l2_state = 0;
     l3_state = 1;
     at_state = 0;
	lcd_put_cur(1, 12);
	lcd_send_string ("HAND");
    }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
    	// nhận chuỗi từ : đến \n
	if(rs_temp[0] != '\n')
	{
		if(rs_temp[0]==':')
		{
			RS_rxBuffer[0]=rs_temp[0];
			rs_inx=1;
		}
		else
		{
			RS_rxBuffer[rs_inx++]=rs_temp[0];
		}
	}
	else
	{
		RS_rxBuffer[rs_inx++]='\n';
    	if(check_lrc(&RS_rxBuffer[1],4,(char*)&RS_rxBuffer[5]) == 1) // check loi lrc
    	{
    		//if (RS_rxBuffer[1] == '0' && RS_rxBuffer[2] == '2') // Kiểm tra địa chỉ node 2
			if (RS_rxBuffer[1] == '0' && RS_rxBuffer[2] == '1') // Kiểm tra địa chỉ
			{
				if (RS_rxBuffer[3] == '0' && RS_rxBuffer[4] == '2')
				{
					get_time();
					// Gửi phản hồi
					RS485_data_to_send();
					HAL_UART_Transmit(&huart1, RS_txBuffer, 25, HAL_MAX_DELAY);
				}
				else if(RS_rxBuffer[3] == '0' && RS_rxBuffer[4] == '3')
				{
					 if( miss_inx!=0)
					 {
						 while(send_inx<miss_inx)
						 {
							RS_txBuffer[0] = ':';
							hexdec_to_ascii(myID,(char*)&RS_txBuffer[1]);
							RS_txBuffer[3] = '0';
							RS_txBuffer[4] = '3';
							strcpy((char*)&RS_txBuffer[5], &data_saved[send_inx][5]);
							lrc = calculate_lrc(&RS_txBuffer[1],18);
							hexdec_to_ascii(lrc,(char*)&RS_txBuffer[19]);
							RS_txBuffer[21]=0x0D ;
							RS_txBuffer[22]=0x0A ;
							 HAL_UART_Transmit(&huart1, RS_txBuffer, 25, HAL_MAX_DELAY);
							 HAL_UART_Receive(&huart1 ,RS_rx, 10, HAL_MAX_DELAY);
							 if( RS_rx[4] == 'B')
							 {
								 HAL_TIM_Base_Stop_IT(&htim3);
								 send_inx++;
							 }
						 }
						 if(send_inx==miss_inx)
						 {
							RS_txBuffer[0] = ':';
							hexdec_to_ascii(myID,(char*)&RS_txBuffer[1]);
							RS_txBuffer[3] = '0';
							RS_txBuffer[4] = '2';
							strcpy((char*)&RS_txBuffer[5], &data_saved[send_inx][5]);
							lrc = calculate_lrc(&RS_txBuffer[1],18);
							hexdec_to_ascii(lrc,(char*)&RS_txBuffer[19]);
							RS_txBuffer[21]=0x0D ;
							RS_txBuffer[22]=0x0A ;
							 HAL_UART_Transmit(&huart1, RS_txBuffer, 25, HAL_MAX_DELAY);
						 }
					 }
				}
				else if(RS_rxBuffer[3] == 'A' && RS_rxBuffer[4] == 'A') // nhận lại oke
				{
					memset(data_saved, 0, sizeof(data_saved));
					HAL_TIM_Base_Start_IT(&htim3);
					send_inx=0;
					connect_error_flag=0;
				}
			}
    	}
    	else
    	{}

        // nhận bản tin v�? th�?i gian
         if(RS_rxBuffer[2] == 0x77)
        {
        	int h,m,s;
        	char ho[3] = {0};
        	char mi[3] = {0};
        	char se[3] = {0};

        	strncpy(ho, (char*)&RS_rxBuffer[3], 2);
        	strncpy(mi, (char*)&RS_rxBuffer[5], 2);
        	strncpy(se, (char*)&RS_rxBuffer[7], 2);

        	h= atoi(ho);
        	m=atoi(mi);
        	s=atoi(se);
        	set_time((uint8_t)h,(uint8_t) m ,(uint8_t)s);
        }
 		rs_inx=0;
 		memset(RS_rxBuffer,0,sizeof(RS_rxBuffer));
 	  }
        // Tiếp tục nhận
        HAL_UART_Receive_IT(&huart1, rs_temp, 1);
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  //while(!CCS811_CheckHW_ID());
  CCS811_Init();
  CCS811_Reset();
  CCS811_Set_Mode(CCS811_MODE_1SEC);
  CCS811_EnableInt();
  //uint16_t baseline = CCS811_GetBaseline();
  lcd_init();
  lcd_put_cur(0, 0);
  lcd_send_string ("CO2:");
  lcd_put_cur(0, 8);
  lcd_send_string ("ppm|");
  lcd_put_cur(0, 12);
  lcd_send_string ("MODE");
  lcd_put_cur(1, 0);
  lcd_send_string("CO :");
  lcd_put_cur(1, 8);
  lcd_send_string ("ppm|");
  lcd_put_cur(1, 12);
  lcd_send_string ("AUTO");

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_UART_Receive_IT(&huart1, rs_temp, 1);

  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x2345)
  {
	  set_time(12, 50, 50);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		  // khi tắt chế độ tự động
	  if(at_state==0)
	 {
	  Manual_Warning();
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
//  sTime.Hours = 0x12;
//  sTime.Minutes = 0x50;
//  sTime.Seconds = 0x0;
//
//  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
//  DateToUpdate.Month = RTC_MONTH_JANUARY;
//  DateToUpdate.Date = 0x1;
//  DateToUpdate.Year = 0x0;
//
//  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
  htim2.Init.Period = 6000;
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
  htim3.Init.Period = 60000;
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CCS_RST_GPIO_Port, CCS_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BUZ_Pin|RL3_Pin|RL2_Pin|RL1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, L1_LED_Pin|L2_LED_Pin|L3_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CCS_RSTA15_GPIO_Port, CCS_RSTA15_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : CCS_RST_Pin */
  GPIO_InitStruct.Pin = CCS_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CCS_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BUZ_Pin RL3_Pin RL2_Pin RL1_Pin */
  GPIO_InitStruct.Pin = BUZ_Pin|RL3_Pin|RL2_Pin|RL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : L1_BUT_Pin */
  GPIO_InitStruct.Pin = L1_BUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(L1_BUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : L3_BUT_Pin L2_BUT_Pin */
  GPIO_InitStruct.Pin = L3_BUT_Pin|L2_BUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : L1_LED_Pin L2_LED_Pin L3_LED_Pin */
  GPIO_InitStruct.Pin = L1_LED_Pin|L2_LED_Pin|L3_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CCS_RSTA15_Pin */
  GPIO_InitStruct.Pin = CCS_RSTA15_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CCS_RSTA15_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CCS811_INT_Pin */
  GPIO_InitStruct.Pin = CCS811_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CCS811_INT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

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
