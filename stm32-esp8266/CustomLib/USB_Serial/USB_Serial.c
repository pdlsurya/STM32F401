/*
 * USB_Serial.c
 *
 *  Created on: Dec 30, 2023
 *      Author: pdlsurya
 */
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "stdarg.h"
#include "main.h"

static uint8_t retry_cnt=0;

void USB_SerialInit() {
	MX_USB_DEVICE_Init();

}

void USB_SerialPrint(char *format, ...) {

	char buffer[128];
	va_list args;
	va_start(args, format);
	size_t size=vsprintf(buffer, format, args);
	perror(buffer);
	va_end(args);

	while(CDC_Transmit_FS((uint8_t*) buffer, size)==(USBD_FAIL || USBD_BUSY)){
		DelayUS(1);
		retry_cnt++;
		if(retry_cnt==200)
			break;
	}
	retry_cnt=0;

}

