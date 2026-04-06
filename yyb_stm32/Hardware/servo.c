#include "stm32f4xx.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：舵机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Servo_Init(void)
{
	PWM_Init();									//初始化舵机的底层PWM
}

//机械抓手
//不能再低于18度
//23度抓物块
//100度为展开的状态
void Servo_SetAngle2_zhuashou(float Angle)
{
	PWM_SetCompare2(Angle / 270 * 2000 + 500);
}

//放置物块平台
//物块台平行为191//加10.5放物块1
void Servo_SetAngle3_wukuaipingtai(float Angle)
{
	PWM_SetCompare3(Angle / 270 * 2000 + 500);
}

//云台
//7V电机
//当角度为75时，升降台平行
//75+90-15的时候可以放置第一个物块
//
void Servo_SetAngle4_yuntai(float Angle)
{
	PWM_SetCompare4(Angle / 360 * 2000 + 500);
}

