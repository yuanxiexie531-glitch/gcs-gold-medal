#ifndef __SEVRVO_H
#define __SEVRVO_H

void Servo_Init(void);

//机械抓手
//不能再低于18度
void Servo_SetAngle2_zhuashou(float Angle);

//放置物块平台
//物块台平行为191//加9.5放物块1
void Servo_SetAngle3_wukuaipingtai(float Angle);

//云台
//7V电机
//当角度为75时，升降台平行
void Servo_SetAngle4_yuntai(float Angle);

#endif
