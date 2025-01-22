/*
 * rs_related.c
 *
 *  Created on: Dec 19, 2024
 *      Author: DELL
 */
#include <rs_related_node_do.h>
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
void dec_to_ascii(uint8_t decimal, char *ascii_out) {
    sprintf(ascii_out, "%02d", decimal); // Chuyển đổi số thập phân thành chuỗi ASCII
}
void RS485_data_to_send()
{
	RS_txBuffer[0] = ':';
	hexdec_to_ascii(myID,(char*)&RS_txBuffer[1]);
	RS_txBuffer[3] = '0';
	RS_txBuffer[4] = '2';

	hexdec_to_ascii(gTime.Hours,(char*)&RS_txBuffer[5]);
	hexdec_to_ascii(gTime.Minutes,(char*)&RS_txBuffer[7]);
	hexdec_to_ascii(gTime.Seconds,(char*)&RS_txBuffer[9]);
	hexdec_to_ascii((uint8_t)(CO_ppm>>8),(char*)&RS_txBuffer[11]);
	hexdec_to_ascii((uint8_t)(CO_ppm),(char*)&RS_txBuffer[13]);
	hexdec_to_ascii((uint8_t)(CO2_ppm>>8),(char*)&RS_txBuffer[15]);
	hexdec_to_ascii((uint8_t)(CO2_ppm),(char*)&RS_txBuffer[17]);

	lrc = calculate_lrc(&RS_txBuffer[1],18);
	hexdec_to_ascii(lrc,(char*)&RS_txBuffer[19]);
	RS_txBuffer[21]=0x0D ;
	RS_txBuffer[22]=0x0A ;
}
