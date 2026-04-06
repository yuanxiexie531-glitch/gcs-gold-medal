#include "stm32f4xx.h"
#include "Serial.h"

// 1. 按键中断回调函数
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)  // 检查中断标志位
    {
		Serial_SendByte(0x72);
        EXTI_ClearITPendingBit(EXTI_Line0);   // 清除中断标志位
    }
}

// 2. GPIO & 中断初始化
void Key_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 2.1 使能 GPIOA 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    // 2.2 使能 SYSCFG 时钟（EXTI 需要）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 2.3 配置 PA0 为输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;  // 下拉，等待按键按下（高电平）
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 2.4 连接 EXTI0 到 PA0
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    // 2.5 配置 EXTI0 为上升沿触发
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  // 上升沿触发（按下检测）
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 2.6 配置 NVIC（中断向量）
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;  // EXTI0 对应中断号
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

