/*
 * rs_related.c
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */
#include <rs_related_node_tong.h>
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
	RS1_txBuffer[0] = ':';
    //RS1_txBuffer[1] = slaveID;  // �?ịa chỉ Slave
	hexdec_to_ascii(slaveID,(char*)&RS1_txBuffer[1]);
    //RS1_txBuffer[2] = command;  // Lệnh
	hexdec_to_ascii(command,(char*)&RS1_txBuffer[3]);
    //RS1_txBuffer[5] = '\n';     // Ký tự kết thúc
	uint8_t lrc = calculate_lrc(&RS1_txBuffer[1], 4);
	hexdec_to_ascii(lrc,(char*)&RS1_txBuffer[5]);
	RS1_txBuffer[7]=0x0D ;
	RS1_txBuffer[8]=0x0A ;
    HAL_UART_Transmit(&huart1, RS1_txBuffer, 10, HAL_MAX_DELAY);
}
void encode_ppm_data(PPM ppm_data, uint8_t *txBuffer, uint16_t start_index) {
    hexdec_to_ascii(ppm_data.hour, (char*)&txBuffer[start_index]);
    hexdec_to_ascii(ppm_data.min, (char*)&txBuffer[start_index + 2]);
    hexdec_to_ascii(ppm_data.sec, (char*)&txBuffer[start_index + 4]);
    hexdec_to_ascii((uint8_t)(ppm_data.CO_ppm >> 8), (char*)&txBuffer[start_index + 6]);
    hexdec_to_ascii((uint8_t)(ppm_data.CO_ppm), (char*)&txBuffer[start_index + 8]);
    hexdec_to_ascii((uint8_t)(ppm_data.CO2_ppm >> 8), (char*)&txBuffer[start_index + 10]);
    hexdec_to_ascii((uint8_t)(ppm_data.CO2_ppm), (char*)&txBuffer[start_index + 12]);
}
void decode_ppm_ver3 (char *RS1_rxBuffer,PPM *ppm)
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
    hour_buf[0] = RS1_rxBuffer[5];
    hour_buf[1] = RS1_rxBuffer[6];
    hour_buf[2] = '\0';

    min_buf[0] = RS1_rxBuffer[7];
    min_buf[1] = RS1_rxBuffer[8];
    min_buf[2] = '\0';

    sec_buf[0] = RS1_rxBuffer[9];
    sec_buf[1] = RS1_rxBuffer[10];
    sec_buf[2] = '\0';

    co_high[0] = RS1_rxBuffer[11];
    co_high[1] = RS1_rxBuffer[12];
    co_high[2] = '\0';

    co_low[0] = RS1_rxBuffer[13];
    co_low[1] = RS1_rxBuffer[14];
    co_low[2] = '\0';

    co2_high[0] = RS1_rxBuffer[15];
    co2_high[1] = RS1_rxBuffer[16];
    co2_high[2] = '\0';

    co2_low[0] = RS1_rxBuffer[17];
    co2_low[1] = RS1_rxBuffer[18];
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
