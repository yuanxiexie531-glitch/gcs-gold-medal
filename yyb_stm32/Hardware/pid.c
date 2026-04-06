#include "stm32f4xx.h"                  // Device header
#include "yyb_move.h"
#include "OLED.h"
#include "delay.h"
#include <math.h>
#include "swith.h"
#include "wit_c_sdk.h"
#include "PWM.h"
#include <stdio.h>
#include <math.h>
#include "LED.h"
#include "imu.h"

#define pi  3.1415926/180

float feedbackValue = 0; //这里获取到被控对象的反馈值
float feedbackValue_pre = 0;
float targetValue = 0; //这里获取到目标值
float fAcc[3], fGyro[3], fAngle[3];
int vx,vy,omiga,flag;
uint32_t delay_time = 0;
uint32_t real_time = 0;
uint8_t dianji_move_flag = 0;
uint8_t PID_control_flag = 0;
uint8_t circle = 0;
float same_time = 0;   //测量PID计算中角度坐标的信息是否重复，也即PID计算的内容是否有效

typedef struct
{
   	float kp, ki, kd; //三个系数
    float error, lastError; //误差、上次误差
    float integral, maxIntegral; //积分、积分限幅
    float output, maxOutput; //输出、输出限幅
}PID;
PID mypid = {0}; //创建一个PID结构体变量

void Time_Init();
void Clear_Init();

void PID_Init(PID *pid, float p, float i, float d, float maxI, float maxOut)
{
    pid->kp = p;
    pid->ki = i;
    pid->kd = d;
    pid->maxIntegral = maxI;
    pid->maxOutput = maxOut;
}
 

/**
  * @brief  进行一次pid计算
  *         参数为(pid结构体,目标值,反馈值)
  *                    
  * @param  pid
  * @param  reference: target_of_value 
  * @param  feedback 
  * @retval None
  */
void PID_Calc(PID *pid, float reference, float feedback)
{
 	//更新数据
    pid->lastError = pid->error; //将旧error存起来
    //计算新误差
    if(reference - feedback < -180){
		pid->error = reference - feedback + 360; //计算新error
	}else if(reference - feedback > 180){
		pid->error = reference - feedback - 360;
	}else{
		pid->error = reference - feedback;
	}	
    //计算微分
    float dout = (pid->error - pid->lastError) * pid->kd;
    //计算比例
    float pout = pid->error * pid->kp;
    //计算积分
    pid->integral += pid->error * pid->ki;
    //积分限幅
    if(pid->integral > pid->maxIntegral) pid->integral = pid->maxIntegral;
    else if(pid->integral < -pid->maxIntegral) pid->integral = -pid->maxIntegral;
    //计算输出
    pid->output = pout+dout + pid->integral;
    //输出限幅
    if(pid->output > pid->maxOutput) pid->output =   pid->maxOutput;
    else if(pid->output < -pid->maxOutput) pid->output = -pid->maxOutput;
}
 
void PIDInit(void)
{
	Time_Init();
    PID_Init(&mypid, 1.5, 0.001, 2, 7, 180); //初始化PID参数
}

void PID_DIL(int v_x,int v_y,int w,uint32_t time,float target)
{   
	vx = v_x;
	vy = v_y;
	omiga = w;
	delay_time = time;
	real_time = 0;
	targetValue = target;
}

uint8_t PID_move(int v_x,int v_y,int w,uint32_t time,float target,int pid_choose)
{   
	TIM_Cmd(TIM4, ENABLE); // 使能TIM4，定时器开始运行
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	flag = 0; //运动标志位归零
	change_A(230);//设置PID move 的加速度为 230rpm
	if(pid_choose == 0){
		mypid.kp = 2;
		mypid.ki = 0.00;
		mypid.kd = 0.8;
	}else if(pid_choose == 1){
		mypid.kp = 3.3;
		mypid.ki = 0.00;
		mypid.kd = 1.8;
		mypid.maxOutput = 30;
	}else if(pid_choose == 2){
		mypid.kp = 5;
		mypid.ki = 0.00;
		mypid.kd = 5;
	}else if(pid_choose == 3){
		mypid.kp = 2;
		mypid.ki = 0.05;
		mypid.kd = 0.4;
	}else if(pid_choose == 4){
		mypid.kp = 2;
		mypid.ki = 0.07;
		mypid.kd = 0.4;
	}
	if(target > 180){
		target = target -360;
	}else if(target < -180){
		target = target + 360;
	}
	PID_DIL(v_x,v_y,w,time,target);
	while(flag == 0){};flag = 0;
	mypid.maxOutput = 230;
	TIM_Cmd(TIM4, DISABLE); 
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	delay_ms1(100);//等待定时器完全关闭
	return 1;
}

void PID_move1(int v_x,int v_y,int w,uint32_t time,int target,int pid_choose)
{   
	TIM_Cmd(TIM4, ENABLE); // 使能TIM4，定时器开始运行
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	flag = 0; //运动标志位归零
	if(pid_choose == 0){
		mypid.kp = 1.5;
		mypid.ki = 0.1;
		mypid.kd = 2;
	}else{
		mypid.kp = 1.5;
		mypid.ki = 0.001;
		mypid.kd = 2.5;
	}
	PID_DIL(v_x,v_y,w,time,target);
}

//定时读取陀螺仪角度数据进行PID计算
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{   
		if(real_time <= delay_time && dianji_move_flag){
			imu_scan(fAcc, fGyro, fAngle);
			feedbackValue = fAngle[2]; //反馈值
			PID_Calc(&mypid, targetValue, feedbackValue); //进行PID计算，结果在output成员变量
			if(feedbackValue == feedbackValue_pre){
				same_time++;
			}
			feedbackValue_pre = fAngle[2];
			car_move(vx,vy,mypid.output);
			flag = 0;
		}else if(PID_control_flag == 0){
			car_move(0,0,0);
			flag =1;
		}
		real_time+=50; //每次加50ms
		circle++;
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}



/*
	输入横纵移动的坐标，移动移动加速度和速度
	单位是mm
	一般设置加速度为70rpm，速度为50rpm
*/
void car_move_distance(float x,float y,int a_in,int speed_in)
{   
	delay_ms1(10);//防止堵塞can通信
	change_SNA1(a_in,speed_in);
	car_move_distance_y(x);
	car_move_delay(x,a_in,speed_in,1);
	
	delay_ms1(10);//防止堵塞can通信
	change_SNA1(a_in,speed_in);
	car_move_distance_x(y);
	car_move_delay(y,a_in,speed_in,0);
}




//单位是mm
void PID_car_move_distance(float x,float y,float angel,float speed)
{	
	double fenliang = speed/pow((x*x+y*y),0.5);
	double time = 0;
	time = pow((x*x+y*y),0.5)*4;
	change_A(0);
	PID_move(y*fenliang,x*fenliang,0,time,angel,1);
}



void Time_Init()
{
    /*开启时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); // 开启TIM4的时钟

    /*配置时钟源*/
    TIM_InternalClockConfig(TIM4);

    /*时基单元初始化*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure; // 定义结构体变量
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式
    TIM_TimeBaseInitStructure.TIM_Period = 10000/2 - 1; // 计数周期，即ARR的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 840 - 1; // 预分频器，即PSC的值
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0; // 重复计数器
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure); // 配置TIM4的时基单元

    /*中断输出配置*/
    TIM_ClearFlag(TIM4, TIM_FLAG_Update); // 清除定时器更新标志位
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); // 开启TIM4的更新中断

    /*NVIC中断分组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 配置NVIC为分组2

    /*NVIC配置*/
    NVIC_InitTypeDef NVIC_InitStructure; // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; // 选择配置NVIC的TIM4线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能NVIC线路
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 指定优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; // 指定响应优先级
    NVIC_Init(&NVIC_InitStructure); // 配置NVIC外设
}
