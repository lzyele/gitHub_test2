#ifndef __UART_H
#define	__UART_H

#include "stm32f0xx.h"
#include <stdio.h>

//lzy_a 自己添加的  GitHub测试

void UART_TTL_Init(void);
void UART_TTL_SendByte(uint8_t byte);
void UART_TTL_SendString(uint8_t *string, uint8_t len);
int fputc(int ch, FILE *f);

void RS485_Init(void);
void RS485_SendString(uint8_t *string, uint8_t len);

#endif /* __UART_H */
