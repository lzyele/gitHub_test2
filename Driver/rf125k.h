#ifndef __RF125K_H
#define	__RF125K_H

#include "stm32f0xx.h"

#define  ANT_X_SWITCH_PIN   GPIO_Pin_1       
#define  ANT_X_SWITCH_OFF() GPIOB->BSRR = ANT_X_SWITCH_PIN;//高电平关闭
#define  ANT_X_SWITCH_ON()  GPIOB->BRR  = ANT_X_SWITCH_PIN;


#define  ANT_Y_SWITCH_PIN   GPIO_Pin_0
#define  ANT_Y_SWITCH_OFF() GPIOB->BSRR = ANT_Y_SWITCH_PIN;//高电平关闭
#define  ANT_Y_SWITCH_ON()  GPIOB->BRR =  ANT_Y_SWITCH_PIN;

typedef __packed struct
{
    uint8_t uchType;          /*消息类型*/
    uint8_t uchLen;           /*消息长度*/
    uint8_t uchRemoteAddr[3]; /*远端读卡器接收地址*/
    uint8_t uchSubAddr;       /*触发器子地址*/
    uint8_t uchFreq;          /*2.4GHz参数 进出标志*/
    uint8_t uchLrc;           /*lrc 检验*/
	
}AIR_MSG_FRAME, *pST_MSG_FRAME;

extern volatile uint8_t 	g_uchPwm1OptFlg; //PWM1开关:0关闭1打开2暂停
extern volatile uint8_t 	g_uchPwm2OptFlg; //PWM2开关:0关闭1打开2暂停
extern uint8_t  g_uchIsTriggerLowSignal;

void vPwmIrq(void);
void vSig125kInit(void);
void vSig125kSndFrameTest(uint8_t uchIndex);
void vSig125kSndTask(void);

#endif /* __RF125K_H */
