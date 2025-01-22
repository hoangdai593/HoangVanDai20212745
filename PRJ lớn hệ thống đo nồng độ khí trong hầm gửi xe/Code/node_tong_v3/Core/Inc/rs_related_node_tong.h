/*
 * rs_related.h
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */

#ifndef INC_RS_RELATED_NODE_TONG_H_
#define INC_RS_RELATED_NODE_TONG_H_
#include "main.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>

// Định nghĩa kiểu dữ liệu PPM

extern UART_HandleTypeDef huart1;
extern uint8_t RS1_txBuffer[20];  // Khai báo mảng RS1_txBuffer
// Khai báo các hàm trong rs_related.c
uint8_t calculate_lrc(uint8_t *ascii_data, uint16_t length);
void hexdec_to_ascii(uint8_t hexdec, char *ascii_out);
void RS485_SendCommand(uint8_t slaveID, uint8_t command);
void encode_ppm_data(PPM ppm_data, uint8_t *txBuffer, uint16_t start_index);
void decode_ppm_ver3(char *RS1_rxBuffer, PPM *ppm);
uint8_t check_lrc(uint8_t *ascii_data, uint16_t length, char* ascii_lrc);



#endif /* INC_RS_RELATED_NODE_TONG_H_ */
