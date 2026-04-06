#include "stm32f4xx.h"                  // Device header
#include "yyb_move.h"
#include "OLED.h"
#include "delay.h"
#include <math.h>
#include "swith.h"
#include "wit_c_sdk.h"
#include <stdio.h>
#include <math.h>

#define pi  3.1415926/180

float feedbackValue = 0; //这里获取到被控对象的反馈值
float targetValue = 0; //这里获取到目标值
float fAcc[3], fGyro[3], fAngle[3];
int vx,vy,omiga,flag;
float test1 = 0;
uint32_t delay_time = 0;
uint32_t real_time = 0;
uint8_t dianji_move_flag = 0;
uint8_t PID_control_flag = 0;

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
    PID_Init(&mypid, 1.5, 0.001, 2, 2, 60); //初始化PID参数
}

void PID_DIL(int v_x,int v_y,int w,uint32_t time,int target)
{   
	vx = v_x;
	vy = v_y;
	omiga = w;
	delay_time = time;
	real_time = 0;
	targetValue = target;
}

uint8_t PID_move(int v_x,int v_y,int w,uint32_t time,int target,int pid_choose)
{   
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
	while(flag == 0){};flag = 0;
	return 1;
}

//定时0.2s读取树莓派数据,进行PID计算,并执行相应的运动
void circle_move(void)
{
	     
}

//单位是mm*10-1
void car_move_distance(float x,float y)
{    
	PID_control_flag = 1;   //防止定时器影响car_move函数的调用
	int v = 15;
	float cos_theta = 0;
	float sin_theta = 0;
	float distance = 0;
	distance = pow((pow(x,2)+pow(y,2)),0.5);
	cos_theta = x/distance;
	sin_theta = y/distance;
//	PID_move(v*sin_theta,-v*cos_theta,0,1.3793*distance,0,1);
	car_move(v*sin_theta,-v*cos_theta,0);
	delay_ms(1.5715*distance);
	car_move(0,0,0);
	PID_control_flag = 0;
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
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1; // 计数周期，即ARR的值
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
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // 指定响应优先级
    NVIC_Init(&NVIC_InitStructure); // 配置NVIC外设

    /*TIM使能*/
    TIM_Cmd(TIM4, ENABLE); // 使能TIM4，定时器开始运行
}
 

