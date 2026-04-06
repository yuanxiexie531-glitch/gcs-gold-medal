#include "stm32f4xx.h"                  // Device header

void Timer_Init(void)
{
    /*开启时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);            // 开启 TIM3 的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);           // 开启 GPIOA 的时钟

    /*配置 PA6 为复用功能*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;                       // 选择 PA6
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                    // 设置为复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;              // 设置速度
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                  // 设置为推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                // 不使能上下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);                           // 初始化 PA6

    /* 将 PA6 复用为 TIM3_CH1 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);

    /*时基单元初始化*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;              // 定义结构体变量
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    // 时钟分频，选择不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 2000 - 1;                // 计数周期，即 ARR 的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 84 - 1;              // 预分频器，即 PSC 的值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);             // 配置 TIM3 的时基单元

    /* PWM 输出配置 */
    TIM_OCInitTypeDef TIM_OCInitStructure;                           // 定义结构体变量
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               // PWM 模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   // 使能输出
    TIM_OCInitStructure.TIM_Pulse = 0;                             // 设置占空比
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       // 输出极性
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);                         // 初始化 TIM3_CH1

    /*TIM使能*/
    TIM_Cmd(TIM3, ENABLE);                                           // 使能 TIM3，定时器开始运行
	TIM_SetCompare1(TIM3, 0);		//设置CCR1的值，改呼吸灯的亮度
}

