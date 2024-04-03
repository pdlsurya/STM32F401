/*
 * NTP_Clock.c
 *
 *  Created on: Jan 22, 2024
 *      Author: pdlsurya
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "hal_esp8266.h"
#include "USB_Serial.h"
#include "NTP_Clock.h"
#include "oled_SH1106.h"

#define NTP_SERVER "pool.ntp.org"
#define NTP_SERVER_PORT 123
#define NTP_LOCAL_PORT 4343
#define NTP_TIMESTAMP_DELTA 2208988800ull
#define NTP_LINK 1

const char *weekdays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday" };

ntp_packet_t ntp_packet = { 0 };

const long nepal_offset = 20700; // UTC+5:45

static time_t epochTime;
static bool first_updated = false;
static bool update_received = false;
static uint32_t lastUpdate;
static uint32_t updateInterval = 120000; // in ms
static uint32_t requestInterval = 5000; //ms
static uint32_t lastRequested;

static void ntp_request_server() {
	memset((uint8_t*) &ntp_packet, 0, sizeof(ntp_packet_t));
	// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
	ntp_packet.li_vn_mode = 0b11100011;
	ntp_packet.poll = 11;

	esp8266_send_ip_data(NTP_LINK, (uint8_t*) &ntp_packet, sizeof(ntp_packet_t));

}

static void ntp_display_time(struct tm *time) {

	// Print the time we got from the server, accounting for local timezone and conversion from UTC time.
	//USB_SerialPrint("%d-%d-%d\n%d:%d:%d\n%s\n\n", time->tm_year + 1900,
	//time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min,
	//time->tm_sec, weekdays[time->tm_wday]);
	char formatted_date[10] = "";
	char formatted_time[9] = "";
	sprintf(formatted_date, "%d-%d-%d", time->tm_year + 1900, time->tm_mon + 1,
			time->tm_mday);
	sprintf(formatted_time, "%d:%d:%d", time->tm_hour, time->tm_min,
			time->tm_sec);

	oled_clearDisplay();
	oled_printString(weekdays[time->tm_wday], 0, 0, 16, false);
	oled_printString(formatted_date, 0, 16, 16, false);
	oled_print7Seg_number(formatted_time, 0, 40);
	oled_display();

}

void rx_cb_1(esp8266_rx_data_t *rx_data) {
	USB_SerialPrint("Update from NTP Server on Link:%d\n", rx_data->link_id);

	memset(&ntp_packet, 0, sizeof(ntp_packet_t));
	memcpy(&ntp_packet, rx_data->data, 48);
	if (ntp_packet.txTm_s == 0) //Invalid data received, return from here.
		return;

	update_received = true;
	if (!first_updated)
		first_updated = true;

	lastUpdate = HAL_GetTick();
}

void ntp_update_time() {

	if (!first_updated
			|| (((HAL_GetTick() - lastUpdate) >= updateInterval)
					&& !update_received)) {
		if (HAL_GetTick() - lastRequested >= requestInterval) {
			ntp_request_server();
			lastRequested = HAL_GetTick();
			return; //return immediatly for reading rx data
		}

	}

	struct tm *time;
	if (update_received) {
		update_received = false;
		ntp_packet.txTm_s = __builtin_bswap32(ntp_packet.txTm_s); // Time-stamp seconds.
		ntp_packet.txTm_f = __builtin_bswap32(ntp_packet.txTm_f); // Time-stamp fraction of a second.

		epochTime = (time_t) (ntp_packet.txTm_s - NTP_TIMESTAMP_DELTA
				+ nepal_offset);
		time = gmtime((const time_t*) &epochTime);
	} else {
		if (first_updated) {
			time_t epochTimeTemp = epochTime
					+ ((HAL_GetTick() - lastUpdate) / 1000);
			time = gmtime((const time_t*) &epochTimeTemp);
		} else
			return;
	}
	ntp_display_time(time);
}

bool ntp_begin() {
	esp8266_cb_register_ip(NTP_LINK, rx_cb_1);
	if (!esp8266_udp_begin(NTP_LINK, NTP_SERVER, NTP_SERVER_PORT,
	NTP_LOCAL_PORT, 0)) {
		return false;
	}
	return true;
}

