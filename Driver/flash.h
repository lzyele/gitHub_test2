#ifndef __FLASH_H
#define __FLASH_H	

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"

int nSaveCfgParaToFlash(uint8_t *puchData, int nLenData, uint8_t uchFlg);
uint8_t nReadCfgParaFromFlash(unsigned char *puchData, unsigned int unLenData ,unsigned char uchFlg);

#endif
