/*
 * flashProgram.h
 *
 *  Created on: Dec 31, 2023
 *      Author: pdlsurya
 */

#ifndef FLASHPROGRAM_FLASHPROGRAM_H_
#define FLASHPROGRAM_FLASHPROGRAM_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define APP_START_ADDRESS 0x08004000

#define RESET_HANDLER_ADDRESS (*((volatile uint32_t*)(APP_START_ADDRESS+4)))

#define STACK_TOP (*((volatile uint32_t*)APP_START_ADDRESS))

typedef void (*reset_handler_t)(void);

typedef enum {
	TYPE_DATA,
	TYPE_EOF,
	TYPE_ESAR,
	TYPE_ELAR = 0x04,
	TYPE_SLAR = 0x05
}recordType;

typedef struct{
	uint8_t length;
	uint16_t address;
	recordType type;
	uint8_t data[16];
	uint8_t checksum;
}hexRecord_t;

extern uint32_t app_start_address;

HAL_StatusTypeDef flash_process();

bool bootloader_init();

bool update_available();

void disable_peripheral_clock();


#endif /* FLASHPROGRAM_FLASHPROGRAM_H_ */
