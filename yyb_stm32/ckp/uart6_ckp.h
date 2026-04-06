#ifndef __UART6_CKP_H
#define __UART6_CKP_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

void uart6_init(uint32_t bound);
void HMISends(char *buf1);		  
void HMISendb(uint8_t k);	

#endif
