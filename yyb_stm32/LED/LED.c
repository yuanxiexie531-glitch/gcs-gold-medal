#include "stm32f4xx.h"

void LED_Toggle(void);

void LED_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       // 设置为输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 50MHz的速度
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;         
 	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //配置PA8
	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       // 设置为输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 50MHz的速度
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;         
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	LED_Toggle();
}


void LED_Toggle0(void)
{
	GPIO_ToggleBits(GPIOF,GPIO_Pin_9);
}

void LED_Toggle1(void)
{
	GPIO_ToggleBits(GPIOF,GPIO_Pin_10);
}

void LED_Toggle(void)
{
	GPIO_ToggleBits(GPIOF,GPIO_Pin_9);
	GPIO_ToggleBits(GPIOF,GPIO_Pin_10);
}

void LED_Toggle2(void)
{
	GPIO_ToggleBits(GPIOA,GPIO_Pin_8);
}
