#include "usart5.h"
#include "hmi.h"
#include "LED.h"
#include "Serial.h"
#include "swith.h"
#include "LED.h"
#include "delay.h"

static u8 RxCounter1=0;
static u8 RxBuffer1[10];
static u8 RxState1 = 0;	
static u8 RxFlag1 = 0;
u16 USART5_RX_STA=0;
int i;
char test[10];

void uart5_init(u32 bound)
{  	 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	 //根据芯片情况选择端口
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE); //使能UART5时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);	 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 

	//USART NVIC配置
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);      
	
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE); 
	USART_Cmd(UART5, ENABLE);                     
}

/**
  ******************************************************************************
  * @brief  UART5中断服务程序（接收帧格式数据处理）
  * @note   接收帧格式为：0x04（起始字节1）+ 0x05（起始字节2）+ 数据（6字节）+ 0x06（结束字节）
  *         共9个字节，其中第1、2字节为帧头，第9字节为帧尾。
  *         当接收完整帧后，设置RxFlag1为1，用户可在主程序中读取RxBuffer1处理数据。
  * @param  无
  * @retval 无
  ******************************************************************************
  */
void UART5_IRQHandler(void)                 
{
    // 判断是否接收到数据（接收缓冲非空中断标志位是否置位）
    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    {
        u8 r;
        // 读取接收到的数据
        r = USART_ReceiveData(UART5);
        // 如果还未接收完成
        if((USART5_RX_STA & 0x8000) == 0)
        {
            // 第一步：等待帧头0x04
            if(RxState1 == 0 && r == 0x04)
            {
                RxState1 = 1;                     // 状态转移
                RxBuffer1[RxCounter1++] = r;      // 存储数据
            }
            // 第二步：等待帧头0x05
            else if(RxState1 == 1 && r == 0x05)
            {
                RxState1 = 2;
                RxBuffer1[RxCounter1++] = r;
            }
            // 第三步：接收中间数据（6字节）
            else if(RxState1 == 2)
            {
                RxBuffer1[RxCounter1++] = r;
                if(RxCounter1 >= 9)
                {
                    RxState1 = 3;     // 转入结束标志判断状态
                    RxFlag1 = 1;      // 标记接收完成
                }
            }
            // 第四步：检查最后一个字节是否为0x06（帧尾）
            else if(RxState1 == 3)
            {
                if(RxBuffer1[RxCounter1 - 1] == 0x06)
                {
                    // 成功接收一帧数据，关闭中断处理
                    USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
                    if(RxFlag1)
                    {
                        // 可以在这里处理接收到的数据，如触发事件、设置标志位等
						UART5_SendArray(RxBuffer1,6);
                    }

                    // 清除状态，准备接收下一帧
                    RxFlag1 = 0;
                    RxCounter1 = 0;
                    RxState1 = 0;
                    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
                }
            }
            // 若接收中出现错误字节或顺序错误，重置状态
            else
            {
                RxState1 = 0;
                RxCounter1 = 0;
                for(i = 0; i < 9; i++)
                    RxBuffer1[i] = 0x00;
            }
        }
        // 接收异常（如帧已标记完成但仍有数据进来），重置
        else
        {
            RxState1 = 0;
            RxCounter1 = 0;
            for(i = 0; i < 9; i++)
                RxBuffer1[i] = 0x00;
        }
    }
}


void UART5_SendString(char *str) 
{
	while (*str != '\0') {
		while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
		USART_SendData(UART5, (uint8_t)*str);
		str++;
	}
}

void UART5_SendArray(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
    {
        // 等待发送缓冲区为空
        while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
        // 发送一个字节
        USART_SendData(UART5, data[i]);
    }
    // 等待所有数据发送完成
    while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET);
}

void find_congji()
{
	UART5_SendString("AT+NAME=STM32_Master\r\n");//设置蓝牙模块名称
	delay_ms(1000);
	// 搜索设备 
	UART5_SendString("AT+PSWD=1234\r\n");//两个要对应的pin值
	delay_ms(1000);
	UART5_SendString("AT+ROLE=1\r\n");//设置为主机模式（主动连接别人，不能被主动连接）
	delay_ms(1000);	
	UART5_SendString("AT+BIND=98:DA:20:07:6D:F5\r\n");//写上从机的蓝牙地址
}


void fasong_task_code(void)
{
	Serial_SendByte(0x72);
	uint8_t task_code[7];
//	task_code[0] = RxBuffer1[2];
//	task_code[1] = RxBuffer1[3];
//	task_code[2] = RxBuffer1[4];
//	task_code[3] = 0x2B;
//	task_code[4] = RxBuffer1[5];
//	task_code[5] = RxBuffer1[6];
//	task_code[6] = RxBuffer1[7];
	delay_ms1(500);
//	while(swith_out() == 1){		
//		LED_Toggle2();
//		delay_ms1(50);
//	}
	task_code[0] = 0x31;
	task_code[1] = 0x32;
	task_code[2] = 0x33;
	task_code[3] = 0x2B;
	task_code[4] = 0x33;
	task_code[5] = 0x32;
	task_code[6] = 0x31;
	Serial_SendArray(task_code,7);
}
