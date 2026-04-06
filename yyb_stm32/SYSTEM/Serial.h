#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>

extern uint8_t Serial_TxPacket[];
extern uint8_t Serial_RxPacket[];
extern int code1[3];
extern int code2[3];
extern uint8_t Serial_RxFlag;	


void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void set_Rxstate(uint8_t i);

void Serial_SendPacket(void);
uint8_t Serial_GetRxFlag(void);

#endif
