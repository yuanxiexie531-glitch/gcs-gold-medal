#ifndef __YYB_MOVE_H
#define __YYB_MOVE_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "Emm_V5.h"

void change_A(int a_in);
void change_SNA1(int a,int speed);
void car_move(int v_x,int v_y,int w);
double shengjiang_control(int target_pos, uint16_t speed, uint8_t accel);
double pingtui_control(float target_pos, uint16_t speed, uint8_t accel);
void pingtui_reset(void);
void car_move_distance_x(float x);
void car_move_distance_y(float y);
void car_move1(float x,float y,int a_in,int car_speed);
void car_move2(float x,float y,int a_in,int car_speed);
void car_move3(float x,float y,int a_in,int car_speed);
void car_move4(float x,float y,int a_in,int car_speed);
double car_move_delay(float distance,uint16_t a_of_car,uint16_t speed_of_car,uint8_t direct);

#endif