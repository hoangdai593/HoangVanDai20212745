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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t *USB_data ;
uint8_t USB_rxbuffer[10];
uint8_t USB_txbuffer[40];

uint8_t RS_txBuffer[10];
uint8_t RS_rxBuffer[40];

uint8_t rs_temp[2];
uint8_t rs_inx=0;

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t CO_ppm;
	uint16_t CO2_ppm;
}PPM;

PPM ppm_data_1;
PPM ppm_data_2;
int time_data_n1;
int time_data_n2;
int receive_node_tong_flag=1;
int node_tong_to_node_1_error=0;
int node_tong_to_node_2_error=0;
int ppm2_time_temp;
int ppm1_time_temp;
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
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t calculate_lrc(uint8_t *ascii_data, uint16_t length) {
    uint8_t lrc = 0;

    // Kiểm tra nếu chiều dài mảng là hợp lệ
    if (length < 2) {
        return 0;  // Trả về 0 nếu chiều dài không hợp lệ
    }

    // Mỗi byte nhị phân được biểu diễn bằng 2 ký tự ASCII
    for (uint16_t i = 0; i < length; i += 2) {
        // Chuyển đổi 2 ký tự ASCII thành 1 byte nhị phân
        char hex_byte[3] = {ascii_data[i], ascii_data[i + 1], '\0'};
        uint8_t byte_value = (uint8_t)strtol(hex_byte, NULL, 16);

        // Cộng giá trị vào tổng LRC
        lrc += byte_value;
    }

    // Lấy phần bù 2 (2's complement)
    lrc = (uint8_t)(-((int8_t)lrc));
    return lrc;
}

uint8_t check_lrc(uint8_t *ascii_data, uint16_t length, char* ascii_lrc)
{

    // Tính LRC từ dữ liệu
    uint8_t calculated_lrc = calculate_lrc(ascii_data, length);

    char hex_byte[3] = {ascii_lrc[0], ascii_lrc[1], '\0'};
    uint8_t received_lrc = (uint8_t)strtol(hex_byte, NULL, 16);

    // So sánh LRC tính toán và LRC nhận được
    if (calculated_lrc == received_lrc)
    {
    	return 1;
    }
    else
    	return 0;
}
// Mã hóa LRC thành chuỗi ASCII
void hexdec_to_ascii(uint8_t hexdec, char *ascii_out)
{
    sprintf(ascii_out, "%02X", hexdec); // Mã hóa thành 2 ký tự HEX ASCII
}
// Gửi lệnh tới Slave với địa chỉ ID
void RS485_SendCommand(uint8_t slaveID, uint8_t command)
{
	RS_txBuffer[0] = ':';
	hexdec_to_ascii(slaveID,(char*)&RS_txBuffer[1]);
	hexdec_to_ascii(command,(char*)&RS_txBuffer[3]);
	uint8_t lrc = calculate_lrc(&RS_txBuffer[1], 4);
	hexdec_to_ascii(lrc,(char*)&RS_txBuffer[5]);
	RS_txBuffer[7]=0x0D ;
	RS_txBuffer[8]=0x0A ;
    HAL_UART_Transmit(&huart1, RS_txBuffer, 10, HAL_MAX_DELAY);
}
void decode_ppm2_ver3 (char *RS_rxBuffer,PPM *ppm, uint8_t inx) // inx=5 (node1) inx=19(node2)
{
	char hour_buf[3] = {0}; // Lưu chuỗi th�?i gian
	char min_buf[3] = {0};
	char sec_buf[3] = {0};
	char co_high[3] = {0};   // Lưu chuỗi giá trị CO
	char co_low[3] = {0};   // Lưu chuỗi giá trị CO
	char co2_high[3] = {0};   // Lưu chuỗi giá trị CO
	char co2_low[3] = {0};  // Lưu chuỗi giá trị CO2
	uint16_t CO;
	uint16_t CO2;
	uint8_t ho;
	uint8_t mi;
	uint8_t se;
	// Tách chuỗi từ buffer
    hour_buf[0] = RS_rxBuffer[inx];
    hour_buf[1] = RS_rxBuffer[inx+1];
    hour_buf[2] = '\0';

    min_buf[0] = RS_rxBuffer[inx+2];
    min_buf[1] = RS_rxBuffer[inx+3];
    min_buf[2] = '\0';

    sec_buf[0] = RS_rxBuffer[inx+4];
    sec_buf[1] = RS_rxBuffer[inx+5];
    sec_buf[2] = '\0';

    co_high[0] = RS_rxBuffer[inx+6];
    co_high[1] = RS_rxBuffer[inx+7];
    co_high[2] = '\0';

    co_low[0] = RS_rxBuffer[inx+8];
    co_low[1] = RS_rxBuffer[inx+9];
    co_low[2] = '\0';

    co2_high[0] = RS_rxBuffer[inx+10];
    co2_high[1] = RS_rxBuffer[inx+11];
    co2_high[2] = '\0';

    co2_low[0] = RS_rxBuffer[inx+12];
    co2_low[1] = RS_rxBuffer[inx+13];
    co2_low[2] = '\0';

	ho = (uint8_t)strtol(hour_buf, NULL, 16);
	mi = (uint8_t)strtol(min_buf, NULL, 16);
	se = (uint8_t)strtol(sec_buf, NULL, 16);
	CO = ((uint8_t)(strtol(co_high, NULL, 16)) << 8) | ((uint8_t)(strtol(co_low, NULL, 16)));
	CO2 = ((uint8_t)(strtol(co2_high, NULL, 16)) << 8) | ((uint8_t)(strtol(co2_low, NULL, 16)));

	ppm->hour = ho;
    ppm->min = mi;
	ppm->sec = se;
	ppm->CO_ppm = CO;
    ppm->CO2_ppm = CO2;

}
void make_time(PPM *ppm, int *time_data)
{
	*time_data = (ppm->hour)*10000 + (ppm->min)*100 + (ppm->sec);
}
void USB_data_to_send()
{
	sprintf((char*)(&(USB_txbuffer[2])), "%02d%02d%02d%5d%5d%02d%02d%02d%5d%5d"
	,(int)ppm_data_1.hour, (int)ppm_data_1.min,(int) ppm_data_1.sec,(int) ppm_data_1.CO_ppm,ppm_data_1.CO2_ppm
	, (int)ppm_data_2.hour, (int)ppm_data_2.min, (int)ppm_data_2.sec, (int)ppm_data_2.CO_ppm, (int)ppm_data_2.CO2_ppm);
	USB_txbuffer[38]='\r';
	USB_txbuffer[39]='\n';
}
void USB_data_miss_to_send()
{
	char hour_buf1[3] = {0}; // Lưu chuỗi th�?i gian
	char min_buf1[3] = {0};
	char sec_buf1[3] = {0};
	char co_high1[3] = {0};   // Lưu chuỗi giá trị CO
	char co_low1[3] = {0};   // Lưu chuỗi giá trị CO
	char co2_high1[3] = {0};   // Lưu chuỗi giá trị CO
	char co2_low1[3] = {0};  // Lưu chuỗi giá trị CO2
	uint16_t CO1;
	uint16_t CO21;
	uint8_t ho1;
	uint8_t mi1;
	uint8_t se1;

	char hour_buf2[3] = {0}; // Lưu chuỗi th�?i gian
	char min_buf2[3] = {0};
	char sec_buf2[3] = {0};
	char co_high2[3] = {0};   // Lưu chuỗi giá trị CO
	char co_low2[3] = {0};   // Lưu chuỗi giá trị CO
	char co2_high2[3] = {0};   // Lưu chuỗi giá trị CO
	char co2_low2[3] = {0};  // Lưu chuỗi giá trị CO2
	uint16_t CO2;
	uint16_t CO22;
	uint8_t ho2;
	uint8_t mi2;
	uint8_t se2;

	// Tách chuỗi từ buffer
	hour_buf1[0] = RS_rxBuffer[5];
	hour_buf1[1] = RS_rxBuffer[5+1];
	hour_buf1[2] = '\0';

	min_buf1[0] = RS_rxBuffer[5+2];
	min_buf1[1] = RS_rxBuffer[5+3];
	min_buf1[2] = '\0';

	sec_buf1[0] = RS_rxBuffer[5+4];
	sec_buf1[1] = RS_rxBuffer[5+5];
	sec_buf1[2] = '\0';

	co_high1[0] = RS_rxBuffer[5+6];
	co_high1[1] = RS_rxBuffer[5+7];
	co_high1[2] = '\0';

	co_low1[0] = RS_rxBuffer[5+8];
	co_low1[1] = RS_rxBuffer[5+9];
	co_low1[2] = '\0';

	co2_high1[0] = RS_rxBuffer[5+10];
	co2_high1[1] = RS_rxBuffer[5+11];
	co2_high1[2] = '\0';

	co2_low1[0] = RS_rxBuffer[5+12];
	co2_low1[1] = RS_rxBuffer[5+13];
	co2_low1[2] = '\0';
// Tách chuỗi từ buffer
	hour_buf2[0] = RS_rxBuffer[19];
	hour_buf2[1] = RS_rxBuffer[19+1];
	hour_buf2[2] = '\0';

	min_buf2[0] = RS_rxBuffer[19+2];
	min_buf2[1] = RS_rxBuffer[19+3];
	min_buf2[2] = '\0';

	sec_buf2[0] = RS_rxBuffer[19+4];
	sec_buf2[1] = RS_rxBuffer[19+5];
	sec_buf2[2] = '\0';

	co_high2[0] = RS_rxBuffer[19+6];
	co_high2[1] = RS_rxBuffer[19+7];
	co_high2[2] = '\0';

	co_low2[0] = RS_rxBuffer[19+8];
	co_low2[1] = RS_rxBuffer[19+9];
	co_low2[2] = '\0';

	co2_high2[0] = RS_rxBuffer[19+10];
	co2_high2[1] = RS_rxBuffer[19+11];
	co2_high2[2] = '\0';

	co2_low2[0] = RS_rxBuffer[19+12];
	co2_low2[1] = RS_rxBuffer[19+13];
	co2_low2[2] = '\0';
	ho1 = (uint8_t)strtol(hour_buf1, NULL, 16);
	mi1 = (uint8_t)strtol(min_buf1, NULL, 16);
	se1 = (uint8_t)strtol(sec_buf1, NULL, 16);
	CO1 = ((uint8_t)(strtol(co_high1, NULL, 16)) << 8) | ((uint8_t)(strtol(co_low1, NULL, 16)));
	CO21 = ((uint8_t)(strtol(co2_high1, NULL, 16)) << 8) | ((uint8_t)(strtol(co2_low1, NULL, 16)));

	ho2 = (uint8_t)strtol(hour_buf2, NULL, 16);
	mi2 = (uint8_t)strtol(min_buf2, NULL, 16);
	se2 = (uint8_t)strtol(sec_buf2, NULL, 16);
	CO2 = ((uint8_t)(strtol(co_high2, NULL, 16)) << 8) | ((uint8_t)(strtol(co_low2, NULL, 16)));
	CO22 = ((uint8_t)(strtol(co2_high2, NULL, 16)) << 8) | ((uint8_t)(strtol(co2_low2, NULL, 16)));

	sprintf((char*)(&(USB_txbuffer[2])), "%02d%02d%02d%5d%5d%02d%02d%02d%5d%5d"
	,(int)ho1, (int)mi1,(int)se1,(int) CO1,CO21
	, (int)ho2, (int)mi2, (int)se2, (int)CO2, (int)CO22);
	USB_txbuffer[38]='\r';
	USB_txbuffer[39]='\n';

}
int check_disconnected (int node)
{
	if(node==1)
	{
		if(time_data_n1 -ppm1_time_temp ==0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if(time_data_n2 - ppm2_time_temp ==0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}
void Change_Time_Read_Data()
{
	if(ppm_data_1.CO2_ppm <=1000 && ppm_data_1.CO_ppm <=25 && ppm_data_2.CO2_ppm <=1000 && ppm_data_2.CO_ppm <=25)
	{
    	HAL_TIM_Base_Stop_IT(&htim2);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim2, T3);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim2);                 // Khởi động lại timer
	}
	else if(ppm_data_1.CO2_ppm >=2000 || ppm_data_1.CO_ppm >=50 || ppm_data_2.CO2_ppm >=2000 || ppm_data_2.CO_ppm >=50)
	{
    	HAL_TIM_Base_Stop_IT(&htim2);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim2, T1);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim2);                 // Khởi động lại timer
	}
	else
	{
    	HAL_TIM_Base_Stop_IT(&htim2);                  // Dừng timer
    	__HAL_TIM_SET_AUTORELOAD(&htim2, T2);     // Cập nhật chu kỳ
    	HAL_TIM_Base_Start_IT(&htim2);                 // Khởi động lại timer
	}
}
// gửi lệnh đ�?c v�? node đo theo th�?i gian
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)  // Kiểm tra nếu ngắt từ Timer 2
    {
    	if(receive_node_tong_flag==1)
    	{
			if(node_tong_to_node_1_error==1 && node_tong_to_node_2_error==0)
			{
				RS485_SendCommand(0x11, 0x21);  // Gửi lệnh 0x21 tới
				receive_node_tong_flag=0;
			}
			else if (node_tong_to_node_1_error==0 && node_tong_to_node_2_error==1)
			{
				RS485_SendCommand(0x11, 0x22);  // Gửi lệnh "22" tới
				receive_node_tong_flag=0;
			}
			else if (node_tong_to_node_1_error==1 && node_tong_to_node_2_error==1)
			{
				RS485_SendCommand(0x11, 0x23);  // Gửi lệnh "23" tới
				receive_node_tong_flag=0;
			}
			else
			{
				RS485_SendCommand(0x11, 0x02);  // Gửi lệnh "02" tới
				receive_node_tong_flag=0;
			}
    	}
    	else if(receive_node_tong_flag==0) // mất kết nối với node tổng
		{
        	RS485_SendCommand(0x11, 0x03);  // Gửi lệnh "03"
        	receive_node_tong_flag=0;
		}
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Nhận dữ liệu từ node tổng
    if (huart->Instance == USART1)
    {
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
    	if(check_lrc(&RS_rxBuffer[1],32,(char*)&RS_rxBuffer[33]) == 1)
    	{
    	 if (RS_rxBuffer[1] == '1' && RS_rxBuffer[2] == '1')
		 { // Kiểm tra địa chỉ
			receive_node_tong_flag=1; // c�? nhận được dữ liệu từ node tong
			if (RS_rxBuffer[3] == '0' && RS_rxBuffer[4] == '2')
			{
				HAL_TIM_Base_Start_IT(&htim2);
				ppm1_time_temp= time_data_n1;
				ppm2_time_temp=time_data_n2;
				decode_ppm2_ver3((char*)RS_rxBuffer, &ppm_data_1, 5);
				decode_ppm2_ver3((char*)RS_rxBuffer, &ppm_data_2, 19);
				Change_Time_Read_Data();
				USB_data_to_send();
				CDC_Transmit_FS(USB_txbuffer, 40); // gui qua USB
				make_time(&ppm_data_1, &time_data_n1);
				make_time(&ppm_data_2, &time_data_n2);
				if(check_disconnected (1)==0 && ppm1_time_temp!=0) // ppm1_time_temp khác 0 mới lỗi vì khi gửi các chuỗi của node 1 thì các số liệu node 2=0 hết
				{
  					node_tong_to_node_1_error =1;
				}
				else
				{
					node_tong_to_node_1_error=0;
				}
				if(check_disconnected (2)==0 && ppm2_time_temp!=0)
				{
					node_tong_to_node_2_error =1;
				}
				else
				{
					node_tong_to_node_2_error=0;
				}
				// Dữ liệu phải cập nhật (tức là không có lỗi đươgnf truyền từ node đo lên node tổng) thì mới gửi AA
				if((check_disconnected (1)==1 || ppm1_time_temp==0) && (check_disconnected (2)==1 || ppm2_time_temp==0))  // 1 trong 2 đúng ( cập nhật)
				{
					RS485_SendCommand(0x11, 0xAA);
				}
        	 }
        	 else if(RS_rxBuffer[3] == '0' && RS_rxBuffer[4] == '3') // nhan nhieu du lieu
        	 {
        		 HAL_TIM_Base_Stop_IT(&htim2);
        		 USB_data_miss_to_send();
				CDC_Transmit_FS(USB_txbuffer, 40); // gui qua USB
         		RS485_SendCommand(0x11, 0xBB); // gui ve node do la da luu thanh cong
        	 }
        	 else if(RS_rxBuffer[3] == '2' && RS_rxBuffer[4] == '1')
        	 {
        		 HAL_TIM_Base_Stop_IT(&htim2);
        		 USB_data_miss_to_send();
				CDC_Transmit_FS(USB_txbuffer, 40); // gui qua USB
        		 RS485_SendCommand(0x11, 0xCC); // gui ve node do la da gui thanh cong 1 chuoi de con gui tiep
        	 }
        	 else if(RS_rxBuffer[3] == '2' && RS_rxBuffer[4] == '2')
        	 {
        		 HAL_TIM_Base_Stop_IT(&htim2);
        		 USB_data_miss_to_send();
				CDC_Transmit_FS(USB_txbuffer,40 ); // gui qua USB
        		 RS485_SendCommand(0x11, 0xDD); // gui ve node do la da gui thanh cong 1 chuoi de con gui tiep
        	 }
        	 else if(RS_rxBuffer[3] == '2' && RS_rxBuffer[4] == '3')
        	 {
        		 HAL_TIM_Base_Stop_IT(&htim2);
        		 USB_data_miss_to_send();
				CDC_Transmit_FS(USB_txbuffer,40 ); // gui qua USB
        		 RS485_SendCommand(0x11, 0xDD); // gui ve node do la da gui thanh cong 1 chuoi de con gui tiep
        	 }
           }
    	}
 		rs_inx=0;
 		memset(RS_rxBuffer,0,sizeof(RS_rxBuffer));
     }
        // Tiếp tục nhận
        HAL_UART_Receive_IT(&huart1,rs_temp, 1);
    }
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, rs_temp, 1); // nhận dữ liệu từ node đo
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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
  htim2.Init.Prescaler = 48000-1;
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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
