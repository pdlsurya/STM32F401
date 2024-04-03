/*
 * esp8266_mqtt.c
 *
 *  Created on: Jan 27, 2024
 *      Author: pdlsurya
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "USB_Serial.h"
#include "hal_esp8266.h"
#include "esp8266_mqtt.h"

bool esp8266_mqtt_user_config(mqtt_user_cfg_t *user_config) {
	char cmd[100] = "";
	sprintf(cmd, "at+mqttusercfg=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"\r\n",
			user_config->link_id, user_config->scheme, user_config->client_id,
			user_config->username, user_config->password,
			user_config->cert_key_id, user_config->ca_id, user_config->path);
	return esp8266_send_cmd(cmd, 5000);
}

void esp8266_mqtt_cb_register(uint8_t link_id, mqtt_cb_t callback) {
	esp8266_cb_register_mqtt(link_id, callback);
}

bool esp8266_mqtt_connect(mqtt_connect_params_t *conn_params) {
	char cmd[100] = "";
	sprintf(cmd, "at+mqttconn=%d,\"%s\",%d,%d\r\n", conn_params->link_id,
			conn_params->broker, conn_params->port, conn_params->reconnect);
	return esp8266_send_cmd(cmd, 5000);
}

bool esp8266_mqtt_subscribe(mqtt_subscribe_params_t *subscribe_params) {
	char cmd[100] = "";
	sprintf(cmd, "at+mqttsub=%d,\"%s\",%d\r\n", subscribe_params->link_id,
			subscribe_params->topic, subscribe_params->qos);
	return esp8266_send_cmd(cmd, 2000);
}

bool esp8266_mqtt_unsubscribe(mqtt_subscribe_params_t *unsub_params) {
	char cmd[100] = "";
	sprintf(cmd, "at+mqttunsub=%d,\"%s\"\r\n", unsub_params->link_id,
			unsub_params->topic);
	return esp8266_send_cmd(cmd, 2000);
}

bool esp8266_mqtt_publish(mqtt_publish_params_t *publish_params) {
	char cmd[256] = "";
	sprintf(cmd, "at+mqttpub=%d,\"%s\",\"%s\",%d,%d\r\n",
			publish_params->link_id, publish_params->topic,
			publish_params->data, publish_params->qos, publish_params->retain);
	return esp8266_send_cmd(cmd, 5000);
}

bool esp8266_mqtt_disconnect(uint8_t link_id) {
	char cmd[50] = "";
	sprintf(cmd, "at+mqttclean=%d\r\n", link_id);
	return esp8266_send_cmd(cmd, 2000);
}
