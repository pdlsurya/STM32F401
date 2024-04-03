/*
 * Serial.h
 *
 *  Created on: Jan 25, 2024
 *      Author: pdlsurya
 */

#ifndef SERIAL_SERIAL_H_
#define SERIAL_SERIAL_H_



bool Serial_Available();
char* Serial_ReadLine();
uint8_t Serial_ReadByte();
void Serial_Init();
void Serial_Write(uint8_t *tx_data, uint16_t len);
void Serial_Flush();

#endif /* SERIAL_SERIAL_H_ */
