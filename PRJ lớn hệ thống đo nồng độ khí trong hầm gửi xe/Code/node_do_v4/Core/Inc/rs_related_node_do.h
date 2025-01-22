/*
 * rs_related.h
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */

#ifndef INC_RS_RELATED_NODE_DO_H_
#define INC_RS_RELATED_NODE_DO_H_
#include "main.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>

// Định nghĩa kiểu dữ liệu PPM

extern UART_HandleTypeDef huart1;
extern uint8_t RS_txBuffer[30];  // Khai báo mảng RS_txBuffer
extern uint8_t lrc;  // Biến lưu giá trị LRC tính toán
extern RTC_TimeTypeDef gTime;
extern uint16_t CO_ppm;
extern uint16_t CO2_ppm;
extern uint8_t myID;

// Hàm tính LRC
uint8_t calculate_lrc(uint8_t *ascii_data, uint16_t length);

// Mã hóa số HEX thành chuỗi ASCII
void hexdec_to_ascii(uint8_t hexdec, char *ascii_out);

// Mã hóa số thập phân thành chuỗi ASCII
void dec_to_ascii(uint8_t decimal, char *ascii_out);

// Hàm chuẩn bị dữ liệu RS485 để gửi
void RS485_data_to_send(void);

uint8_t check_lrc(uint8_t *ascii_data, uint16_t length, char* ascii_lrc);


#endif /* INC_RS_RELATED_NODE_DO_H_ */
