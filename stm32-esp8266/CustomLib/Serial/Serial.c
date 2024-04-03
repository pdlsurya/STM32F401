/*
 * Serial.c
 *
 *  Created on: Jan 25, 2024
 *      Author: pdlsurya
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "USB_Serial.h"

#define MAX_BUF_SIZE 1024
#define SERIAL_TIMEOUT 1000 //in ms

static uint8_t serial_buffer[MAX_BUF_SIZE];
static char line_buffer[256];

static uint16_t read_index;
static uint16_t write_index;
static int byte_cnt;
static uint8_t byte;
static UART_HandleTypeDef *huart;
static uint32_t serial_cnt;
static bool timed_out = false;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *p_huart) {
	serial_buffer[write_index] = byte;
	write_index = (write_index + 1) % MAX_BUF_SIZE;
	byte_cnt++;
	serial_cnt++;
	if (byte_cnt == MAX_BUF_SIZE + 1) //if byte_cnt exceeds MAX_BUF_SIZE, wrap around the buffer.
		byte_cnt = 1;
	HAL_UART_Receive_IT(huart, &byte, 1);
}
/*Check if data is available in Serial buffer */
bool Serial_Available() {
	return ((byte_cnt != 0) && (byte_cnt > 0));
}

/*Wait until timeout for availability of next byte in Serial Buffer */
static bool NextByteAvailable() {
	uint32_t lastMillis = HAL_GetTick();
	while (!Serial_Available()) {
		if (HAL_GetTick() - lastMillis >= SERIAL_TIMEOUT) {
			timed_out = true;
			return false;
		}
	}
	timed_out = false;
	return true;
}

/*Flush Serial buffer*/
void Serial_Flush() {
	memset(serial_buffer, 0, sizeof(serial_buffer));
	read_index = 0;
	write_index = 0;
	byte_cnt = 0;

}

void Serial_Write(uint8_t *tx_data, uint16_t len) {
	HAL_UART_Transmit(huart, tx_data, len, HAL_MAX_DELAY);
}

uint8_t Serial_ReadByte() {
	uint8_t c = 0;
	if (NextByteAvailable()) {
		c = serial_buffer[read_index];
		read_index = (read_index + 1) % MAX_BUF_SIZE;
		byte_cnt--;
	}
	return c;

}

char* Serial_ReadLine() {
	uint8_t idx = 0;
	uint8_t c;
	memset(line_buffer, 0, sizeof(line_buffer));

	while (NextByteAvailable()) {

		c = serial_buffer[read_index];
		read_index = (read_index + 1) % MAX_BUF_SIZE;
		byte_cnt--;
		if (c == '\r')
			break;
		line_buffer[idx++] = c;
		if ((idx == sizeof(line_buffer)))
			goto exit;

	}
	if (timed_out)
		goto exit;
	read_index = (read_index + 1) % MAX_BUF_SIZE; //line feed
	byte_cnt--;

	exit: return line_buffer;
}

void Serial_Init(UART_HandleTypeDef *p_huart) {
	Serial_Flush();
	huart = p_huart;
	HAL_UART_Receive_IT(huart, (uint8_t*) &byte, 1); //Initiate UART reception
}

