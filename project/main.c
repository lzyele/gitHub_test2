#include "stm32f0xx.h"

#include <stdio.h>
#include <string.h>

#include "devcfg.h"
#include "nrf24l01.h"
#include "io.h"
#include "rf125k.h"
#include "rc522.h"
#include "timer.h"
#include "uart.h"

#define countof(a) (sizeof(a) / sizeof(*(a))) //计算数组内的成员个数
uint8_t Tx_Buffer[] = "Start!\n";

void System_Init(void)
{
		//nDevCfgSetDefaultPara();
		nDevCfgLoadCfgParaFromFlash();
	
		Port_Init();
		TIM3_Config();
	
		UART_TTL_Init();
		RS485_Init();
		RC522_Init();
}

int main(void)
{ 
		SystemInit();
		System_Init();
		
		UART_TTL_SendString(Tx_Buffer, countof(Tx_Buffer)-1);	
		RS485_SendString   (Tx_Buffer, countof(Tx_Buffer)-1);
			
		nNrfInit(NRF_PRIM_RX, NRF_RF_CH_1, NRF_TRAN_SPEED_1Mbps, NRF_TRAN_POWER_0dbm);
		//nNrf_test();
		
		vSig125kInit();
		
		vBeepDelay(50);
	
		while(1)
		{
				FD_Card();
				OSTimeDly(300);
		}
}
