#ifndef __CAN_H
#define __CAN_H

#include "board.h"
#include "fifo.h"

/**********************************************************
***	ZDT_X57步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

typedef struct {
	__IO CanRxMsg CAN_RxMsg;
	__IO CanTxMsg CAN_TxMsg;

	__IO bool rxFrameFlag;
}CAN_t;

void can_SendCmd(__IO uint8_t *cmd, uint8_t len);
uint8_t can_GetRxFlag(void);
extern __IO CAN_t can;

#endif
