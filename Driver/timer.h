#ifndef __TIMER_H
#define	__TIMER_H

#include "stm32f0xx.h"

void TIM1_Config(uint16_t uchDutyCycle);
void Set_TIM1_Frequency(uint32_t frequency);
void TIM3_Config(void);

#endif /* __TIMER_H */
