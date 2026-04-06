#ifndef __PID_H
#define __PID_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h"
#include "LED.h"
#include "Emm_V5.h"

typedef struct
{
   	float kp, ki, kd; //三个系数
    float error, lastError; //误差、上次误差
    float integral, maxIntegral; //积分、积分限幅
    float output, maxOutput; //输出、输出限幅
}PID;

extern int flag;
extern PID mypid;
extern uint8_t dianji_move_flag;
extern uint8_t PID_control_flag;
extern float fAcc[3], fGyro[3], fAngle[3];
void PID_DIL(int v_x,int v_y,int w,uint32_t time,int target);

void PIDInit(void);
uint8_t PID_move(int v_x,int v_y,int w,uint32_t time,float target,int pid_choose);
//单位是mm
void car_move_distance(float x,float y,int a_in,int speed_in);
void PID_car_move_distance(float x,float y,float angel,float speed);

#endif