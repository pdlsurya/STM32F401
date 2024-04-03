/**
 * @file SD_driver.c
 * @author Surya Poudel (poudel.surya2011@gmail.com)
 * @brief Driver for SD/MMC Card connected to SPI bus
 * @version 0.1
 * @date 2023-07-05
 *
 * @copyright Copyright(c) 2023, Surya Poudel
 */

#include <stdint.h>
#include "SD_driver.h"
#include "stm32f4xx_hal.h"

#define CMD0 0
#define CMD0_ARG 0x00000000
#define CMD0_CRC 0x94

// SEND IF_COND
#define CMD8 8
#define CMD8_ARG 0x000001AA
#define CMD8_CRC 0x86 //(1000011 << 1)

// READ CSD
#define CMD9 9
#define CMD9_ARG 0x00000000
#define CMD9_CRC 0x00

// Read OCR
#define CMD58 58
#define CMD58_ARG 0x00000000
#define CMD58_CRC 0x00

#define CMD55 55
#define CMD55_ARG 0x00000000
#define CMD55_CRC 0x00

#define ACMD41 41
#define ACMD41_ARG 0x40000000
#define ACMD41_CRC 0x00

// Read Single Block
#define CMD17 17
#define CMD17_CRC 0x95
#define SD_MAX_READ_ATTEMPTS 4500

// Write Single Block
#define CMD24 24
#define CMD24_CRC 0x00
#define SD_MAX_WRITE_ATTEMPTS 3907

// Read Multiple Block
#define CMD18 18
#define CMD18_CRC 0x00

// STOP_MULTIPLE_READ
#define CMD12 12
#define CMD12_ARG 0x00000000
#define CMD12_CRC 0x00

// Write Multiple Block
#define CMD25 25
#define CMD25_CRC 0x00

#define PARAM_ERROR(X) X & 0b01000000
#define ADDR_ERROR(X) X & 0b00100000
#define ERASE_SEQ_ERROR(X) X & 0b00010000
#define CRC_ERROR(X) X & 0b00001000
#define ILLEGAL_CMD(X) X & 0b00000100
#define ERASE_RESET(X) X & 0b00000010
#define IN_IDLE(X) X & 0b00000001

#define CMD_VER(X) ((X >> 4) & 0x0F)
#define VOL_ACC(X) (X & 0x1F)

#define VOLTAGE_ACC_27_33 0b00000001
#define VOLTAGE_ACC_LOW 0b00000010
#define VOLTAGE_ACC_RES1 0b00000100
#define VOLTAGE_ACC_RES2 0b00001000

#define POWER_UP_STATUS(X) X & 0x80
#define CCS_VAL(X) X & 0x40
#define VDD_2728(X) X & 0b10000000
#define VDD_2829(X) X & 0b00000001
#define VDD_2930(X) X & 0b00000010
#define VDD_3031(X) X & 0b00000100
#define VDD_3132(X) X & 0b00001000
#define VDD_3233(X) X & 0b00010000
#define VDD_3334(X) X & 0b00100000
#define VDD_3435(X) X & 0b01000000
#define VDD_3536(X) X & 0b10000000

#define SD_TOKEN_OOR(X) X & 0b00001000
#define SD_TOKEN_CECC(X) X & 0b00000100
#define SD_TOKEN_CC(X) X & 0b00000010
#define SD_TOKEN_ERROR(X) X & 0b00000001

#define SD_START_TOKEN 0xFE
#define SD_BLOCK_LEN 512

#define CS_DISABLE() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define CS_ENABLE()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)

SPI_HandleTypeDef hspi1;

static void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

static inline uint8_t SPI_transfer(uint8_t tx_Byte) {
	uint8_t rx_Byte;
	HAL_SPI_TransmitReceive(&hspi1, &tx_Byte, &rx_Byte, 1, HAL_MAX_DELAY);
	while (hspi1.State == HAL_SPI_STATE_BUSY)
		;  // wait xmission complete
	return rx_Byte;
}

void SD_powerUpSeq() {
	// make sure card is deselected
	CS_DISABLE();

	// give SD card time to power up
	HAL_Delay(1);

	// send 80 clock cycles to synchronize
	for (uint8_t i = 0; i < 20; i++)
		SPI_transfer(0xFF);

	// deselect SD card
	CS_DISABLE();
	SPI_transfer(0xFF);
}

void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
	// transmit command to sd card
	SPI_transfer(cmd | 0x40);

	// transmit argument
	SPI_transfer((uint8_t) (arg >> 24));
	SPI_transfer((uint8_t) (arg >> 16));
	SPI_transfer((uint8_t) (arg >> 8));
	SPI_transfer((uint8_t) (arg));

	// transmit crc
	SPI_transfer(crc | 0x01);
}

uint8_t SD_readRes1() {
	uint8_t i = 0, res1;

	// keep polling until actual data received
	while ((res1 = SPI_transfer(0xFF)) == 0xFF) {
		i++;

		// if no data received for 8 bytes, break
		if (i > 8)
			break;
	}

	return res1;
}

uint8_t SD_goIdleState() {
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD0
	SD_command(CMD0, CMD0_ARG, CMD0_CRC);

	// read response
	uint8_t res1 = SD_readRes1();

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}

void SD_readRes3_7(uint8_t *res) {
	// read response 1 in R7
	res[0] = SD_readRes1();

	// if error reading R1, return
	if (res[0] > 1)
		return;

	// read remaining bytes
	res[1] = SPI_transfer(0xFF);
	res[2] = SPI_transfer(0xFF);
	res[3] = SPI_transfer(0xFF);
	res[4] = SPI_transfer(0xFF);
}

void SD_sendIfCond(uint8_t *res) {
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD8
	SD_command(CMD8, CMD8_ARG, CMD8_CRC);

	// read response
	SD_readRes3_7(res);

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
}

void SD_readOCR(uint8_t *res) {
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD58
	SD_command(CMD58, CMD58_ARG, CMD58_CRC);

	// read response
	SD_readRes3_7(res);

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
}

uint8_t SD_sendApp() {
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD0
	SD_command(CMD55, CMD55_ARG, CMD55_CRC);

	// read response
	uint8_t res1 = SD_readRes1();

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}

uint8_t SD_sendOpCond() {
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD0
	SD_command(ACMD41, ACMD41_ARG, ACMD41_CRC);

	// read response
	uint8_t res1 = SD_readRes1();

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}
/*
void SD_printR1(uint8_t res) {
	if (res & 0b10000000) {
		////USB_SerialPrint("\tError: MSB = 1\n\r");
		return;
	}
	if (res == 0) {
		////USB_SerialPrint("\t Card Ready \n\r");
		return;
	}
	if (PARAM_ERROR(res))
		//USB_SerialPrint("\tParameter Error\n\r");
	if (ADDR_ERROR(res))
		//USB_SerialPrint("\tAddress Error\n\r");
	if (ERASE_SEQ_ERROR(res))
		//USB_SerialPrint("\tErase Seq Error\n\r");
	if (CRC_ERROR(res))
		//USB_SerialPrint("\tCRC Error\n\r");
	if (ILLEGAL_CMD(res))
		//USB_SerialPrint("\tIllegal Cmd\n\r");
	if (ERASE_RESET(res))
		//USB_SerialPrint("\tErase Rst Error\n\r");
	if (IN_IDLE(res))
		//USB_SerialPrint("Idle State\n\r");
}

void SD_printR7(uint8_t *res) {
	SD_printR1(res[0]);

	if (res[0] > 1)
		return;

	//USB_SerialPrint("\tCommand Version: ");
	//USB_SerialPrint("%x\n", CMD_VER(res[1]));

	//USB_SerialPrint("\tVoltage Accepted: ");
	if (VOL_ACC(res[3]) == VOLTAGE_ACC_27_33)
		//USB_SerialPrint("2.7-3.6V\n\r");
	else if (VOL_ACC(res[3]) == VOLTAGE_ACC_LOW)
		//USB_SerialPrint("LOW VOLTAGE\n\r");
	else if (VOL_ACC(res[3]) == VOLTAGE_ACC_RES1)
		//USB_SerialPrint("RESERVED\n\r");
	else if (VOL_ACC(res[3]) == VOLTAGE_ACC_RES2)
		//USB_SerialPrint("RESERVED\n\r");
	else
		//USB_SerialPrint("NOT DEFINED\n");

	//USB_SerialPrint("\tEcho: ");
	//USB_SerialPrint("%x \n", res[4]);
}

void SD_printR3(uint8_t *res) {
	SD_printR1(res[0]);

	if (res[0] > 1)
		return;

	//USB_SerialPrint("\tCard Power Up Status: ");
	if (POWER_UP_STATUS(res[1])) {
		//USB_SerialPrint("READY\r\n");
		//USB_SerialPrint("\tCCS Status: ");
		if (CCS_VAL(res[1])) {
			//USB_SerialPrint("1\r\n");
		} else
			//USB_SerialPrint("0\r\n");
	} else {
		//USB_SerialPrint("BUSY\r\n");
	}

	//USB_SerialPrint("\tVDD Window: ");
	if (VDD_2728(res[3]))
		//USB_SerialPrint("2.7-2.8, ");
	if (VDD_2829(res[2]))
		//USB_SerialPrint("2.8-2.9, ");
	if (VDD_2930(res[2]))
		//USB_SerialPrint("2.9-3.0, ");
	if (VDD_3031(res[2]))
		//USB_SerialPrint("3.0-3.1, ");
	if (VDD_3132(res[2]))
		//USB_SerialPrint("3.1-3.2, ");
	if (VDD_3233(res[2]))
		//USB_SerialPrint("3.2-3.3, ");
	if (VDD_3334(res[2]))
		//USB_SerialPrint("3.3-3.4, ");
	if (VDD_3435(res[2]))
		//USB_SerialPrint("3.4-3.5, ");
	if (VDD_3536(res[2]))
		//USB_SerialPrint("3.5-3.6");
	//USB_SerialPrint("\r\n");
}

void SD_printDataErrToken(uint8_t token) {
	if (SD_TOKEN_OOR(token))
		//USB_SerialPrint("\tData out of range\r\n");
	if (SD_TOKEN_CECC(token))
		//USB_SerialPrint("\tCard ECC failed\r\n");
	if (SD_TOKEN_CC(token))
		//USB_SerialPrint("\tCC Error\r\n");
	if (SD_TOKEN_ERROR(token))
		//USB_SerialPrint("\tError\r\n");
}
*/

uint8_t SD_read_start(uint8_t *buf, uint16_t read_len, uint8_t *token) {
	uint8_t res1, read;
	uint16_t readAttempts;

	// read R1
	res1 = SD_readRes1();

	// if response received from card
	if (res1 == SD_READY) {
		// wait for a response token (timeout = 100ms)
		readAttempts = 0;

		while ((read = SPI_transfer(0xFF)) != 0xFE) {

			if (readAttempts == SD_MAX_READ_ATTEMPTS)
				break;
			readAttempts++;
		}

		// if response token is 0xFE
		if (read == 0xFE) {
			// read 512 byte block
			for (uint16_t i = 0; i < read_len; i++)
				*buf++ = SPI_transfer(0xFF);

			// read 16-bit CRC
			SPI_transfer(0xFF);
			SPI_transfer(0xFF);
		}

		// set token to card response
		*token = read;
	}

	return res1;
}

uint8_t SD_readCSD(uint8_t *CSD) {
	uint8_t token, res1;
	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD0
	SD_command(CMD9, CMD9_ARG, CMD9_CRC);

	res1 = SD_read_start(CSD, 16, &token);

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	if (res1 == SD_READY) {
		// if error token received
		if (!(token & 0xF0)) {
			// SD_printDataErrToken(token);
			return SD_READ_ERROR;
		} else if (token == 0xFF) {
			//USB_SerialPrint("Read Timeout\r\n");
			return SD_READ_ERROR;
		}
		return SD_READ_SUCCESS;
	} else {
		// SD_printR1(res1);
		return SD_READ_ERROR;
	}
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

uint8_t SD_init() {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	MX_SPI1_Init();

	/*Configure GPIO pin : PA4 for Chip Select*/
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	uint8_t res[5], cmdAttempts = 0;

	SD_powerUpSeq();

	// command card to idle
	while ((res[0] = SD_goIdleState()) != 0x01) {
		cmdAttempts++;
		if (cmdAttempts > 50) {
			if (res[0] == 0)
				//USB_SerialPrint("Card Not Found!\r\n");

			// SD_printR1(res[0]);
			return SD_INIT_ERROR;
		}
	}

	// send interface conditions
	SD_sendIfCond(res);
	if (res[0] != 0x01) {
		// SD_printR1(res[0]);
		return SD_INIT_ERROR;
	}

	// check echo pattern
	if (res[4] != 0xAA) {
		// SD_printR7(res);
		return SD_INIT_ERROR;
	}

	// attempt to initialize card
	cmdAttempts = 0;
	do {
		if (cmdAttempts > 100)
			return SD_INIT_ERROR;

		// send app cmd
		res[0] = SD_sendApp();

		// if no error in response
		if (res[0] < 2) {
			res[0] = SD_sendOpCond();
		}

		// wait
		HAL_Delay(10);

		cmdAttempts++;
	} while (res[0] != SD_READY);

	// read OCR
	SD_readOCR(res);
	// check card is ready
	if (!(res[1] & 0x80)) {
		// SD_printR3(res);
		return SD_INIT_ERROR;
	} else {
		//if (res[1] & 0x40)
			//USB_SerialPrint("Card Type: SDHC\r\n");
	}
	return SD_INIT_SUCCESS;
}

uint8_t SD_readSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token) {
	// set token to none
	*token = 0xFF;

	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD17
	SD_command(CMD17, addr, CMD17_CRC);

	uint8_t res1 = SD_read_start(buf, SD_BLOCK_LEN, token);

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}

uint8_t SD_readSector(uint32_t addr, uint8_t *buf) {
	uint8_t res1, token;

	res1 = SD_readSingleBlock(addr, buf, &token);
	if (res1 == SD_READY) {
		// if error token received
		if (!(token & 0xF0)) {
			//SD_printDataErrToken(token);
			return SD_READ_ERROR;
		} else if (token == 0xFF) {
			//USB_SerialPrint("Read Timeout\r\n");
			return SD_READ_ERROR;
		}
		return SD_READ_SUCCESS;
	} else {
		// SD_printR1(res1);
		return SD_READ_ERROR;
	}
}

uint8_t _writeSingleBlock(uint32_t addr, uint8_t *buf, uint8_t *token) {
	uint8_t writeAttempts, read, res1;

	// set token to none
	*token = 0xFF;

	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD24
	SD_command(CMD24, addr, CMD24_CRC);

	// read response
	res1 = SD_readRes1();

	// if no error
	if (res1 == SD_READY) {
		// send start token
		SPI_transfer(SD_START_TOKEN);

		// write buffer to card
		for (uint16_t i = 0; i < SD_BLOCK_LEN; i++)
			SPI_transfer(buf[i]);
		// wait for a response (timeout = 250ms)
		writeAttempts = 0;

		while (writeAttempts != SD_MAX_WRITE_ATTEMPTS) {
			if ((read = SPI_transfer(0xFF)) != 0xFF)
				break;
			writeAttempts++;
		}
		// if data accepted
		if ((read & 0x1F) == 0x05) {
			// set token to data accepted
			*token = 0x05;

			// wait for write to finish (timeout = 250ms)
			writeAttempts = 0;
			while (SPI_transfer(0xFF) == 0x00) {
				if (writeAttempts == SD_MAX_WRITE_ATTEMPTS) {
					*token = 0x00;
					break;
				}
				writeAttempts++;
			}
		}
	}
	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}

uint8_t SD_writeSector(uint32_t addr, uint8_t *buf) {
	uint8_t token, res1;
	res1 = _writeSingleBlock(addr, buf, &token);

	if (res1 == SD_READY) {
		if (token == 0x05)
			return SD_WRITE_SUCCESS;
		else if (token == 0xFF || token == 0x00)
			return SD_WRITE_ERROR;
	} else {
		// SD_printR1(res1);
		return SD_WRITE_ERROR;
	}
	return SD_WRITE_ERROR;
}

uint8_t SD_readMultipleSecStart(uint32_t start_addr) {
	uint8_t res1;

	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD24
	SD_command(CMD18, start_addr, CMD18_CRC);

	// read response
	res1 = SD_readRes1();

	return res1;
}

sd_ret_t SD_readMultipleSec(uint8_t *buff) {
	uint8_t read = 0xFF;
	uint32_t readAttempts;

	// wait for a response token (timeout = 100ms)
	readAttempts = 0;

	while ((read = SPI_transfer(0xFF)) != 0xFE) {

		if (readAttempts == SD_MAX_READ_ATTEMPTS)
			break;
		readAttempts++;
	}

	// if response token is 0xFE
	if (read == 0xFE) {
		// read 512 byte block
		for (uint16_t i = 0; i < 512; i++)
			buff[i] = SPI_transfer(0xFF);

		// read 16-bit CRC
		SPI_transfer(0xFF);
		SPI_transfer(0xFF);
	}

	if (!(read & 0xF0)) {
		//SD_printDataErrToken(read);
		return SD_READ_ERROR;
	} else if (read == 0xFF) {
		//USB_SerialPrint("Read Timeout\r\n");
		return SD_READ_ERROR;
	}
	return SD_READ_SUCCESS;
}

void SD_readMultipleSecStop() {
	SD_command(CMD12, CMD12_ARG, CMD12_CRC);

	while (SPI_transfer(0xFF) == 0x00)
		;

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);
}

uint8_t _writeMultipleBlock(uint32_t start_addr, uint8_t blockCnt,
		uint8_t *token) {
	uint8_t writeAttempts, read, res1;

	// set token to none
	*token = 0xFF;

	// assert chip select
	SPI_transfer(0xFF);
	CS_ENABLE();
	SPI_transfer(0xFF);

	// send CMD25
	SD_command(CMD25, start_addr, CMD25_CRC);

	// read response
	res1 = SD_readRes1();

	// if no error
	if (res1 == SD_READY) {
		//USB_SerialPrint("Enter text:\r\n");
		while (blockCnt--) {
			uint8_t write_buff[512] = { 0 };
			int i = 0;
			while (i < 512) {
			}

			// send start token
			SPI_transfer(0xFC);

			// write buffer to card
			for (uint16_t i = 0; i < SD_BLOCK_LEN; i++)
				SPI_transfer(write_buff[i]);
			// wait for a response (timeout = 250ms)
			writeAttempts = 0;

			while (writeAttempts != SD_MAX_WRITE_ATTEMPTS) {
				if ((read = SPI_transfer(0xFF)) != 0xFF)
					break;
				writeAttempts++;
			}
			// if data accepted
			if ((read & 0x1F) == 0x05) {
				// set token to data accepted
				*token = 0x05;

				// wait for write to finish (timeout = 250ms)
				writeAttempts = 0;
				while (SPI_transfer(0xFF) == 0x00) {
					if (writeAttempts == SD_MAX_WRITE_ATTEMPTS) {
						*token = 0x00;
						break;
					}
					writeAttempts++;
				}
				if (writeAttempts < SD_MAX_WRITE_ATTEMPTS) {
					//USB_SerialPrint("Block write success!\r\n");
				}
			}
		}
		// stop writing
		SPI_transfer(0xFD);
	}

	// deassert chip select
	SPI_transfer(0xFF);
	CS_DISABLE();
	SPI_transfer(0xFF);

	return res1;
}

uint8_t SD_writeMultipleBlock(uint32_t start_addr, uint8_t blockCnt) {

	uint8_t token, res1;
	res1 = _writeMultipleBlock(start_addr, blockCnt, &token);

	if (res1 == SD_READY) {
		if (token == 0x05)
			return SD_WRITE_SUCCESS;
		else if (token == 0xFF || token == 0x00)
			return SD_WRITE_ERROR;
	}

	//SD_printR1(res1);
	return SD_WRITE_ERROR;
}
