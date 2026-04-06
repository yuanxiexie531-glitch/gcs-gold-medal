#ifndef __IMU_H
#define __IMU_H

void imu_init();
void imu_scan(float* fAcc,float* fGyro,float* fAngle);
void Uart2Send(unsigned char *p_data, unsigned int uiSize);
void Usart2Init(unsigned int uiBaud);

#endif