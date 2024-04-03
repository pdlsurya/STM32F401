 /*
 * hal_esp8266.c
 *
 *  Created on: Jan 21, 2024
 *      Author: pdlsurya
 */

// Function to check if 'str' ends with 'suffix'
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "USB_Serial.h"
#include "Serial.h"
#include "hal_esp8266.h"
#include "oled_SH1106.h"
#include "esp8266_mqtt.h"

#define SSID "Surya_303184"
#define PASSWORD "ptk0405GJH@303184"

#define TIMEOUT 1000 //in ms
static esp8266_rx_cb_t rx_callbacks[5];

static mqtt_cb_t mqtt_callbacks[5];

static esp8266_rx_data_t rx_data = { 0 };

static char ip_address[15] = "";
static char wifi[20] = "";

void esp8266_cb_register_ip(uint8_t link_id, esp8266_rx_cb_t callback) {
	rx_callbacks[link_id] = callback;
}

void esp8266_cb_register_mqtt(uint8_t link_id, mqtt_cb_t mqtt_cb) {
	mqtt_callbacks[link_id] = mqtt_cb;
}

static bool WaitForResponse(char *response, uint32_t timeout) {
	char *prx;
	uint32_t lastMillis = HAL_GetTick();
	lastMillis = HAL_GetTick();
	do {
		prx = Serial_ReadLine();
		USB_SerialPrint("%s\n", prx);
		if (strcmp(prx, "ERROR") == 0)
			return false;
		if (HAL_GetTick() - lastMillis >= timeout)
			return false;

	} while ((strcmp(response, prx) != 0));

	return true;
}

bool isIpData(const char *data) {
	return strncmp(data, "+IPD", 4) == 0;
}

bool isMqttData(const char *data) {
	return strncmp(data, "+MQTTSUBRECV", 12) == 0;
}

bool esp8266_recv() {
	char *prx;
	if (Serial_Available()) {
		memset((uint8_t*) &rx_data, 0, sizeof(esp8266_rx_data_t));
		prx = Serial_ReadLine();

		if (isIpData(prx)) {
			rx_data.link_id = (uint8_t) prx[5] - 48;
			uint8_t index = 7;
			while (prx[index] != ':') {

				rx_data.len = rx_data.len * 10 + ((uint8_t) prx[index] - 48);
				index++;
				if (index == 12)
					return false;
			}
			memcpy(rx_data.data, &prx[index + 1], rx_data.len);
			rx_callbacks[rx_data.link_id](&rx_data);
			return true;
		} else if (isMqttData(prx)) {

			mqtt_data_t mqtt_data = { 0 };
			mqtt_data.link_id = (uint8_t) prx[13] - 48;

			int r_idx = 16;
			int w_idx = 0;
			while (prx[r_idx] != '"') {
				mqtt_data.topic[w_idx++] = prx[r_idx];
				r_idx++;
			}
			r_idx += 2;
			while (prx[r_idx] != ',') {

				mqtt_data.len = mqtt_data.len * 10
						+ ((uint8_t) prx[r_idx] - 48);
				r_idx++;
			}
			memcpy(mqtt_data.data, &prx[r_idx + 1], mqtt_data.len);
			mqtt_callbacks[mqtt_data.link_id](&mqtt_data);
			return true;

		} else
			return false;
	}
	return false;

}

static inline void esp8266_tx(uint8_t *tx_data, uint16_t len) {
	Serial_Write(tx_data, len);
}

bool esp8266_send_cmd(char *cmd, uint32_t timeout) {
	esp8266_tx((uint8_t*) cmd, strlen(cmd));
	return WaitForResponse("OK", timeout);

}

bool esp8266_send_ip_data(uint8_t link_id, uint8_t *data, uint16_t len) {
	char cmd_buf[20] = "";
	uint8_t cmd_len = sprintf(cmd_buf, "at+cipsend=%d,%d\r\n", link_id, len);
	esp8266_tx((uint8_t*) cmd_buf, cmd_len);
	for (uint8_t i = 0; i < 3; i++) //'>' occurs on the fourth line
		Serial_ReadLine();
	if (Serial_ReadByte() != '>')
		return false;

	esp8266_tx(data, len);
	return WaitForResponse("SEND OK", TIMEOUT);

}

bool esp8266_udp_begin(uint8_t link_id, char *remote_ip, uint16_t remote_port,
		uint16_t local_port, uint8_t mode) {
	char tx_buf[64] = "";
	sprintf(tx_buf, "at+cipstart=%d,\"UDP\",\"%s\",%d,%d,%d\r\n", link_id,
			remote_ip, remote_port, local_port, mode);
	return esp8266_send_cmd(tx_buf, TIMEOUT);

}

bool esp8266_wifi_connect() {
	char conn_cmd[50] = "";
	int cmdlen = sprintf(conn_cmd, "at+cwjap=\"%s\",\"%s\"\r\n", SSID,
	PASSWORD);
	Serial_Flush();
	esp8266_send_cmd("at+cwmode=1\r\n", TIMEOUT);
	esp8266_tx((uint8_t*) conn_cmd, cmdlen);

	USB_SerialPrint("Connecting to Wifi:%s\n", SSID);

	return WaitForResponse("OK", 5000); //Max wait for 5seconds

}

static void get_ip_address() {
	char *prx;
	char *cmd = "at+cifsr\r\n";
	esp8266_tx((uint8_t*) cmd, strlen(cmd));
	prx = Serial_ReadLine();
	if (strncmp(prx, "+CIFSR", 6) == 0) {
		int r_idx = 14;
		int w_idx = 0;
		memset(ip_address, 0, sizeof(ip_address));
		while (prx[r_idx] != '"') {
			ip_address[w_idx++] = prx[r_idx];
			r_idx++;
		}
		if (!WaitForResponse("OK", TIMEOUT))
			return;

	}

}

static void get_wifi_details() {
	char *prx;
	char *cmd = "at+cwjap?\r\n";
	esp8266_tx((uint8_t*) cmd, strlen(cmd));
	prx = Serial_ReadLine();

	if (strncmp(prx, "+CWJAP", 6) == 0) {
		int r_idx = 8;
		int w_idx = 0;
		memset(wifi, 0, sizeof(wifi));
		while (prx[r_idx] != '"') {
			wifi[w_idx++] = prx[r_idx];
			r_idx++;
		}
		if (!WaitForResponse("OK", TIMEOUT))
			return;

		USB_SerialPrint("Connected to WiFi: %s\n", wifi);

	}

}

bool esp8266_reset() {
	char *cmd = "at+rst\r\n";
	esp8266_tx((uint8_t*) cmd, strlen(cmd));
	return WaitForResponse("WIFI GOT IP", 4000);

}

bool esp8266_init(UART_HandleTypeDef *huart) {

	Serial_Init(huart);
	if (!esp8266_reset()) { // if reset function returns false, there is a chance that wifi is not connected. Try to connect here.
		if (!esp8266_wifi_connect())
			return false;

	}
	esp8266_send_cmd("ate0\r\n", TIMEOUT); //Disable command echo
	get_wifi_details();
	get_ip_address();
	oled_clearDisplay();
	oled_printString("WiFi SSID:", 0, 0, 16, true);
	oled_printString(wifi, 0, 16, 16, false);
	oled_printString("IP Address:", 0, 32, 16, true);
	oled_printString(ip_address, 0, 48, 16, false);
	oled_display();
	if (!esp8266_send_cmd("at+cipmux=1\r\n", TIMEOUT))
		return false;
	return true;

}

