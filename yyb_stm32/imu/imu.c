#include "stm32f4xx.h"                  // Device header
#include "wit_c_sdk.h"
#include "delay.h"
#include "timer.h"

#define ACC_UPDATE		0x01
#define GYRO_UPDATE		0x02
#define ANGLE_UPDATE	0x04
#define MAG_UPDATE		0x08
#define READ_UPDATE		0x80
float realbaud = 0;
static volatile char s_cDataUpdate = 0, s_cCmd = 0xff;
const uint32_t c_uiBaud[10] = {0, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
static void CmdProcess(void);
static void AutoScanSensor(void);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
void Usart2Init(unsigned int uiBaud);//函数的提前声明

void imu_init()
{ 
	Timer_Init();
	Usart2Init(115200);
	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(SensorDataUpdata);
	WitDelayMsRegister(Delayms);
	AutoScanSensor();
}

void Usart2Init(unsigned int uiBaud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure; 

    // Enable clocks for GPIOD and USART2
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    // Configure PD5 as alternate function (TX)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // Enable pull-up resistor
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // Configure PD6 as alternate function (RX)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // No pull-up/pull-down for RX
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // Connect pins to USART2
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

    // USART configuration
    USART_InitStructure.USART_BaudRate = uiBaud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure); 

    // Enable RX interrupt
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    // Enable USART
    USART_Cmd(USART2, ENABLE);
    
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
	
    // NVIC configuration for USART2
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART2_IRQHandler(void)
{
    unsigned char ucTemp;
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        ucTemp = USART_ReceiveData(USART2);
        WitSerialDataIn(ucTemp);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void Uart2Send(unsigned char *p_data, unsigned int uiSize)
{   
    unsigned int i;
    for (i = 0; i < uiSize; i++)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, *p_data++);
    }
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

void CopeCmdData(unsigned char ucData)
{
	static unsigned char s_ucData[50], s_ucRxCnt = 0;
	
	s_ucData[s_ucRxCnt++] = ucData;
	if(s_ucRxCnt<3)return;										//Less than three data returned
	if(s_ucRxCnt >= 50) s_ucRxCnt = 0;
	if(s_ucRxCnt >= 3)
	{
		if((s_ucData[1] == '\r') && (s_ucData[2] == '\n'))
		{
			s_cCmd = s_ucData[0];
			memset(s_ucData,0,50);//
			s_ucRxCnt = 0;
		}
		else 
		{
			s_ucData[0] = s_ucData[1];
			s_ucData[1] = s_ucData[2];
			s_ucRxCnt = 2;
			
		}
	}

}
static void ShowHelp(void)
{
//	printf("\r\n************************	 WIT_SDK_DEMO	************************");
//	printf("\r\n************************          HELP           ************************\r\n");
//	printf("UART SEND:a\\r\\n   Acceleration calibration.\r\n");
//	printf("UART SEND:m\\r\\n   Magnetic field calibration,After calibration send:   e\\r\\n   to indicate the end\r\n");
//	printf("UART SEND:U\\r\\n   Bandwidth increase.\r\n");
//	printf("UART SEND:u\\r\\n   Bandwidth reduction.\r\n");
//	printf("UART SEND:B\\r\\n   Baud rate increased to 115200.\r\n");
//	printf("UART SEND:b\\r\\n   Baud rate reduction to 9600.\r\n");
//	printf("UART SEND:R\\r\\n   The return rate increases to 10Hz.\r\n");
//	printf("UART SEND:r\\r\\n   The return rate reduction to 1Hz.\r\n");
//	printf("UART SEND:C\\r\\n   Basic return content: acceleration, angular velocity, angle, magnetic field.\r\n");
//	printf("UART SEND:c\\r\\n   Return content: acceleration.\r\n");
//	printf("UART SEND:h\\r\\n   help.\r\n");
//	printf("******************************************************************************\r\n");
}

static void CmdProcess(void)
{
	switch(s_cCmd)
	{
//		case 'a':	
//			if(WitStartAccCali() != WIT_HAL_OK) 
//				//printf("\r\nSet AccCali Error\r\n");
//			break;
//		case 'm':	
//			if(WitStartMagCali() != WIT_HAL_OK) 
//				//printf("\r\nSet MagCali Error\r\n");
//			break;
//		case 'e':	
//			if(WitStopMagCali() != WIT_HAL_OK)
//				//printf("\r\nSet MagCali Error\r\n");
//			break;
//		case 'u':	
//			if(WitSetBandwidth(BANDWIDTH_5HZ) != WIT_HAL_OK) 
//				//printf("\r\nSet Bandwidth Error\r\n");
//			break;
//		case 'U':	
//			if(WitSetBandwidth(BANDWIDTH_256HZ) != WIT_HAL_OK) 
//				//printf("\r\nSet Bandwidth Error\r\n");
//			break;
//		case 'B':	
//			if(WitSetUartBaud(WIT_BAUD_115200) != WIT_HAL_OK) 
//				printf("\r\nSet Baud Error\r\n");
//			else 
//				Usart2Init(c_uiBaud[WIT_BAUD_115200]);											
//			break;
//		case 'b':	
//			if(WitSetUartBaud(WIT_BAUD_9600) != WIT_HAL_OK)
//				printf("\r\nSet Baud Error\r\n");
//			else 
//				Usart2Init(c_uiBaud[WIT_BAUD_9600]);												
//			break;
//		case 'R':	
//			if(WitSetOutputRate(RRATE_10HZ) != WIT_HAL_OK) 
//				printf("\r\nSet Rate Error\r\n");
//			break;
//		case 'r':	
//			if(WitSetOutputRate(RRATE_1HZ) != WIT_HAL_OK) 
//				printf("\r\nSet Rate Error\r\n");
//			break;
//		case 'C':	
//			if(WitSetContent(RSW_ACC|RSW_GYRO|RSW_ANGLE|RSW_MAG) != WIT_HAL_OK) 
//				printf("\r\nSet RSW Error\r\n");
//			break;
//		case 'c':	
//			if(WitSetContent(RSW_ACC) != WIT_HAL_OK) 
//				printf("\r\nSet RSW Error\r\n");
//			break;
//		case 'h':
//			ShowHelp();
//			break;
	}
	s_cCmd = 0xff;
}

static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
	Uart2Send(p_data, uiSize);
}

static void Delayms(uint16_t ucMs)
{
	delay_ms(ucMs);
}

static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
    for(i = 0; i < uiRegNum; i++)
    {
        switch(uiReg)
        {
//            case AX:
//            case AY:
            case AZ:
				s_cDataUpdate |= ACC_UPDATE;
            break;
//            case GX:
//            case GY:
            case GZ:
				s_cDataUpdate |= GYRO_UPDATE;
            break;
//            case HX:
//            case HY:
            case HZ:
				s_cDataUpdate |= MAG_UPDATE;
            break;
//            case Roll:
//            case Pitch:
            case Yaw:
				s_cDataUpdate |= ANGLE_UPDATE;
            break;
            default:
				s_cDataUpdate |= READ_UPDATE;
			break;
        }
		uiReg++;
    }
}

static void AutoScanSensor(void)
{
	int i, iRetry;
	
	for(i = 1; i < 10; i++)
	{
		Usart2Init(c_uiBaud[i]);
		realbaud = c_uiBaud[i];
		iRetry = 2;
		do
		{
			s_cDataUpdate = 0;
			WitReadReg(AX, 3);
			delay_ms(100);
			if(s_cDataUpdate != 0)
			{
				ShowHelp();
				return ;
			}
			iRetry--;
		}while(iRetry);		
	}
}

void imu_scan(float* fAcc,float* fGyro,float* fAngle)
{   
	int i;
	float fAngle_tmp = 0;
		if(s_cDataUpdate)
		{
			for(i = 0; i < 3; i++)
			{
				fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
				fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
			}
			if(sReg[TEMP] - sReg[Roll] > 10 | sReg[Roll] - sReg[TEMP] > 10)
			{
				fAngle[0] = sReg[Roll] / 32768.0f * 180.0f;
				fAngle_tmp = fAngle[0];
			}
			else   fAngle[0] = fAngle_tmp;
			
			for(i = 1; i<3;i++)
			{
				fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
			}
		}
}

