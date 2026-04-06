#ifndef __USART5_H
#define __USART5_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

void uart5_init(u32 bound);
void UART5_SendString(char *str);
void UART5_SendArray(uint8_t *data, uint16_t len);
void fasong_task_code(void);

#endif
