#include "stm32f4xx.h"                  // Device header
#include "Emm_V5.h"
#include "Delay.h"
#include "delay.h"
#include "OLED.h"
#include "math.h"

int  car_move_speed = 50;
int  a1 = 70;
int  a  = 230;
static float pingtui_distance = 0.0f;
static float current_pos = 0.0f; 
static float current_pos1 = 0.0f;  //升降的位置
double car_move_delay(float distance,uint16_t a_of_car,uint16_t speed_of_car,uint8_t direct);
double shengjiang_move_delay(float quanshu,uint16_t a,uint16_t speed);
void change_A(int a_in);
void change_SNA1(int a,int speed);

void car_move(int v_x,int v_y,int w)
{
	int speed[5];
	int dt=1;
	
	speed[1] = +v_x + v_y-w;
	speed[2] = -v_x + v_y+w;
	speed[3] = -v_x + v_y-w;
	speed[4] = +v_x + v_y+w;
	if(speed[1]>=0){
		Emm_V5_Vel_Control(2, 0, speed[1], a, false);//正向
	}
	else{
	    Emm_V5_Vel_Control(2, 1, -speed[1], a, false);   
	}
	delay_ms(dt);
	if(speed[2]>=0){
		Emm_V5_Vel_Control(1, 1, speed[2], a, false);//正向	
	}
	else{
	    Emm_V5_Vel_Control(1, 0, -speed[2], a, false);  
	}
	delay_ms(dt);
	if(speed[3]>=0){
		Emm_V5_Vel_Control(3, 0, speed[3], a, false);//正向	
	}
	else{
	    Emm_V5_Vel_Control(3, 1, -speed[3], a, false);  
	}
	delay_ms(dt);
	if(speed[4]>=0){
		Emm_V5_Vel_Control(4, 1, speed[4], a, false);//正向	
	}
	else{
	    Emm_V5_Vel_Control(4, 0, -speed[4], a, false); 
	}
	delay_ms(dt);
}

//输入为x轴需要移动的距离,单位是mm
void car_move_distance_x(float x)
{
	int speed[5];
	int dt=5;
	int v_x;
	int quanshu = ABS(x)*10.5; //实际的转动圈数
    v_x = car_move_speed*x/ABS(x);
	speed[1] = +v_x ;
	speed[2] = -v_x ;
	speed[3] = -v_x ;
	speed[4] = +v_x ;
	if(speed[1]>=0){
		Emm_V5_Pos_Control(2, 0, speed[1], a1, quanshu,false,true);//正向
	}
	else{
	    Emm_V5_Pos_Control(2, 1, -speed[1], a1, quanshu,false,true);   
	}
	delay_ms(dt);
	if(speed[2]>=0){
		Emm_V5_Pos_Control(1, 1, speed[2], a1,quanshu,false, true);//正向	
	}
	else{
	    Emm_V5_Pos_Control(1, 0, -speed[2], a1,quanshu,false,true);  
	}
	delay_ms(dt);
	if(speed[3]>=0){
		Emm_V5_Pos_Control(3, 0, speed[3], a1, quanshu,false,true);//正向	
	}
	else{
	    Emm_V5_Pos_Control(3, 1, -speed[3], a1, quanshu,false,true);  
	}
	delay_ms(dt);
	if(speed[4]>=0){
		Emm_V5_Pos_Control(4, 1, speed[4], a1, quanshu,false,true);//正向	
	}
	else{
	    Emm_V5_Pos_Control(4, 0, -speed[4], a1, quanshu,false,true); 
	}
	delay_ms(dt);
	Emm_V5_Synchronous_motion(0x00);
}

//输入为y轴需要移动的距离,单位是mm
void car_move_distance_y(float y)
{
	int speed[5];
	int dt=5;
	int v_y;
	int quanshu = ABS(y)*10.3; //实际的转动圈数
    v_y = car_move_speed*y/ABS(y);
	speed[1] = v_y;
	speed[2] = v_y;
	speed[3] = v_y;
	speed[4] = v_y;
	if(speed[1]>=0){
		Emm_V5_Pos_Control(2, 0, speed[1], a1, quanshu,false,true);//正向
	}
	else{
	    Emm_V5_Pos_Control(2, 1, -speed[1], a1, quanshu,false,true);   
	}
	delay_ms(dt);
	if(speed[2]>=0){
		Emm_V5_Pos_Control(1, 1, speed[2], a1,quanshu,false, true);//正向	
	}
	else{
	    Emm_V5_Pos_Control(1, 0, -speed[2], a1,quanshu,false, true);  
	}
	delay_ms(dt);
	if(speed[3]>=0){
		Emm_V5_Pos_Control(3, 0, speed[3], a1, quanshu,false,true);//正向	
	}
	else{
	    Emm_V5_Pos_Control(3, 1, -speed[3], a1, quanshu,false, true);  
	}
	delay_ms(dt);
	if(speed[4]>=0){
		Emm_V5_Pos_Control(4, 1, speed[4], a1, quanshu,false,true);//正向	
	}
	else{
	    Emm_V5_Pos_Control(4, 0, -speed[4], a1, quanshu,false,true); 
	}
	delay_ms(dt);
	Emm_V5_Synchronous_motion(0x00);
}

//当y>0时左移
void car_move1(float x,float y,int a_in,int car_speed)
{
	delay_ms1(10);
	change_SNA1(a_in,car_speed);
	car_move_distance_y(-y);
	car_move_delay(y,a_in,car_speed,1);
}

//当x>0时左移
void car_move2(float x,float y,int a_in,int car_speed)
{
	delay_ms1(10);
	change_SNA1(a_in,car_speed);
	car_move_distance_x(-x);
	car_move_delay(x,a_in,car_speed,0);
}

//当y>0时左移//仅执行任务，不计时
void car_move3(float x,float y,int a_in,int car_speed)
{
	delay_ms1(10);
	change_SNA1(a_in,car_speed);
	car_move_distance_y(-y);
}

//当x>0时左移//仅执行任务，不计时
void car_move4(float x,float y,int a_in,int car_speed)
{
	delay_ms1(10);
	change_SNA1(a_in,car_speed);
	car_move_distance_x(-x);
}


double shengjiang_control(int target_pos, uint16_t speed, uint8_t accel)
{   
	if(target_pos >=135.0f){
		target_pos =135.0f;
	}
	if (target_pos <0.0f){
        OLED_ShowString(1, 1, "error:OUT");
        while(1){}
    }
	delay_ms1(10);//放置堵塞can通信
	   // 计算需要移动的距离和方向
	int move_distance = target_pos - current_pos1;
    uint8_t dir = (move_distance > 0) ? 0 : 1;
    
    // 计算需要的脉冲数（取绝对值）
    uint32_t pulses = (uint32_t)(ABS(move_distance) * 80);

    // 执行运动控制
	if(target_pos != current_pos1){
		Emm_V5_Pos_Control(5, dir, speed, accel, pulses, false, false);
	}
    shengjiang_move_delay(2*pulses, accel, speed);

    // 更新当前位置
    current_pos1 = target_pos;
	return shengjiang_move_delay(1.0f*pulses, accel, speed);
}


/*
	dir = 0时，向前运动,距离不能超过60mm
	dir = 1时，向后运动,距离不能超过120mm
	25.465脉冲大概对应1mm
	以最高处为基准
	滑道最长
*/
double pingtui_control(float target_pos, uint16_t speed, uint8_t accel)
{   
	if (target_pos > 65.0f || target_pos < -122.0f){
        OLED_ShowString(1, 1, "error:OUT");
        while(1){}
    }
	delay_ms1(10);//放置堵塞can通信
	   // 计算需要移动的距离和方向
    float move_distance = target_pos - current_pos;
    uint8_t dir = (move_distance > 0) ? 0 : 1; // 0=向前(上升), 1=向后(下降)
    
    // 计算需要的脉冲数（取绝对值）
    uint32_t pulses = (uint32_t)(fabs(move_distance) * 25.465f);

    // 执行运动控制
	if(target_pos != current_pos){
		Emm_V5_Pos_Control(6, dir, speed, accel, pulses, false, false);
	}
    shengjiang_move_delay(2*pulses, accel, speed);

    // 更新当前位置
    current_pos = target_pos;
	return shengjiang_move_delay(1.0f*pulses, accel, speed);
}

void pingtui_reset(void)
{   
	pingtui_control(0.0f,1000,200);
}

/*
	//0代表x轴
	//1代表y轴
	根据输入的速度和加速度和路程
	delay一定的时间，用来空过两个连续的位置坐标指令
	放置前后指令的覆盖
*/
double car_move_delay(float distance,uint16_t a_of_car,uint16_t speed_of_car,uint8_t direct)
{	
	if(a_of_car == 0){a_of_car = 256;}
	double quanshu = 0;
	if(direct == 0){
		quanshu = ABS(distance) * 10.5;
	}else{
		quanshu = ABS(distance) * 10.3;
	}
	double quanshu_fenjie = 320 * pow(speed_of_car,2)/(a_of_car * 6);
	double time_fenjie = 2000*speed_of_car/a_of_car;
	double time = 0;
	if(quanshu_fenjie >= quanshu){
		time = time_fenjie*pow((quanshu/quanshu_fenjie),0.5);
	}else{
		time = 1000 * (quanshu-quanshu_fenjie)/(320*speed_of_car/6) + time_fenjie;
	}
	if(ABS(distance) <= 2){time = 200;}
	delay_ms1((int)time);
	return time;
}


/*
	根据输入的速度和加速度和路程
	delay一定的时间，用来空过两个连续的位置坐标指令
	放置前后指令的覆盖
*/
double shengjiang_move_delay(float quanshu,uint16_t a,uint16_t speed)
{	
	if(a == 0){a = 256;}
	double quanshu_fenjie = 320.0f * pow(speed,2)/(a * 6.0f);
	double time_fenjie = 2000.0f*speed/a;
	double time = 0;
	if(quanshu_fenjie >= quanshu){
		time = time_fenjie*pow((quanshu/quanshu_fenjie),0.5);
	}else{
		time = 1000 * (quanshu-quanshu_fenjie)/(320*speed/6) + time_fenjie;
	}
	time = time * 0.5f;
	if(ABS(time) <= 200){time = 200;}//最少也要等待200ms
//	delay_ms1((int)time);
	return time;
}


/***************************
	改变PID_move的速度大小
	输入为加速度的大小
****************************/
void change_A(int a_in)
{
	a = a_in;
}


/******************************
	改变car_move的加速度和速度大小
	前面输入加速度，后面输入速度
*******************************/
void change_SNA1(int a,int speed)
{
	a1 = a;
	car_move_speed = speed;
}
