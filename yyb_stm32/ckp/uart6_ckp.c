#include "stm32f4xx.h"                  // Device header
#include "usart.h"
#include "delay.h"

/**************************************************************************
Function: Serial port 6 initialization
Input   : none
Output  : none
函数功能：串口6初始化
入口参数：无
返回  值：无
**************************************************************************/
void uart6_init(uint32_t bound)
{  	 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 使能GPIOC时钟和USART6时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	  // 使能GPIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); // 使能USART6时钟
	
	// 配置GPIO复用功能
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);  // PC6 -> USART6_TX
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);  // PC7 -> USART6_RX
	
	// GPIO初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;              // 复用模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;            // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         // 高速50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;              // 上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);  		          // 初始化
	
	// 配置USART6中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	
	
	// 初始化USART6
	USART_InitStructure.USART_BaudRate = bound;               // 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;    // 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;       // 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
	USART_Init(USART6, &USART_InitStructure);                 // 初始化USART6
	
	// 使能USART6接收中断
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);            // 开启接收中断
	USART_Cmd(USART6, ENABLE);                                // 使能USART6
}

//字符串发送函数
void HMISends(char *buf1)		  
{
	u8 i=0;
	while(1)
	{
		if(buf1[i] != 0)
	 	{
			USART_SendData(USART6,buf1[i]);  //发送一个字节
			while(USART_GetFlagStatus(USART6,USART_FLAG_TXE)==RESET){};//等待发送结束
		 	i++;
		}
		else
		{
			return ;
		}
	}
}

//字节发送函数
void HMISendb(uint8_t k)		         
{		 
	 uint8_t i;
	 for(i=0; i<3; i++)
	 {
			if(k != 0)
			{
				USART_SendData(USART6,k);  //发送一个字节
				while(USART_GetFlagStatus(USART6,USART_FLAG_TXE)==RESET){};//等待发送结束
			}
			else
			{
				return ;
			}
	 } 
} 

