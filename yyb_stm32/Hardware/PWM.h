#ifndef __PWM_H
#define __PWM_H

// 定义伺服机构参数结构体
typedef struct {
    float start_angle;    	//从什么角度开始
    float target_angle;	  	//目标是旋转多少角度
    float current_angle;  	//当前的角度是多少
    int moving_flag;    // 插值更新标志，1: 正在运动；0: 空闲
    int t;            // 时间变量，用于插值计算
	int time;		  //整个插值运动所需的时间
    void (*SetServoAngle)(float angle);  // 设置当前角度的函数指针
} Servo_t;

void PWM_Init(void);
void PWM_SetCompare2(uint16_t Compare);
void PWM_SetCompare3(uint16_t Compare);
void PWM_SetCompare4(uint16_t Compare);


void set_zhuashou_he(void);
void set_zhuashou_kai(void);
void set_yuantai_Angle(float target, float time);
void set_zhuashou_Angle(float target_abs, float speed);
void set_wukuaipingtai_Angle(float target_abs, float speed);
void set_wukuaipingtai_weizhi(int pos);

int move_all(float target_pos, uint16_t speed, uint8_t accel,float target_pos1, uint16_t speed1, uint8_t accel1,float target, float speed_of_yuntai);
int move_all1(float target_pos, uint16_t speed, uint8_t accel,float target_pos1, uint16_t speed1, uint8_t accel1,float target, float speed_of_yuntai);
#endif
