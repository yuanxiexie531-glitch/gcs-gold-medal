#include "stm32f4xx.h"                  // Device header

void Timer5_Init(void)
{
    /*开启时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);          

    /*时基单元初始化*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;              // 定义结构体变量
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    // 时钟分频，选择不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 50000 - 1;                // 计数周期，即 ARR 的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 840 - 1;              // 预分频器，即 PSC 的值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);             // 配置 TIM5 的时基单元
	
	  /*中断输出配置*/
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);                            // 清除定时器更新标志位
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);                      // 开启 TIM5 的更新中断
    
    /*NVIC中断分组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                 // 配置 NVIC 为分组2
    
    /*NVIC配置*/
    NVIC_InitTypeDef NVIC_InitStructure;                             // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;                 // 选择配置 NVIC 的 TIM5 线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 // 指定 NVIC 线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;       // 指定 NVIC 线路的抢占优先级为 0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;              // 指定 NVIC 线路的响应优先级为 0
    NVIC_Init(&NVIC_InitStructure);                                            
}
