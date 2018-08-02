#include "io.h"

#include <string.h>

#include "devcfg.h"
#include "nrf24l01.h"
#include "rf125k.h"
#include "uart.h"

volatile uint32_t ulSysTick = 0xFFFFFF00;

void OSTimeDly(uint32_t ulTick);

/**********************************************************************
**函数原型:   void Port_Init(void)
**入口参数:   无
**返 回 值:   无
**功    能:   外部端口初始化
**说    明:   
************************************************************************/
void Port_Init(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;   
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 	
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

/**********************************************************************
**函数原型:   void vBeepDelay(unsigned int unDelay)
**入口参数:   延时时间
**返 回 值:   无
**功    能:   蜂鸣器响
**说    明:   
************************************************************************/
void vBeepDelay(unsigned int unDelay)
{
    BEEP_ON();
    OSTimeDly(unDelay);
    BEEP_OFF();
}

/**********************************************************************
**函数原型:   uint32_t ulGetCurTick(void) 
**入口参数:   无
**返 回 值:   无
**功    能:   获取当前时钟节拍
**说    明:   
************************************************************************/
static uint32_t ulGetCurTick(void)
{
    //__disable_irq();
		uint32_t ulTick = ulSysTick;
    //__enable_irq();
	
		return ulTick;
}

/**********************************************************************
**函数原型:   void CheckOverflowCurTick(uint32_t ulTick) 
**入口参数:   时钟节拍
**返 回 值:   无
**功    能:   复位时钟节拍
**说    明:   
************************************************************************/
static void CheckOverflowCurTick(uint32_t ulTick)
{
    if(ulTick >= 0xFFFFFF00)
		{
		    ulSysTick = 0;		
		}   
}

/**********************************************************************
**函数原型:   void OSTimeDly(uint32_t ulTick)
**入口参数:   延时时间
**返 回 值:   无
**功    能:   系统延时
**说    明:   
************************************************************************/
extern uint8_t Tx_Buffer1[6];

void OSTimeDly(uint32_t ulTick)
{
    uint64_t ulBeginTick;
    uint64_t ulCurTick = 0;

    ulBeginTick = ulGetCurTick();
    ulCurTick   = ulBeginTick;

    while(1)
    {
        if(nrf_rx_inf.rx_irq_flag)
				{
						nrf_rx_inf.rx_irq_flag = false;
						
						NRF_Rx_Dat(&nrf_rx_inf);
						UART_TTL_SendString((void*)&nrf_rx_inf.rx_data[0], nrf_rx_inf.rx_len);
						UART_TTL_SendByte(nrf_rx_inf.rx_len);
						memset((void*)&nrf_rx_inf.rx_data[0], 0, nrf_rx_inf.rx_len);
						nrf_rx_inf.rx_len = 0;
				}
				
				if(CFG_DEV_TYPE_TRI_FREQ == uchDevCfgGetDevType())
				{
						vSig125kSndTask();
				}
						
				ulCurTick = ulGetCurTick();
        if(ulCurTick-ulBeginTick >= ulTick)
        {
            CheckOverflowCurTick(ulCurTick);
						break;
        }
    }
}




