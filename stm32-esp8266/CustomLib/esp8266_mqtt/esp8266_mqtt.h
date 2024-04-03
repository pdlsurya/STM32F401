/*
 * esp8266_mqtt.h
 *
 *  Created on: Jan 27, 2024
 *      Author: pdlsurya
 */

#ifndef ESP8266_MQTT_ESP8266_MQTT_H_
#define ESP8266_MQTT_ESP8266_MQTT_H_

typedef struct {
	uint8_t link_id;
	uint8_t scheme;
	char *client_id;
	char *username;
	char *password;
	uint8_t cert_key_id;
	uint8_t ca_id;
	char *path;

} mqtt_user_cfg_t;

typedef struct {
	uint8_t link_id;
	char *broker;
	uint16_t port;
	bool reconnect;
} mqtt_connect_params_t;

typedef struct {
	uint8_t link_id;
	uint8_t len;
	char topic[20];
	char data[128];
} mqtt_data_t;

typedef struct {
	uint8_t link_id;
	char *topic;
	uint8_t qos;

} mqtt_subscribe_params_t;

typedef struct {
	uint8_t link_id;
	char *topic;
	char *data;
	uint8_t qos;
	bool retain;
} mqtt_publish_params_t;

typedef void (*mqtt_cb_t)(mqtt_data_t *mqtt_data);

void esp8266_mqtt_cb_register(uint8_t link_id, mqtt_cb_t mqtt_cb);

bool esp8266_mqtt_user_config(mqtt_user_cfg_t *user_config);

bool esp8266_mqtt_set_username(uint8_t link_id, char *username);

bool esp8266_mqtt_set_password(uint8_t link_id, char *password);

bool esp8266_mqtt_connect(mqtt_connect_params_t *connect_params);

bool esp8266_mqtt_disconnect(uint8_t link_id);

bool esp8266_mqtt_subscribe(mqtt_subscribe_params_t *subscribe_params);

bool esp8266_mqtt_unsubscribe(mqtt_subscribe_params_t *unsub_params);

bool esp8266_mqtt_publish(mqtt_publish_params_t *publish_params);

#endif /* ESP8266_MQTT_ESP8266_MQTT_H_ */
