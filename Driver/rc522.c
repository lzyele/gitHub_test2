#include "rc522.h"
#include "uart.h"
#include <string.h>

CARD_STRUCT card_inf; 

#define rc522_spi_mode  0
#define rc522_dubug     1

#if rc522_spi_mode
		
#else
		#define	STU_522_MISO	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)
		
		#define SET_522_MOSI	GPIO_SetBits(GPIOA, GPIO_Pin_7)
		#define CLR_522_MOSI	GPIO_ResetBits(GPIOA, GPIO_Pin_7)
		
		#define SET_522_CK		GPIO_SetBits(GPIOA, GPIO_Pin_5)
		#define CLR_522_CK		GPIO_ResetBits(GPIOA, GPIO_Pin_5)
#endif

		#define SET_522_CS		GPIO_SetBits(GPIOA, GPIO_Pin_4)
		#define CLR_522_CS		GPIO_ResetBits(GPIOA, GPIO_Pin_4)

		#define SET_522_RST		GPIO_SetBits(GPIOA, GPIO_Pin_12)
		#define CLR_522_RST		GPIO_ResetBits(GPIOA, GPIO_Pin_12)

/**********************************************************************
**函数原型:   void RC522_Delay(uint16_t cnt)
**入口参数:   延时时间
**返 回 值:   无
**功    能:   RC522延时
**说    明:   
************************************************************************/
static void RC522_Delay(uint16_t cnt)  
{
		cnt *=48; //主频48M
		while(cnt--);
}

#if rc522_spi_mode
/**********************************************************************
**函数原型:   void spi_master_init(void)
**入口参数:   NULL
**返 回 值:   NULL
**功    能:   RC522 SPI初始化
**说    明:   
************************************************************************/
static void spi_master_init(void)
{		
			GPIO_InitTypeDef GPIO_InitStructure;
			SPI_InitTypeDef  SPI_InitStructure;

			RCC_AHBPeriphClockCmd (RCC_AHBPeriph_GPIOA, ENABLE);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

			/* SPI1 管脚复用功能 */
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_0);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);
			
			/* SPI SCK, MISO, MOSI pin configuration */
			GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
			GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			
			//CSN2->PA4	
			GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
			GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
			GPIO_Init(GPIOA, &GPIO_InitStructure);

			//RST->PA12	
			GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
			GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			
			SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
			SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;  //设置SPI工作模式:设置为主SPI
			SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b;  //设置SPI的数据大小:SPI发送接收8位帧结构
			SPI_InitStructure.SPI_CPOL      = SPI_CPOL_Low;	 	  //串行同步时钟的空闲状态为低电平
			SPI_InitStructure.SPI_CPHA      = SPI_CPHA_1Edge;	  //串行同步时钟的第一个跳变沿（上升或下降）数据被采样
			SPI_InitStructure.SPI_NSS 			= SPI_NSS_Soft;		  //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
			SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; //定义波特率预分频的值:波特率预分频值为32
			SPI_InitStructure.SPI_FirstBit  = SPI_FirstBit_MSB; //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
			SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC值计算的多项式
			SPI_Init(SPI1, &SPI_InitStructure); //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
			
			SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
		 
			SPI_Cmd(SPI1, ENABLE); //使能SPI外设			
}
#else
static void spi_master_init(void)
{		
		GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 |GPIO_Pin_12;   
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 	
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;   
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;    	
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
}
#endif

#if rc522_spi_mode
/**********************************************************************
**函数原型:   uint8_t SPI1_RW_Byte(uint8_t TxData)
**入口参数:   发送数据
**返 回 值:   读取数据
**功    能:   读写SPI
**说    明:   
************************************************************************/
static uint8_t SPI1_RW_Byte(uint8_t TxData)
{				 	
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{

		}
		SPI_SendData8(SPI1, TxData); //通过外设SPIx发送一个数据

		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
		{

		}
		return SPI_ReceiveData8(SPI1); //返回通过SPIx最近接收的数据				 	    
}

/**********************************************************************
**函数原型:   uint8_t spi_write_reg(uint8_t ucRegAddr, uint8_t ucRegVal)
**入口参数:   ucRegAddr：写地址；ucRegVal：写数据
**返 回 值:   true or false
**功    能:   写寄存器
**说    明:   
************************************************************************/
static uint8_t SPIReadByte(void)
{                                                        
		return (SPI1_RW_Byte(0xFF));                                              
}

/**********************************************************************
**函数原型:   static uint8_t spi_read_reg(uint8_t ucRegAddr)
**入口参数:   ucRegAddr：读地址
**返 回 值:   寄存器值
**功    能:   读寄存器
**说    明:   
************************************************************************/
static void SPIWriteByte(uint8_t SPIData)
{
		SPI1_RW_Byte(SPIData);	
}
#else
/**********************************************************************
**函数原型:   uint8_t SPIReadByte(void)
**入口参数:   无
**返 回 值:   读数据
**功    能:   SPI读  
**说    明:   
************************************************************************/
static uint8_t SPIReadByte(void)
{
    uint8_t SPICount = 0;                                      		  
    uint8_t SPIData  = 0;                  
    
		for(; SPICount < 8; SPICount++)                  
    {
        SPIData <<=1;                                            
        CLR_522_CK; 
				RC522_Delay(1);                             								  
					
				if(STU_522_MISO)
        {
            SPIData|=0x01;
        }
        
				SET_522_CK;   
				RC522_Delay(1);                                              	
   }                                                              
   
	 return (SPIData);                                              
}

/**********************************************************************
**函数原型:   void SPIWriteByte(uint8_t SPIData)
**入口参数:   写数据
**返 回 值:   无
**功    能:   SPI写
**说    明:   
************************************************************************/
static void SPIWriteByte(uint8_t SPIData)
{
    uint8_t SPICount = 0;                                       			
    
		for(; SPICount < 8; SPICount++)
    {
        if (SPIData & 0x80)
        {
            SET_522_MOSI;
        }  
        else
        {
            CLR_522_MOSI;
        }
				RC522_Delay(1);   
        
				CLR_522_CK;
				RC522_Delay(1);   
        
				SET_522_CK;
				RC522_Delay(1);   

				SPIData <<= 1;
    }	
}
#endif

/**********************************************************************
**函数原型:   uint8_t ReadRawRC(uint8_t Address)
**入口参数:  	Address：寄存器地址
**返 回 值:   寄存器值
**功    能:   读RC522寄存器
**说    明:   
************************************************************************/
static uint8_t ReadRawRC(uint8_t  Address)
{
    uint8_t ucAddr   = ((Address <<1) &0x7E) |0x80;
    uint8_t ucResult = 0;
		
		CLR_522_CS;
		SPIWriteByte(ucAddr);
		ucResult = SPIReadByte();
		SET_522_CS;
	
		return ucResult;
}

/**********************************************************************
**函数原型:   void WriteRawRC(uint8_t Address, uint8_t value)
**入口参数:  	Address：寄存器地址；value：写入的值
**返 回 值:   无
**功    能:   写RC522寄存器
**说    明:   
************************************************************************/
static void WriteRawRC(uint8_t  Address, uint8_t  value)
{  
    uint8_t ucAddr = (Address <<1) &0x7E;

		CLR_522_CS;
		SPIWriteByte(ucAddr);
		SPIWriteByte(value);
		SET_522_CS;
}

/**********************************************************************
**函数原型:   void SetBitMask(uint8_t reg, uint8_t mask) 
**入口参数:  	reg：寄存器地址；mask；置位值
**返 回 值:   无
**功    能:   置RC522寄存器位
**说    明:   
************************************************************************/
static void SetBitMask(uint8_t reg, uint8_t mask)  
{
    char tmp = ReadRawRC(reg);
 
    WriteRawRC(reg, tmp | mask);
}

/**********************************************************************
**函数原型:   void ClearBitMask(uint8_t reg, uint8_t mask) 
**入口参数:  	reg：寄存器地址；mask：清位值
**返 回 值:   无
**功    能:   清RC522寄存器位
**说    明:   
************************************************************************/
static void ClearBitMask(uint8_t reg, uint8_t mask)  
{
    char tmp = ReadRawRC(reg);
    
    WriteRawRC(reg, tmp & (~mask));
} 

/**********************************************************************
**函数原型:   void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
**入口参数:  	pIndata：输入数据；len：数据长度；pOutData：输出数据
**返 回 值:   无
**功    能:   计算CRC16
**说    明:   
************************************************************************/ 
static void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
    uint8_t i=0, n=0;
  
		ClearBitMask(DivIrqReg,  0x04);
    WriteRawRC(CommandReg,   PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    
		for(; i<len; i++)
    {   
				WriteRawRC(FIFODataReg, *(pIndata+i));
		}
    WriteRawRC(CommandReg, PCD_CALCCRC);
    
		i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while((i!=0) && !(n&0x04));
		
		pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/**********************************************************************
**函数原型:   char PcdComMF522(uint8_t  Command, 
															uint8_t  *pInData, 
															uint8_t  InLenByte,
															uint8_t  *pOutData, 
															uint32_t *pOutLenBit) 
**入口参数:  	Command：RC522命令字；pInData：发送到卡片的数据；InLenByte：发送数据的字节长度；pOutData：接收卡片的返回数据；pOutLenBit：返回数据的长度
**返 回 值:   成功返回MI_OK
**功    能:   RC522与卡通信
**说    明:   
************************************************************************/
static char PcdComMF522(uint8_t  Command, 
											  uint8_t  *pInData, 
												uint8_t  InLenByte,
											  uint8_t  *pOutData, 
											  uint16_t *pOutLenBit)
{
    char status = MI_ERR;
    uint8_t  irqEn    = 0x00;
    uint8_t  waitFor  = 0x00;
    uint8_t  lastBits = 0x00;
    uint8_t  n = 0;
    uint16_t i = 0;
    
	switch (Command)
    {
				case PCD_AUTHENT:
						irqEn   = 0x12;
						waitFor = 0x10;
						break;
		
				case PCD_TRANSCEIVE:
						irqEn   = 0x77;
						waitFor = 0x30;
						break;
		
				default:
						break;
    }
   
		WriteRawRC(ComIEnReg, 	 irqEn|0x80);
    ClearBitMask(ComIrqReg,  0x80);
    WriteRawRC(CommandReg, 	 PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
		
    for(; i<InLenByte; i++)
    {   
				WriteRawRC(FIFODataReg, pInData[i]);
		}
		WriteRawRC(CommandReg, Command);
			
    if(Command == PCD_TRANSCEIVE)
    {    
				SetBitMask(BitFramingReg, 0x80);  
		}
		
		i = 6000;
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while((i!=0) && !(n&0x01) && !(n&waitFor));
		ClearBitMask(BitFramingReg, 0x80);
		
    if(i!=0)
    {    
				if(!(ReadRawRC(ErrorReg)&0x1B))
        {
						status = MI_OK;
            
						if(n & irqEn & 0x01)
            {   
								status = MI_NOTAGERR;   
						}
            
						if(Command == PCD_TRANSCEIVE)
            {
								n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;
							
								if(lastBits)
                {   
										*pOutLenBit = (n-1)*8 + lastBits;   
								}
                else
                {   
										*pOutLenBit = n*8;   
								}
                
								if (n == 0)
                {   
										n = 1;    
								}
								
								if(n > MAXRLEN)
                {   
										n = MAXRLEN;   
								}
								
								for(i=0; i<n; i++)
                {   
										pOutData[i] = ReadRawRC(FIFODataReg);    
								}
            }
        }
        else
        {   
						status = MI_ERR;   
				}
        
    }
   
    SetBitMask(ControlReg, 0x80); // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE); 
    
		return status;	
}

/**********************************************************************
**函数原型:   char PcdRequest(uint8_t req_code, uint8_t *pTagType)
**入口参数:   req_code：寻卡方式；pTagType：卡片类型代码
**返 回 值:   成功返回MI_OK
**功    能:   寻卡  
**说    明:   
************************************************************************/
static char PcdRequest(uint8_t req_code, uint8_t *pTagType)
{
		char  	 status = MI_ERR;  
		uint16_t unLen  = 0;
		uint8_t  ucComMF522Buf[MAXRLEN] = {0}; 

		ClearBitMask(Status2Reg,  0x08);
		WriteRawRC(BitFramingReg, 0x07);
		SetBitMask(TxControlReg,  0x03);
		
		ucComMF522Buf[0] = req_code;

		status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);

		if ((status == MI_OK) && (unLen == 0x10))
		{    
				*pTagType     = ucComMF522Buf[0];
				*(pTagType+1) = ucComMF522Buf[1];
		}
		else
		{   
				status = MI_ERR;   
		}
   
		return status;	
}

/**********************************************************************
**函数原型:   char PcdAnticoll(uint8_t *pSnr)
**入口参数:   rpSnr：卡片序列号
**返 回 值:   成功返回MI_OK
**功    能:   防冲撞
**说    明:   
************************************************************************/
static char PcdAnticoll(uint8_t *pSnr)
{
    char  	 status = MI_ERR;
    uint8_t  i=0, snr_check=0;
    uint16_t unLen = 0;
    uint8_t  ucComMF522Buf[MAXRLEN] = {0}; 
    

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,    0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if(status == MI_OK)
    {
    	 for(; i<4; i++)
       {   
           *(pSnr+i)  = ucComMF522Buf[i];
           snr_check ^= ucComMF522Buf[i];
       }
       if(snr_check != ucComMF522Buf[i])
       {   
				   status = MI_ERR;    
			 }
    }
    
    SetBitMask(CollReg, 0x80);
    return status;
}

/**********************************************************************
**函数原型:   char PcdSelect(uint8_t *pSnr)
**入口参数:   rpSnr：卡片序列号
**返 回 值:   成功返回MI_OK
**功    能:   选定卡片
**说    明:   
************************************************************************/
static char PcdSelect(uint8_t *pSnr)
{
    char  	 status = MI_ERR;
    uint8_t  i      = 0;
    uint16_t unLen  = 0;
    uint8_t  ucComMF522Buf[MAXRLEN] = {0}; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for(; i<4; i++)
    {
				ucComMF522Buf[i+2] = *(pSnr+i);
				ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);
    
    if((status == MI_OK) && (unLen == 0x18))
    {   
				status = MI_OK;  
		}
    else
    {   
				status = MI_ERR;    
		}

    return status;
}

/**********************************************************************
**函数原型:   char PcdAuthState(uint8_t auth_mode, uint8_t addr, uint8_t *pKey, uint8_t *pSnr)
**入口参数:   auth_mode：密码验证模式；addr：块地址；pKey：密码；pSnr：卡片序列号
**返 回 值:   成功返回MI_OK
**功    能:   验证卡片密钥
**说    明:   
************************************************************************/             
static char PcdAuthState(uint8_t auth_mode, uint8_t addr, uint8_t *pKey, uint8_t *pSnr)
{
    char  		status = MI_ERR;
    uint16_t  unLen  = 0;
    uint8_t   i=0, ucComMF522Buf[MAXRLEN] = {0}; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for(i=0; i<6; i++)
    {    
				ucComMF522Buf[i+2] = *(pKey+i);   
		}
    for(i=0; i<4; i++)
    {    
				ucComMF522Buf[i+8] = *(pSnr+i);   
		}
 
    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   
				status = MI_ERR;   
		}
    
    return status;	
}

/**********************************************************************
**函数原型:   char PcdRead(uint8_t addr, uint8_t *pData)
**入口参数:  	addr：块地址；pData：读出的数据
**返 回 值:   成功返回MI_OK
**功    能:   读取M1卡一块数据
**说    明:   
************************************************************************/
static char PcdRead(uint8_t addr, uint8_t *pData)
{
    char     status = MI_ERR;
    uint16_t unLen  = 0;
    uint8_t  i=0, ucComMF522Buf[MAXRLEN] = {0}; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if((status == MI_OK) && (unLen == 0x90))
    {
        for(i=0; i<16; i++)
        {    
						*(pData+i) = ucComMF522Buf[i];   
				}
    }
    else
    {   
				status = MI_ERR;   
		}
    
    return status;
		
}

/**********************************************************************
**函数原型:   char PcdWrite(uint8_t addr, uint8_t *pData)
**入口参数:  	addr：块地址；pData：写入的数据
**返 回 值:   成功返回MI_OK
**功    能:   写数据到M1卡一块
**说    明:   
************************************************************************/                 
static char PcdWrite(uint8_t addr, uint8_t *pData)
{
    char     status = MI_ERR;
    uint16_t unLen  = 0;
		uint8_t  ucComMF522Buf[MAXRLEN] = {0}; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   
				status = MI_ERR;   
		}
        
    if(status == MI_OK)
    {
        memcpy(ucComMF522Buf, pData, 16);
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf, 18, ucComMF522Buf, &unLen);
				if((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   
						status = MI_ERR;   
				}
    }
 
    return status;		
}

/**********************************************************************
**函数原型:   char PcdHalt(void)
**入口参数：  无
**返 回 值:   成功返回MI_OK
**功    能:   命令卡片进入休眠状态
**说    明:   
************************************************************************/ 
static char PcdHalt(void)
{
    char     status = MI_ERR;
    uint16_t unLen  = 0;
    uint8_t  ucComMF522Buf[MAXRLEN] = {0}; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
		
		return status;
}

/**********************************************************************
**函数原型:   char PcdReset(void)
**入口参数:  	无
**返 回 值:   成功返回MI_OK
**功    能:   复位
**说    明:   
************************************************************************/
static char PcdReset(void)
{
		SET_522_RST;
		RC522_Delay(10);

		CLR_522_RST;
		RC522_Delay(10);

		SET_522_RST;
		RC522_Delay(10);
   
		WriteRawRC(CommandReg, PCD_RESETPHASE);
   
    WriteRawRC(ModeReg,      0x3D);
    WriteRawRC(TReloadRegL,  30);
    WriteRawRC(TReloadRegH,  0);
    WriteRawRC(TModeReg,     0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
	
		WriteRawRC(TxAutoReg,    0x40);
		
    return MI_OK;
}

/**********************************************************************
**函数原型:   void PcdAntennaOn(void)
**入口参数:  	无
**返 回 值:   无
**功    能:   开启天线
**说    明:   
************************************************************************/
//static void PcdAntennaOn(void)
//{ 
//		uint8_t i = ReadRawRC(TxControlReg);
//		
//    if(!(i & 0x03))
//    {
//        SetBitMask(TxControlReg, 0x03);
//    }
//}

/**********************************************************************
**函数原型:   void PcdAntennaOn(void)
**入口参数:  	无
**返 回 值:   无
**功    能:   关闭天线
**说    明:   
************************************************************************/
static void PcdAntennaOff(void)
{
		ClearBitMask(TxControlReg, 0x03);
}

/**********************************************************************
**函数原型:   char M500PcdConfigISOType(uint8_t type)
**入口参数:  	type：协议类型
**返 回 值:   成功返回MI_OK
**功    能:   设置RC522的工作方式
**说    明:   
************************************************************************/
static char M500PcdConfigISOType(uint8_t type)
{
   if(type == 'A')                      //ISO14443_A
   { 
			 ClearBitMask(Status2Reg,  0x08);
		   WriteRawRC(ModeReg,       0x3D);	//3F
       WriteRawRC(RxSelReg,      0x86);	//84
		   WriteRawRC(RFCfgReg,      0x7F); //4F
		   WriteRawRC(TReloadRegL,   30);	  //tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		   WriteRawRC(TReloadRegH,   0);
		   WriteRawRC(TModeReg,      0x8D);
		   WriteRawRC(TPrescalerReg, 0x3E);
   }
   else
	 { 
			return 1; 
   }
   
   return MI_OK; 
}

/**********************************************************************
**函数原型:   char RC522_Pdown(void)
**入口参数:  	无
**返 回 值:   成功返回MI_OK
**功    能:   关闭RC522
**说    明:   
************************************************************************/ 
static char RC522_Pdown(void)
{
    PcdHalt();
		
    CLR_522_RST;

		return MI_OK;
}

/**********************************************************************
**函数原型:   void RC522_Init(void)
**入口参数:  	无
**返 回 值:   无
**功    能: 	初始化RC522  
**说    明:   
************************************************************************/ 
void RC522_Init(void)
{
    spi_master_init();
				
		PcdReset();
    PcdAntennaOff();
    M500PcdConfigISOType('A');
		RC522_Delay(10);
}

/**********************************************************************
**函数原型:   uint8_t RE_Pcd(uint8_t *MLastSelectedSnr, uint8_t *RevBuffer)
**入口参数:  	MLastSelectedSnr：返回卡序列号；RevBuffer：返回卡类型
**返 回 值:   成功返回MI_OK
**功    能:   寻卡
**说    明:   
************************************************************************/
static uint8_t RE_Pcd(uint8_t *MLastSelectedSnr, uint8_t *RevBuffer)
{
		char status = MI_OK;	
	
		PcdReset();
		
		status = PcdRequest(PICC_REQALL, RevBuffer);   

		if(status != MI_OK) 
		{
				return status;	
		}
		
		status = PcdAnticoll(MLastSelectedSnr);      
		if(status != MI_OK) 
		{
				return status;
		}
		
		status=PcdSelect(MLastSelectedSnr);         
		if(status != MI_OK) 
		{
				return status;
		}
		
		return status;	
}

/**********************************************************************
**函数原型:   uint8_t RE_Sector(uint8_t Block_No, uint8_t *p)
**入口参数:  	Block_No：扇区号；p：读卡数据
**返 回 值:   操作成功与否，返回0，成功,返回1失败
**功    能:   读卡
**说    明:   
************************************************************************/
static uint8_t RE_Sector(uint8_t Block_No, uint8_t *p)
{
		char    status = MI_ERR;		
		uint8_t i = 0;
	
		for(; i<4; i++)
		{
				status = PcdRead(Block_No, p);		              
				if(status == MI_OK)
				{
						break;
				}
				if(i >= 3)
				{
						RC522_Pdown();
						return status;;		
				}	

		}
	
		if(status == MI_OK)
		{
				Block_No++;	                                  
				for(i=0; i<4; i++)
				{
						status = PcdRead(Block_No, p+16);		              
						if(status == MI_OK)
						{
								break;
						}
						if(i >= 3)
						{
								RC522_Pdown();
								return status;;		
						}	
				}
		}

		if(status == MI_OK)
		{
				Block_No++;	                                  
				for(i=0; i<4; i++)
				{
						status = PcdRead(Block_No, p+32);		               
						if(status == MI_OK)
						{
								break;
						}
						if(i >= 3)
						{
								RC522_Pdown();
								return status;;		
						}	
				}
		}
	  
		if(status != MI_OK)
		{
				RC522_Pdown();
				return status;
		}
	
		RC522_Pdown();
		return MI_OK;
}

/**********************************************************************
**函数原型:  uint8_t SE_Card(void)
**入口参数:  无
**返 回 值:  操作成功与否，返回0，成功,返回1失败
**功    能:  寻卡    
**说    明:   
************************************************************************/
static uint8_t SE_Card(void)
{
		char status = MI_OK;
	
		PcdReset();
		status = RE_Pcd((void *)&card_inf.serialno[0], (void *)&card_inf.type[0]);
		//RC522_Pdown();
		
		if(status != MI_OK)
		{
				memset((void *)&card_inf.serialno[0], 0, 4);
				memset((void *)&card_inf.type[0], 0, 2);
				return status;
		}
	
#if rc522_dubug	
		UART_TTL_SendString((uint8_t*)&card_inf, 6);	
#endif
		
		return status;	
}

/**********************************************************************
**函数原型:  uint8_t RE_Card(uint8_t Sector, uint8_t *card_key)
**入口参数:  Sector：扇区号M1卡：0-15
**返 回 值:  操作成功与否，返回0，成功,返回1失败
**功    能:  读卡  
**说    明:   
************************************************************************/
uint8_t RE_Card(uint8_t Sector, uint8_t *card_key)
{	
		uint8_t BlockNo = 0;
		char status = SE_Card();

		if (status != MI_OK)
		{
				return MI_ERR;
		}
	
		BlockNo = Sector<<2;

		status = PcdAuthState(PICC_AUTHENT1A, BlockNo, card_key, (void *)&card_inf.serialno[0]);
		
		if(status == MI_OK)
		{
				status = RE_Sector(BlockNo, (void *)&card_inf.rblockdata[0]);
				
#if rc522_dubug
				UART_TTL_SendString(&card_inf.rblockdata[0], 48);
#endif
		}		

		if (status != MI_OK)
		{
				return MI_ERR;
		}
		
		return status;			
}

/**********************************************************************
**函数原型:  uint8_t WR_Card(uint8_t BlockNo, uint8_t *Data, uint8_t *card_key)
**入口参数:  BlockNO：块号；Data：待写入数据
**返 回 值:  操作成功与否，返回0，成功,返回1失败
**功    能:  写卡  
**说    明:   
************************************************************************/
uint8_t WR_Card(uint8_t BlockNo, uint8_t *Data, uint8_t *card_key)
{
		char status = SE_Card();
		
		if (status != MI_OK)
		{
				return MI_ERR;
		}

		status = PcdAuthState(PICC_AUTHENT1A, BlockNo, card_key, (void *)&card_inf.serialno[0]);
	
		if(status == MI_OK)
		{
				status = PcdWrite(BlockNo, Data); 
				
				if (status != MI_OK)
				{
						return MI_ERR;
				}
		}
		
		RE_Card(BlockNo/4, card_key);
		
		return status;	
}

/**********************************************************************
**函数原型:  uint8_t RE_Card(uint8_t Sector, uint8_t *card_key)
**入口参数:  Sector：扇区号M1卡：0-15
**返 回 值:  操作成功与否，返回0，成功,返回1失败
**功    能:  读卡  
**说    明:   
************************************************************************/
void FD_Card(void)
{
		SE_Card();
}
