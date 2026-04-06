#include "stm32f4xx.h"                  // Device header
#include "yyb_move.h"
#include "math.h"
#include "servo.h"
#include "delay.h"

typedef struct {
    float start_angle;    	//从什么角度开始
    float target_angle;	  	//目标是旋转多少角度
    float current_angle;  	//当前的角度是多少
    int moving_flag;    // 插值更新标志，1: 正在运动；0: 空闲
    float t;            // 时间变量，用于插值计算
	float time;		  //整个插值运动所需的时间
    void (*SetServoAngle)(float angle);  // 设置当前角度的函数指针
} Servo_t;

Servo_t yuantai = {60.84f, 0.0f, 60.84f, 0, 0, 0, Servo_SetAngle4_yuntai};
Servo_t zhuashou = {2.7f, 2.7f, 2.7f, 0, 0, 0, Servo_SetAngle2_zhuashou};
Servo_t wukuaipingtai = {20.0f, 20.0f, 20.0f, 0, 0, 0,Servo_SetAngle3_wukuaipingtai};

void PWM_Init(void)
{
	/* 开启时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			// 开启TIM2的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);			// 开启GPIOA的时钟
	
	/* GPIO初始化 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;					// 复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3; // 配置PA1, PA2, PA3
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;					// 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;				// 不使用上下拉电阻
	GPIO_Init(GPIOA, &GPIO_InitStructure);							// 初始化GPIOA
	
	// 配置GPIO复用功能为TIM2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);  // PA1 -> TIM2_CH2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);  // PA2 -> TIM2_CH3
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);  // PA3 -> TIM2_CH4

	/* 配置时钟源 */
	TIM_InternalClockConfig(TIM2);		// 选择TIM2为内部时钟
	
	/* 时基单元初始化 */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分频，选择不分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;				// 计数周期，即ARR的值，控制PWM周期
	TIM_TimeBaseInitStructure.TIM_Prescaler = 84 - 1;				// 预分频器，控制PWM频率
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             // 初始化TIM2时基单元
	
	/* 输出比较初始化 */ 
	TIM_OCInitTypeDef TIM_OCInitStructure;							
	TIM_OCStructInit(&TIM_OCInitStructure);                         
	
	// 配置通道
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               // 输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       // 输出极性，选择为高
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   // 输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;							    // 设置占空比
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);                        // 初始化TIM2的输出比较通道2
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);                        // 初始化TIM2的输出比较通道3
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);                        // 初始化TIM2的输出比较通道4
    
	NVIC_InitTypeDef NVIC_InitStructure;
    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	/* TIM使能 */
	TIM_Cmd(TIM2, ENABLE);			// 使能TIM2，定时器开始运行
	TIM_SetCompare2(TIM2, 520);		// 抓手
	TIM_SetCompare3(TIM2, 648);		// 物块平台
	TIM_SetCompare4(TIM2, 838);		// 云台
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void PWM_SetCompare2(uint16_t Compare)
{
	TIM_SetCompare2(TIM2, Compare);		// 设置CCR2的值
}

void PWM_SetCompare3(uint16_t Compare)
{
	TIM_SetCompare3(TIM2, Compare);		// 设置CCR3的值
}

void PWM_SetCompare4(uint16_t Compare)
{
	TIM_SetCompare4(TIM2, Compare);		// 设置CCR4的值
}

// 公共函数：设置目标角度（相对变化量）********
void set_servo_angle(Servo_t *servo, float target)
{
    servo->target_angle = target;
    servo->moving_flag = 1;
}

// 插值函数
float interpolate(float start, float target, float t) {
    if (t < 200) {
        return (25.0f / 8.0f) * target * pow(t / 1000.0f, 2) + start;
    } else if (t < 800) {
        return (5.0f / 4.0f) * target * (t / 1000.0f) - target / 8.0f + start;
    } else if (t < 1000) {
        return (-25.0f / 8.0f) * target * pow(t / 1000.0f, 2) + (25.0f / 4.0f) * target * (t / 1000.0f) - (17.0f / 8.0f) * target + start;
    } else {
        return start + target;
    }
}

// 插值更新函数
void servo_update(Servo_t *servo) {
	double tau,s;
    if (servo->moving_flag == 1) {
		servo->t +=20;
		tau = servo->t / servo->time;
			if (tau <= 0.0) {
				servo->current_angle = servo->start_angle;
			} else if (tau >= 1.0) {
				servo->current_angle = servo->target_angle;
				servo->start_angle = servo->target_angle;
				servo->t = 0;
				servo->moving_flag = 0;
			} else {
				s = 6 * tau * tau * tau * tau * tau
				  - 15 * tau * tau * tau * tau
				  + 10 * tau * tau * tau;
				servo->current_angle = servo->start_angle + (servo->target_angle - servo->start_angle) * s;
			}	
        servo->SetServoAngle(servo->current_angle);
    }
}


//278度为抓物块的位置
//???度为放置到物块平台的位置
//77.4度为初始位置
void set_yuantai_Angle(float target, float time){
    Servo_t *servo = &yuantai;
	target -=7.9f;
	if(target<=25.8f){
		target = 25.8f;
	}
    servo->target_angle = target;
	servo->time = time;
    servo->moving_flag = 1;
	// 等待运动完成，注意这里改为等待moving_flag为0
//    while(servo->moving_flag != 0){}
//    delay_ms1(50);
}

//2.7度为初始位置
//10度为抓物块的位置
//60度张开夹爪
void set_zhuashou_Angle(float target, float time){
	target -=1.0f; 
    Servo_t *servo = &zhuashou;
    servo->target_angle = target;
	servo->time = time;
    servo->moving_flag = 1;
    // 等待运动完成，注意这里改为等待moving_flag为0
    while(servo->moving_flag != 0){}
    delay_ms1(30);
}

void set_zhuashou_he(void)
{
	set_zhuashou_Angle(6.0f,300);
}	

void set_zhuashou_kai(void)
{
	set_zhuashou_Angle(45.0f,300);
}	
	
//20.0f度放置物块一
//20.0f+113.0f度放置物块二
//20.0f+113.0f+117.0f度放置物块三
void set_wukuaipingtai_Angle(float target, float time){
	target -=0.0f; 
    Servo_t *servo = &wukuaipingtai;
    servo->target_angle = target;
	servo->time = time;
    servo->moving_flag = 1;
    // 等待运动完成，注意这里改为等待moving_flag为0
//    while(servo->moving_flag != 0){}
//    delay_ms1(50);
}

void set_wukuaipingtai_weizhi(int pos)
{	
	if(pos == 0x01){
		set_wukuaipingtai_Angle(20.0f,300);
	}else if(pos == 0x02){
		set_wukuaipingtai_Angle(20.0f+119.0f,300);
	}else if(pos == 0x03){
		set_wukuaipingtai_Angle(20.0f+119.0f+117.0f,300);
	}
}


// 统一调用三个伺服的更新函数
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        // 分别更新三个伺服的插值
        servo_update(&yuantai);
        servo_update(&zhuashou);
        servo_update(&wukuaipingtai);

        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

/*前三个参数分别为
	平推的目标位置：65.0f 到 -122.0 之间
	平推的速度： 1000 rpm
	平推的加速度：200rpm
  中间三个参数分别为
	升降的目标位置：0.0f 到 130.0f
	升降的速度：1000 rpm
	升降的加速度：200 rpm
  最后两个参数分别为
	云台的目标位置：0 到 360
	云台的转速：13左右

*/
int move_all(float target_pos, uint16_t speed, uint8_t accel,float target_pos1, uint16_t speed1, uint8_t accel1,float target, float speed_of_yuntai)
{	
	accel1 = 240;
	accel = 230;
	speed1 = 2000;
	double time1 = pingtui_control(target_pos,speed,accel);
	double time2 = shengjiang_control(target_pos1,speed1,accel1);
	double time;
	if(fabs(target - yuantai.current_angle) <= 28){
		time = 120 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 50){
		time = 100 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 100){
		time = 85 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 150){
		time = 70 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 250){
		time = 60 * fabs(target - yuantai.current_angle)/13.0f;
	}else{
		time = 50 * fabs(target - yuantai.current_angle)/13.0f;
	}
	set_yuantai_Angle(target,time);
	double max = fmax(time, fmax(time1, time2));
	delay_ms1((int)max);
	return (int)max;
}


int move_all1(float target_pos, uint16_t speed, uint8_t accel,float target_pos1, uint16_t speed1, uint8_t accel1,float target, float speed_of_yuntai)
{	
	accel1 = 240;
	accel = 230;
	speed1 = 2000;
	double time1 = pingtui_control(target_pos,speed,accel);
	double time2 = shengjiang_control(target_pos1,speed1,accel1);
	double time;
	if(fabs(target - yuantai.current_angle) <= 28){
		time = 120 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 50){
		time = 100 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 100){
		time = 85 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 150){
		time = 70 * fabs(target - yuantai.current_angle)/13.0f;
	}else if(fabs(target - yuantai.current_angle) <= 250){
		time = 60 * fabs(target - yuantai.current_angle)/13.0f;
	}else{
		time = 50 * fabs(target - yuantai.current_angle)/13.0f;
	}
	set_yuantai_Angle(target,time);
	double max = fmax(time, fmax(time1, time2));
	return (int)max;
}

