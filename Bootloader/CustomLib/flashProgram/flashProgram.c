/*
 * flashProgram.c
 *
 *  Created on: Dec 31, 2023
 *      Author: pdlsurya
 */

#include "flashProgram.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "mySdFat.h"
#include "stm32f4xx_hal.h"

uint32_t write_address_hi = 0x00000000; //upper 16-bits of the write address.
static myFile appHexFile;
static uint32_t byteCnt = 0;
static uint8_t recBuf_ASCII[43];
static uint8_t recBuf_HEX[21];
static bool erased = false;

static uint8_t getHexRecord_ASCII() {
	memset(recBuf_ASCII, 0, 43);
	uint8_t index = 0;
	uint8_t ch = 0;
	while (1) {
		ch = readByte(&appHexFile);
		byteCnt++;
		if (ch == '\r' || ch == '\n') {
			if (ch == '\r')
				ch = readByte(&appHexFile); //If first character is '\r', second should be '\n'.
			break;
		}
		recBuf_ASCII[index++] = ch;

		if (byteCnt == fileSize(&appHexFile))
			return index;

	}
	return index;

}

static uint8_t getHexByte(uint8_t *hexByte_ASCII) {
	uint8_t hexVal = 0;
	for (uint8_t i = 0; i < 2; i++) {
		if (hexByte_ASCII[i] >= 'A' && hexByte_ASCII[i] <= 'F')
			hexVal |= (hexByte_ASCII[i] - 55) << (4 * (1 - i));
		else
			hexVal |= (hexByte_ASCII[i] - 48) << (4 * (1 - i));
	}
	return hexVal;
}

static uint8_t getHexRecord_HEX(uint8_t len) {
	memset(recBuf_HEX, 0, 21);
	uint8_t index = 0;
	for (uint8_t i = 1; i < len; i += 2)
		recBuf_HEX[index++] = getHexByte(&recBuf_ASCII[i]);

	return index;

}

static uint8_t getCheckSum(uint8_t recordLen) {
	uint8_t res = 0;
	for (int i = 0; i < recordLen - 1; i++) {
		res += recBuf_HEX[i];
	}
	return ~res + 1;
}

static hexRecord_t getRecordStruct(uint8_t recordLen) {
	hexRecord_t record = { 0 };
	record.length = recBuf_HEX[0];

	record.address = recBuf_HEX[2];
	record.address |= ((uint16_t) recBuf_HEX[1]) << 8;

	record.type = recBuf_HEX[3];

	memcpy(record.data, &recBuf_HEX[4], record.length);
	record.checksum = recBuf_HEX[recordLen - 1];

	return record;
}

void disable_peripheral_clock() {
	HAL_RCC_DeInit(); //Turn off the PLL and set the clock to it's default state
	HAL_DeInit();	//Disable all the peripherals
}

bool bootloader_init() {
	if (!mySdFat_init()) {
		return false;
	}
	return true;

}

bool update_available() {
	appHexFile = pathExists("/STM32-BOOT/app.hex");

	if (startCluster(&appHexFile) == 0)
		return false;
	return true;
}

static uint8_t get_sectors_count(uint32_t image_size, uint32_t start_sector) {

	switch (start_sector) {
	case FLASH_SECTOR_0: {
		if (image_size <= 16384)
			return 1;
		else if (image_size > 16384 && image_size <= 32768)
			return 2;
		else if (image_size > 32768 && image_size <= 49152)
			return 3;
		else if (image_size > 49152 && image_size <= 65536)
			return 4;
		else if (image_size > 65536 && image_size <= 131072)
			return 5;
		else
			return 6;
		break;
	}
	case FLASH_SECTOR_1: {
		if (image_size <= 16384)
			return 1;
		else if (image_size > 16384 && image_size <= 32768)
			return 2;
		else if (image_size > 32768 && image_size <= 49152)
			return 3;
		else if (image_size > 49152 && image_size <= 114688)
			return 4;
		else
			return 5;
		break;
	}
	case FLASH_SECTOR_2: {
		if (image_size <= 16384)
			return 1;
		else if (image_size > 16384 && image_size <= 32768)
			return 2;
		else if (image_size > 32768 && image_size <= 98304)
			return 3;
		else
			return 4;
		break;
	}
	case FLASH_SECTOR_3: {
		if (image_size <= 16384)
			return 1;
		else if (image_size > 16384 && image_size <= 81920)
			return 2;
		else
			return 3;
		break;
	}
	case FLASH_SECTOR_4: {
		if (image_size <= 65536)
			return 1;
		else
			return 2;
		break;
	}
	case FLASH_SECTOR_5: {
		return 1;
		break;
	}
	default:
		return 0xFF;
		break;

	}
}

static uint32_t start_sector(uint32_t address) {
	if (address >= 0x08000000 && address <= 0x08003FFF)
		return FLASH_SECTOR_0;
	else if (address >= 0x08004000 && address <= 0x08007FFF)
		return FLASH_SECTOR_1;
	else if (address >= 0x08008000 && address <= 0x0800BFFF)
		return FLASH_SECTOR_2;
	else if (address >= 0x0800C000 && address <= 0x0800FFFF)
		return FLASH_SECTOR_3;
	else if (address >= 0x08010000 && address <= 0x0801FFFF)
		return FLASH_SECTOR_4;
	else if (address >= 0x08020000 && address <= 0x0803FFFF)
		return FLASH_SECTOR_5;
	return 0xFFFFFFFF;
}

static HAL_StatusTypeDef erase_flash(uint32_t start_address) {
	FLASH_EraseInitTypeDef EraseInit;
	uint32_t SectorError;
	uint32_t hexFileSize = fileSize(&appHexFile);

	uint32_t image_size = (uint32_t) ((double) hexFileSize / 2.8); //Approximate size of the flash image
	uint8_t start_sec = start_sector(start_address);
	EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInit.Sector = start_sec;
	EraseInit.NbSectors = get_sectors_count(image_size, start_sec);
	EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	return HAL_FLASHEx_Erase(&EraseInit, &SectorError);

}

static HAL_StatusTypeDef program_flash() {
	HAL_StatusTypeDef status = HAL_OK;

	while (1) {

		uint8_t len_ASCII = getHexRecord_ASCII();
		uint8_t len_HEX = getHexRecord_HEX(len_ASCII);
		uint8_t checkSum = getCheckSum(len_HEX);
		hexRecord_t record = getRecordStruct(len_HEX);
		uint32_t write_address = 0x00000000;

		if (checkSum == record.checksum) {

			switch (record.type) {
			case TYPE_ELAR: {
				uint16_t extended_address = (uint16_t) record.data[1];
				extended_address |= (((uint16_t) record.data[0]) << 8);
				write_address_hi = (((uint32_t) extended_address) << 16);
				break;
			}
			case TYPE_ESAR: {
				uint16_t extended_address = (uint16_t) record.data[1];
				extended_address |= (((uint16_t) record.data[0]) << 8);
				write_address_hi = (((uint32_t) extended_address) << 4);
				break;
			}
			case TYPE_DATA: {
				write_address = write_address_hi +
						((uint32_t) record.address);

				if (!erased) {
					status = erase_flash(write_address);
					if (status != HAL_OK)
						return status;
					erased = true;
				}

				for (uint8_t i = 0; i < record.length; i += 4) {
					status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
							write_address, *((uint32_t*) &record.data[i]));
					if (status != HAL_OK)
						return status;

					write_address += 4;
				}
				break;
			}
			case TYPE_SLAR: {
				break;
			}

			case TYPE_EOF: {

				return status;
				break;
			}

			}

		} else {
			return HAL_ERROR;
		}
	}

	return status;

}

HAL_StatusTypeDef flash_process() {

	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_FLASH_Unlock();

	if (status != HAL_OK)
		return status;

	status = program_flash();

	if (status != HAL_OK)
		return status;

	HAL_FLASH_Lock();

	fileClose(&appHexFile);

	fileDelete("/STM32-BOOT", "app.hex");

	return status;

}
