#ifndef __IO_H
#define	__IO_H

#include "stm32f0xx.h"
#include <stdint.h>

#define BEEP_OFF() (GPIOA->BRR  = GPIO_Pin_15)
#define BEEP_ON()  (GPIOA->BSRR = GPIO_Pin_15)

extern volatile uint32_t ulSysTick;
extern volatile uint8_t  g_uchShortSysTick;

void Port_Init(void);
void vBeepDelay(unsigned int unDelay);
void OSTimeDly(uint32_t ulTick);

#endif /* __IO_H */

