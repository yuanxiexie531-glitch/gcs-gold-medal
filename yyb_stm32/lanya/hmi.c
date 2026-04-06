#include "sys.h"
#include "usart.h"	
#include "stm32f4xx.h"
#include "hmi.h"

//字符串发送函数
void HMISends(char *buf1)		  
{
	u8 i=0;
	while(1)
	{
		if(buf1[i] != 0)
	 	{
			USART_SendData(UART5,buf1[i]);  //发送一个字节
			while(USART_GetFlagStatus(UART5,USART_FLAG_TXE)==RESET){};//等待发送结束
		 	i++;
		}
		else
		{
			return ;
		}
	}
}

//字节发送函数
void HMISendb(u8 k)		         
{		 
	u8 i;
	 for(i=0; i<3; i++)
	 {
			if(k != 0)
			{
				USART_SendData(UART5,k);  //发送一个字节
				while(USART_GetFlagStatus(UART5,USART_FLAG_TXE)==RESET){};//等待发送结束
			}
			else
			{
				return ;
			}
	 } 
} 