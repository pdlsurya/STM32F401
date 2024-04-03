/*
 * hal_esp8266.h
 *
 *  Created on: Jan 21, 2024
 *      Author: pdlsurya
 */

#ifndef HAL_ESP8266_HAL_ESP8266_H_
#define HAL_ESP8266_HAL_ESP8266_H_

#include "esp8266_mqtt.h"

typedef struct {
	uint8_t link_id;
	uint16_t len;
	char data[100];
} esp8266_rx_data_t;

typedef void (*esp8266_rx_cb_t)(esp8266_rx_data_t*);

bool esp8266_init(UART_HandleTypeDef *huart);

bool esp8266_udp_begin(uint8_t link_id, char *remote_ip, uint16_t remote_port,
		uint16_t local_port,uint8_t mode);

bool esp8266_send_ip_data(uint8_t link_id, uint8_t *data, uint16_t len);

bool esp8266_recv();

void esp8266_cb_register_ip(uint8_t link_id, esp8266_rx_cb_t callback);

void esp8266_cb_register_mqtt(uint8_t link_id, mqtt_cb_t mqtt_cb);

bool esp8266_send_cmd(char *cmd, uint32_t timeout);

#endif /* HAL_ESP8266_HAL_ESP8266_H_ */
