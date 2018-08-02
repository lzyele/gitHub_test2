#include "flash.h"

#include "string.h"

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000800)   /* FLASH Page Size 2K*/
#define FLASH_USER_START_ADDR   ((uint32_t)0x08009000)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x08020000)   /* End @ of user Flash area */

//#define DATA_32                 ((uint32_t)0x12345678)
/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private variables ---------------------------------------------------------*/
uint32_t EraseCounter = 0x00, Address = 0x00;

//保存数据在FLASH里面
int nSaveCfgParaToFlash(unsigned char *puchData, int nLenData, unsigned char uchFlg)
{
	
		__IO TestStatus MemoryProgramStatus = PASSED;
		/* Unlock the Flash to enable the flash control register access *************/ 
		FLASH_Unlock();
	
		/* Clear pending flags (if any) */  
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
	
		__disable_irq();
		if (FLASH_ErasePage(FLASH_USER_START_ADDR + FLASH_PAGE_SIZE*uchFlg)!= FLASH_COMPLETE)
		{
				MemoryProgramStatus = FAILED;  
		}
	
		/* Program the user Flash area word by word
		(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

		Address = FLASH_USER_START_ADDR + FLASH_PAGE_SIZE*uchFlg;
		if((nLenData%4)!=0)
		{
				nLenData +=4;
		}
		nLenData = nLenData/4;
	
		for(uint8_t i=0; i<nLenData; i++)
		{
				uint32_t data  = 0;
		
				data = puchData[i*4+0]+ (puchData[i*4+1]<<8)+ (puchData[i*4+2]<<16)+ (puchData[i*4+3]<<24);
		
				if (FLASH_ProgramWord(Address+i*4, data) != FLASH_COMPLETE)
				{
						MemoryProgramStatus = FAILED;  
				}
		
				uint32_t read_data = *(__IO uint32_t *)Address+i*4;
				if (read_data != data)
				{
						MemoryProgramStatus = FAILED;  
				}
		}
	
		__enable_irq();

		/* Lock the Flash to disable the flash control register access (recommended
		to protect the FLASH memory against possible unwanted operation) *********/
		FLASH_Lock(); 

		return (uint8_t)MemoryProgramStatus;
}

uint8_t nReadCfgParaFromFlash(unsigned char *puchData, unsigned int unLenData ,unsigned char uchFlg)
{
	
		memcpy(puchData, (unsigned char *) (FLASH_USER_START_ADDR+FLASH_PAGE_SIZE*uchFlg), unLenData);
	
		return 0;
}
