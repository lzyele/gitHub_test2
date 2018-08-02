#include "io.h"

#include <string.h>

#include "devcfg.h"
#include "nrf24l01.h"
#include "rf125k.h"
#include "uart.h"

volatile uint32_t ulSysTick = 0xFFFFFF00;

void OSTimeDly(uint32_t ulTick);

/**********************************************************************
**����ԭ��:   void Port_Init(void)
**��ڲ���:   ��
**�� �� ֵ:   ��
**��    ��:   �ⲿ�˿ڳ�ʼ��
**˵    ��:   
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
**����ԭ��:   void vBeepDelay(unsigned int unDelay)
**��ڲ���:   ��ʱʱ��
**�� �� ֵ:   ��
**��    ��:   ��������
**˵    ��:   
************************************************************************/
void vBeepDelay(unsigned int unDelay)
{
    BEEP_ON();
    OSTimeDly(unDelay);
    BEEP_OFF();
}

/**********************************************************************
**����ԭ��:   uint32_t ulGetCurTick(void) 
**��ڲ���:   ��
**�� �� ֵ:   ��
**��    ��:   ��ȡ��ǰʱ�ӽ���
**˵    ��:   
************************************************************************/
static uint32_t ulGetCurTick(void)
{
    //__disable_irq();
		uint32_t ulTick = ulSysTick;
    //__enable_irq();
	
		return ulTick;
}

/**********************************************************************
**����ԭ��:   void CheckOverflowCurTick(uint32_t ulTick) 
**��ڲ���:   ʱ�ӽ���
**�� �� ֵ:   ��
**��    ��:   ��λʱ�ӽ���
**˵    ��:   
************************************************************************/
static void CheckOverflowCurTick(uint32_t ulTick)
{
    if(ulTick >= 0xFFFFFF00)
		{
		    ulSysTick = 0;		
		}   
}

/**********************************************************************
**����ԭ��:   void OSTimeDly(uint32_t ulTick)
**��ڲ���:   ��ʱʱ��
**�� �� ֵ:   ��
**��    ��:   ϵͳ��ʱ
**˵    ��:   
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




