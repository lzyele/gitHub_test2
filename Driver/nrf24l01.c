#include "nrf24l01.h"

#include <stdbool.h>
#include <string.h>

#include "uart.h"


#ifndef enable
#define enable  1
#endif

#ifndef disable
#define disable 0
#endif

// SPI(nRF24L01) commands ,	NRF的SPI命令宏定义，详见NRF功能使用文档
#define NRF_READ_REG  0x00  // Define read command to register
#define NRF_WRITE_REG 0x20  // Define write command to register
#define R_RX_PL_WID		0x60  // Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO.
#define RD_RX_PLOAD 	0x61  // Define RX payload register address
#define WR_TX_PLOAD 	0xA0  // Define TX payload register address
#define FLUSH_TX    	0xE1  // Define flush TX register command
#define FLUSH_RX    	0xE2  // Define flush RX register command
#define REUSE_TX_PL 	0xE3  // Define reuse TX payload register command
#define NOP        	 	0xFF  // Define No Operation, might be used to read status register

// SPI(nRF24L01) registers(addresses) ，NRF24L01 相关寄存器地址的宏定义
#define CONFIG      	0x00  // 'Config' register address
#define EN_AA       	0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   	0x02  // 'Enabled RX addresses' register address
#define SETUP_AW    	0x03  // 'Setup address width' register address
#define SETUP_RETR  	0x04  // 'Setup Auto. Retrans' register address
#define RF_CH       	0x05  // 'RF channel' register address
#define RF_SETUP    	0x06  // 'RF setup' register address
#define STATUS      	0x07  // 'Status' register address
#define OBSERVE_TX  	0x08  // 'Observe TX' register address
#define CD          	0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0  	0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1  	0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2  	0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3  	0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4  	0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5  	0x0F  // 'RX address pipe5' register address
#define TX_ADDR     	0x10  // 'TX address' register address
#define RX_PW_P0    	0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1    	0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2    	0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3    	0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4    	0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5    	0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS 	0x17  // 'FIFO Status Register' register address

#define FEATURE     	0x1D  //lzy_add  Feature Register
#define DYNPD       	0x1C  //lzy_add Enable dynamic payload length

#define MAX_RT      	0x10  //达到最大重发次数中断标志位
#define TX_DS					0x20  //发送完成中断标志位	  // 

#define RX_DR					0x40  //接收到数据中断标志位

#define NRF_CSN_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define NRF_CSN_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_12)		

#define NRF_CE_HIGH()	 GPIO_SetBits(GPIOC, GPIO_Pin_7)
#define NRF_CE_LOW()	 GPIO_ResetBits(GPIOC, GPIO_Pin_7)			   

#define NRF_Read_IRQ() GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6)

// SETUP_AW
#define NRF_REGF_AW					 0
#define NRF_REGF_AW_MASK    (0x3 << NRF_REGF_AW)

// RF_CH
#define NRF_REGF_RF_CH			 0
#define NRF_REGF_RF_CH_MASK (0x7f<<NRF_REGF_RF_CH)

// CONFIG  用来配置第一个寄存器 CONFIG
#define NRF_REGF_PRIM_RX			 		   0
#define NRF_REGF_PWR_UP				 		   1
#define NRF_REGF_CRCO				   		   2
#define NRF_REGF_EN_CRC				 		   3
#define NRF_REGF_MASK_MAX_RT	 		   4
#define NRF_REGF_MASK_TX_DS		 		   5
#define NRF_REGF_MASK_RX_DR		 		   6
#define NRF_REGF_PRIM_RX_MASK			  (1<<NRF_REGF_PRIM_RX)
#define NRF_REGF_PWR_UP_MASK			  (1<<NRF_REGF_PWR_UP)
#define NRF_REGF_CRCO_MASK				  (1<<NRF_REGF_CRCO)
#define NRF_REGF_EN_CRC_MASK        (1<<NRF_REGF_EN_CRC)
#define NRF_REGF_MASK_MAX_RT_MASK	  (1<<NRF_REGF_MASK_MAX_RT)
#define NRF_REGF_MASK_TX_DS_MASK	  (1<<NRF_REGF_MASK_TX_DS)
#define NRF_REGF_MASK_RX_DR_MASK    (1<<NRF_REGF_MASK_RX_DR)

// RF_SETUP
#define NRF_REGF_LNA_HCURR				   0
#define NRF_REGF_RF_PWR						   1
#define NRF_REGF_RF_DR						   3
#define NRF_REGF_PLL_LOCK				     4
#define NRF_REGF_RF_DR_LOW           5
#define NRF_REGF_CONT_WAVE           7
#define NRF_REGF_LNA_HCURRMAST_MASK	(1<<NRF_REGF_LNA_HCURR)
#define NRF_REGF_RF_PWR_MASK				(0x03<<NRF_REGF_RF_PWR)
#define NRF_REGF_RF_DR_MASK					(1<<NRF_REGF_RF_DR)
#define NRF_REGF_PLL_LOCK_MASK			(1<<NRF_REGF_PLL_LOCK)
#define NRF_REGF_RF_DR_LOW_MASK     (1<<NRF_REGF_RF_DR_LOW)
#define NRF_REGF_CONT_WAVE_MASK     (1<<NRF_REGF_CONT_WAVE)

// STATUS
#define NRF_REGF_TX_FULL						 0
#define NRF_REGF_RX_P_NO						 1
#define NRF_REGF_MAX_RT							 4
#define NRF_REGF_TX_DS							 5
#define NRF_REGF_RX_DR							 6
#define NRF_REGF_TX_FULL_MASK	      (1<<NRF_REGF_TX_FULL)
#define NRF_REGF_RX_P_NO_MASK				(0x07<<NRF_REGF_RX_P_NO)
#define NRF_REGF_MAX_RT_MASK				(1<<NRF_REGF_MAX_RT)
#define NRF_REGF_TX_DS_MASK		    	(1<<NRF_REGF_TX_DS)
#define NRF_REGF_RX_DR_MASK		    	(1<<NRF_REGF_RX_DR)

// SETUP_RETR
#define NRF_REGF_ARC								 0
#define NRF_REGF_ARD								 4
#define NRF_REGF_ARC_MASK           (0xf<<NRF_REGF_ARC)
#define NRF_REGF_ARD_MASK           (0xf<<NRF_REGF_ARD)

// FEATURE
#define NRF_REGF_EN_DYN_ACK					 0
#define NRF_REGF_EN_ACK_PAY					 1
#define NRF_REGF_EN_DPL							 2
#define NRF_REGF_EN_DYN_ACK_MASK		(1<<NRF_REGF_EN_DYN_ACK)
#define NRF_REGF_EN_ACK_PAY_MASK		(1<<NRF_REGF_EN_ACK_PAY)
#define NRF_REGF_EN_DPL_MASK			  (1<<NRF_REGF_EN_DPL)

/*设置寄存器FIELD 值*/
#define NRF_GET_REG_FIELD_VALUE(regValue, fieldOffset, mask) \
															((regValue & mask)>>fieldOffset)
#define NRF_SET_REG_FIELD_VALUE(regValue, fieldOffset, mask, fieldValue) \
														  ((regValue&(~mask)) | (fieldValue<<fieldOffset))

// RF_SETUP
#define NRF_REGF_LNA_HCURR						0
#define NRF_REGF_RF_PWR								1
#define NRF_REGF_RF_DR								3
#define NRF_REGF_PLL_LOCK							4
#define NRF_REGF_RF_DR_LOW    				5
#define NRF_REGF_CONT_WAVE      			7

//static uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x11, 0x12, 0x13};
static uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x21, 0x22, 0x23};

NRF_PIPE_ADDR pstAddr;
volatile NRF_RX_STRCUT nrf_rx_inf;

/**********************************************************************
**函数原型:   void EXIT_NRF_Init(void)
**入口参数:   无
**返 回 值:   无
**功    能:   NRF外部中断
**说    明:   
************************************************************************/
static void EXIT_NRF_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStruct; 
  	EXTI_InitTypeDef EXTI_InitStruct;
	  NVIC_InitTypeDef NVIC_InitStruct;
	  
		/* config the extiline(PC6) clock and AFIO clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,    ENABLE);
	  
    /* Configyre P[A|B|C|D|E]0  NIVC  */
		NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPriority = 0x00;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
	
		/* EXTI line gpio config(PC6) */	
		//IRQ->PC6，配置成输入
		GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_6 ;
		GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStruct);	

		/* EXTI line(PC6) mode config */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource6);
		EXTI_InitStruct.EXTI_Line = EXTI_Line6;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿中断
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStruct); 
}

/**********************************************************************
**函数原型:   void EXTI4_15_IRQHandler(void)
**入口参数:   无
**返 回 值:   无
**功    能:   NRF外部中断处理函数
**说    明:   
************************************************************************/
void EXTI4_15_IRQHandler(void)
{
		if(EXTI_GetITStatus(EXTI_Line6) != RESET)
		{			
				nrf_rx_inf.rx_irq_flag = true;
					
				/* Clear the EXTI line 6 pending bit */
				EXTI_ClearITPendingBit(EXTI_Line6);
		}
}

/**********************************************************************
**函数原型:   void NRF_Delay(uint16_t cnt)
**入口参数:   延时时间
**返 回 值:   无
**功    能:   NRF延时
**说    明:   
************************************************************************/
//static void NRF_Delay(uint16_t cnt)  
//{
//		while(cnt--);
//}

/**********************************************************************
**函数原型:   void SPI2_Config(void)
**入口参数:   NULL
**返 回 值:   NULL
**功    能:   SPI2初始化
**说    明:   
************************************************************************/
static void SPI2_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		SPI_InitTypeDef  SPI_InitStructure;

		RCC_AHBPeriphClockCmd (RCC_AHBPeriph_GPIOB, ENABLE); //PORTB时钟使能 
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); //SPI2时钟使能 

		/* SPI pin mappings */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_0);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_0);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_0);
	
		/* MOSI2->PB15, MISO2->PB14, SPISCK2->PB13, */
		GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化GPIOB
	
		/*	CSN2->PB12	*/
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 ;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化GPIOB
		GPIO_SetBits(GPIOB,GPIO_Pin_12); 	     //SPI片选取消 
	
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
		SPI_InitStructure.SPI_Mode      = SPI_Mode_Master; //设置SPI工作模式:设置为主SPI
		SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b; //设置SPI的数据大小:SPI发送接收8位帧结构
		SPI_InitStructure.SPI_CPOL      = SPI_CPOL_Low;		 //串行同步时钟的空闲状态为低电平
		SPI_InitStructure.SPI_CPHA      = SPI_CPHA_1Edge;	 //串行同步时钟的第一个跳变沿（上升或下降）数据被采样
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		       //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //定义波特率预分频的值:波特率预分频值为16
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
		SPI_InitStructure.SPI_CRCPolynomial = 7; 					 //CRC值计算的多项式
		SPI_Init(SPI2, &SPI_InitStructure);  							 //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	
		SPI_RxFIFOThresholdConfig(SPI2,SPI_RxFIFOThreshold_QF);
 
		SPI_Cmd(SPI2, ENABLE); //使能SPI外设		
}

/**********************************************************************
**函数原型:   uint8_t SPI2_RW_Byte(uint8_t TxData)
**入口参数:   待发送数据
**返 回 值:   接收数据
**功    能:   SPI2读写一个字节
**说    明:   
************************************************************************/
static uint8_t SPI2_RW_Byte(uint8_t TxData)
{					 	
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{

		}
	
		SPI_SendData8(SPI2, TxData); //通过外设SPIx发送一个数据

		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
		{

		}
		return SPI_ReceiveData8(SPI2); //返回通过SPIx最近接收的数据				 	    
}

/**********************************************************************
**函数原型:   void SPI_NRF_Init(void)
**入口参数:   NULL
**返 回 值:   NULL
**功    能:   NRF24L01 SPI初始化
**说    明:   
************************************************************************/
void SPI_NRF_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
		//CE->PC7,配置成输出； 
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7 ;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_7);
		
		SPI2_Config(); //初始化SPI
}

/**********************************************************************
**函数原型:   uint8_t SPI_NRF_WriteReg(uint8_t reg, uint8_t dat)
**入口参数:   寄存器地址,写入数据
**返 回 值:   寄存器状态
**功    能:   NRF24L01 写寄存器
**说    明:   
************************************************************************/
static uint8_t SPI_NRF_WriteReg(uint8_t reg, uint8_t dat)
{
		uint8_t status;
		
		/*置低CSN，使能SPI传输*/
    NRF_CSN_LOW();
				
		/*发送命令及寄存器号 */
		status = SPI2_RW_Byte(reg);
		 
		/*向寄存器写入数据*/
    SPI2_RW_Byte(dat); 
	          
		/*CSN拉高，完成*/	   
  	NRF_CSN_HIGH();	
		
		/*返回状态寄存器的值*/
   	return(status);
}

/**********************************************************************
**函数原型:   uint8_t SPI_NRF_ReadReg(uint8_t reg)
**入口参数:   寄存器地址
**返 回 值:   寄存器状态
**功    能:   NRF24L01 读寄存器
**说    明:   
************************************************************************/
static uint8_t SPI_NRF_ReadReg(uint8_t reg)
{
		uint8_t reg_val;

		/*置低CSN，使能SPI传输*/
		NRF_CSN_LOW();
				
  	/*发送寄存器号*/
		SPI2_RW_Byte(reg); 

		/*读取寄存器的值 */
		reg_val = SPI2_RW_Byte(NOP);
	            
   	/*CSN拉高，完成*/
		NRF_CSN_HIGH();		
   	
		return reg_val;
}	

/**********************************************************************
**函数原型:   uint8_t SPI_NRF_ReadBuf(uint8_t reg, uint8_t bytes, uint8_t *pBuf)
**入口参数:   寄存器地址,读取字节数,读取数据
**返 回 值:   寄存器状态
**功    能:   NRF24L01 读数据
**说    明:   
************************************************************************/
static uint8_t SPI_NRF_ReadBuf(uint8_t reg, uint8_t bytes, uint8_t *pBuf)
{
		uint8_t status, byte_cnt;
	
		/*置低CSN，使能SPI传输*/
		NRF_CSN_LOW();
		
		/*发送寄存器号*/		
		status = SPI2_RW_Byte(reg); 

		/*读取缓冲区数据*/
		for(byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		{
				pBuf[byte_cnt] = SPI2_RW_Byte(NOP); //从NRF24L01读取数据	 
		}

		/*CSN拉高，完成*/
		NRF_CSN_HIGH();	
		
		return status; //返回寄存器状态值
}

/**********************************************************************
**函数原型:   uint8_t SPI_NRF_WriteBuf(uint8_t reg, uint8_t bytes, uint8_t *pBuf)
**入口参数:   寄存器地址,读取字节数,写入数据
**返 回 值:   寄存器状态
**功    能:   NRF24L01 写数据
**说    明:   
************************************************************************/
static uint8_t SPI_NRF_WriteBuf(uint8_t reg, uint8_t bytes, uint8_t *pBuf)
{
		uint8_t status, byte_cnt;
	
   	/*置低CSN，使能SPI传输*/
		NRF_CSN_LOW();			

		/*发送寄存器号*/	
  	status = SPI2_RW_Byte(reg); 
 	
  	/*向缓冲区写入数据*/
		for(byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		{
				SPI2_RW_Byte(*pBuf++); //写数据到缓冲区 	 
	  }
		
		/*CSN拉高，完成*/
		NRF_CSN_HIGH();			
  
  	return status; //返回NRF24L01的状态 		
}

/**********************************************************************
**函数原型:   void vNrfSetFreq(NRF_RF_CHANNEL Freq)
**入口参数:   频率
**返 回 值:   无
**功    能:   设置频道
**说    明:   
************************************************************************/
void vNrfSetFreq(NRF_RF_CHANNEL Freq)
{
    Freq &= NRF_REGF_RF_CH_MASK; 
    SPI_NRF_WriteReg(NRF_WRITE_REG+RF_CH, Freq); //频率寄存器
}

/**********************************************************************
**函数原型:   void vNrfSetTranPower(NRF_TRAN_POWER power)
**入口参数:   功率
**返 回 值:   无
**功    能:   设置功率
**说    明:   
************************************************************************/
void vNrfSetTranPower(NRF_TRAN_POWER power)
{
    uint8_t uchReg = 0;
		
    uchReg = SPI_NRF_ReadReg(RF_SETUP);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_PWR, NRF_REGF_RF_PWR_MASK, power); 
    SPI_NRF_WriteReg(NRF_WRITE_REG+RF_SETUP, uchReg);
}

/**********************************************************************
**函数原型:   void vNrfSetTranSpeed(NRF_TRAN_SPEED speed)
**入口参数:   速率
**返 回 值:   无
**功    能:   设置速率
**说    明:   
************************************************************************/
void vNrfSetTranSpeed(NRF_TRAN_SPEED speed)
{
    uint8_t uchReg = 0;
    
    uchReg = SPI_NRF_ReadReg(RF_SETUP);
    if(speed == NRF_TRAN_SPEED_1Mbps)
    {       
        uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_DR_LOW, NRF_REGF_RF_DR_LOW_MASK, 0); 
        uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_DR, NRF_REGF_RF_DR_MASK, 0); 
    }
    else if(speed == NRF_TRAN_SPEED_2Mbps)
    {
        uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_DR_LOW, NRF_REGF_RF_DR_LOW_MASK, 0); 
        uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_DR, NRF_REGF_RF_DR_MASK, 1); 
    }
    else
    {
        uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_DR_LOW, NRF_REGF_RF_DR_LOW_MASK, 1); 
        uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RF_DR, NRF_REGF_RF_DR_MASK, 0); 
    }
    SPI_NRF_WriteReg(NRF_WRITE_REG+RF_SETUP, uchReg);
}

/**********************************************************************
**函数原型:   void vNrfEnableRxPipe(NRF_PIPE_NUM enumPipe, bool uchIsEnable)
**入口参数:   管道;开关
**返 回 值:   无
**功    能:   打开或者关闭某个管道
**说    明:   
************************************************************************/
static void vNrfEnableRxPipe(NRF_PIPE_NUM enumPipe, bool uchIsEnable)
{
    uint8_t uchReg = 0;
    uint8_t uchMask = 0;
    
    uchReg = SPI_NRF_ReadReg(EN_RXADDR);
    uchMask = (1<<enumPipe);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, enumPipe, uchMask, uchIsEnable);  
    SPI_NRF_WriteReg(NRF_WRITE_REG+EN_RXADDR, uchReg);
}

/**********************************************************************
**函数原型:   void vNrfEnablePipeAutoAck(NRF_PIPE_NUM enumPipe, bool uchIsEnable)
**入口参数:   管道;开关
**返 回 值:   无
**功    能:   打开或者关闭某个管道的自动回复
**说    明:   
************************************************************************/
static void vNrfEnablePipeAutoAck(NRF_PIPE_NUM enumPipe, bool uchIsEnable)
{
    uint8_t uchReg = 0;
    uint8_t uchMask = 0;
    
    uchReg = SPI_NRF_ReadReg(EN_AA);
    uchMask = (1<<enumPipe);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, enumPipe, uchMask, uchIsEnable); 
    SPI_NRF_WriteReg(NRF_WRITE_REG+EN_AA, uchReg);
}

/**********************************************************************
**函数原型:   void nNrfSetOperationMode(NRF_OPT_MODE enumMode)
**入口参数:  	工作模式
**返 回 值:   无
**功    能:   设置工作模式
**说    明:   
************************************************************************/
void nNrfSetOperationMode(NRF_OPT_MODE enumMode)
{
    uint8_t uchReg = 0;
    uchReg = SPI_NRF_ReadReg(CONFIG);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_PRIM_RX, NRF_REGF_PRIM_RX_MASK, enumMode);
    SPI_NRF_WriteReg(NRF_WRITE_REG+CONFIG, uchReg);
}

/**********************************************************************
**函数原型:   void vNrfSetCrcPara(bool bIsEnable, NRF_CRC_TYPE crcType)
**入口参数:   开关;CRC模式
**返 回 值:   无
**功    能:   CRC参数设置
**说    明:   
************************************************************************/
/*
EN_CRC 3 1 R/W Enable CRC. Forced high if one of the bits in the
EN_AA is high
CRCO 2 0 R/W CRC encoding scheme
'0' - 1 byte
'1' – 2 bytes
*/
static void vNrfSetCrcPara(bool bIsEnable, NRF_CRC_TYPE crcType)
{
    uint8_t uchReg = 0;
    
    uchReg = SPI_NRF_ReadReg(CONFIG);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_EN_CRC, NRF_REGF_EN_CRC_MASK, bIsEnable);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_CRCO, NRF_REGF_CRCO_MASK, crcType);
    SPI_NRF_WriteReg(NRF_WRITE_REG+CONFIG, uchReg);
}

/**********************************************************************
**函数原型:   void vNrfEnableIrq(bool bEnableRxDR, bool bEnableTxDS, bool bEnbaleMaxRT)
**入口参数:   接收中断开关;
**返 回 值:   无
**功    能:   中断参数设置
**说    明:   
************************************************************************/
static void vNrfEnableIrq(bool bEnableRxDR, bool bEnableTxDS, bool bEnbaleMaxRT)
{
    uint8_t uchReg = 0;
    
    uchReg = SPI_NRF_ReadReg(CONFIG);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_RX_DR,  NRF_REGF_RX_DR_MASK,  !bEnableRxDR);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_TX_DS,  NRF_REGF_TX_DS_MASK,  !bEnableTxDS);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_MAX_RT, NRF_REGF_MAX_RT_MASK, !bEnbaleMaxRT);    
    SPI_NRF_WriteReg(NRF_WRITE_REG+CONFIG, uchReg);    
}

/**********************************************************************
**函数原型:   void vNrfSetRetransmitPara(NRF_RT_DELAY enumDelay, NRF_RT_CONT uchCount)
**入口参数:   延时时间;重发次数
**返 回 值:   无
**功    能:   重发参数设置
**说    明:   
************************************************************************/
static void vNrfSetRetransmitPara(NRF_RT_DELAY enumDelay, NRF_RT_CONT uchCount)
{
    uint8_t uchReg = 0;
    
    uchReg = SPI_NRF_ReadReg(SETUP_RETR);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_ARC, NRF_REGF_ARC_MASK, uchCount); 
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_ARD, NRF_REGF_ARD_MASK, enumDelay);
    SPI_NRF_WriteReg(NRF_WRITE_REG+SETUP_RETR, uchReg);    
}

/**********************************************************************
**函数原型:   void  vNrfSetAddrWidth(NRF_ADDR_WIDTHS enumLenAddr)
**入口参数:   地址位宽
**返 回 值:   无
**功    能:   地址宽度参数设置
**说    明:   
************************************************************************/
static void  vNrfSetAddrWidth(NRF_ADDR_WIDTHS enumLenAddr)
{
    uint8_t uchReg = 0;
    uchReg = SPI_NRF_ReadReg(SETUP_AW);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_AW, NRF_REGF_AW_MASK, enumLenAddr); 
    SPI_NRF_WriteReg(NRF_WRITE_REG+SETUP_AW, uchReg);
}

/**********************************************************************
**函数原型:   void nNrfSetRxPipeAddr(NRF_PIPE_NUM enumPipe, bool uchIsEnable, uint8_t *puchAddr, uint8_t uchLenAddr)
**入口参数:   接收管道;开关;地址;地址长度
**返 回 值:   无
**功    能:   设置接收管道地址
**说    明:   
************************************************************************/
void nNrfSetRxPipeAddr(NRF_PIPE_NUM enumPipe, bool uchIsEnable, uint8_t *puchAddr, uint8_t uchLenAddr)
{
    vNrfEnableRxPipe(enumPipe, uchIsEnable);
    
    if(uchLenAddr != 0 && puchAddr != NULL)
    {	
        SPI_NRF_WriteBuf(NRF_WRITE_REG+(RX_ADDR_P0 + enumPipe), uchLenAddr, puchAddr);
    }
}

/**********************************************************************
**函数原型:   int nNrfSetTxAddr(uint8_t *puchAddr, uint8_t uchLenAddr)
**入口参数:   发送地址;地址长度
**返 回 值:   无
**功    能:   发送地址参数设置
**说    明:   
************************************************************************/
static void nNrfSetTxAddr(uint8_t *puchAddr, uint8_t uchLenAddr)
{
    SPI_NRF_WriteBuf(NRF_WRITE_REG+TX_ADDR, uchLenAddr, puchAddr);
}

/**********************************************************************
**函数原型:   void vNrfEnableDynamicPayloadLen(NRF_PIPE_NUM enumPipe,  uint8_t uchIsEnable)
**入口参数:   管道;开关
**返 回 值:   无
**功    能:   动态接收参数设置
**说    明:   
************************************************************************/
/*Enable dynamic payload length*/
static void vNrfEnableDynamicPayloadLen(NRF_PIPE_NUM enumPipe,  uint8_t uchIsEnable)
{
    uint8_t uchReg = 0;
    uint8_t uchMask = 0; 

    uchReg = SPI_NRF_ReadReg(DYNPD);
    uchMask = (1<<enumPipe);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, enumPipe, uchMask, uchIsEnable); 
    SPI_NRF_WriteReg(NRF_WRITE_REG+DYNPD, uchReg);    
}

/**********************************************************************
**函数原型:   void vNrfEnableFeature( bool bIsEnDynAck,  bool bIsEnAckPay, bool bIsEnDPL)
**入口参数:   
**返 回 值:   
**功    能:   
**说    明:   
************************************************************************/
/*
EN_DPL 2 0 R/W Enables Dynamic Payload Length
EN_ACK_PAYd 1 0 R/W Enables Payload with ACK
EN_DYN_ACK 0 0 R/W Enables the W_TX_PAYLOAD_NOACK command
*/
static void vNrfEnableFeature( bool bIsEnDynAck,  bool bIsEnAckPay, bool bIsEnDPL)
{
    uint8_t uchReg = 0;
    
    uchReg = SPI_NRF_ReadReg(FEATURE);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_EN_DYN_ACK, NRF_REGF_EN_DYN_ACK_MASK, bIsEnDynAck);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_EN_ACK_PAY, NRF_REGF_EN_ACK_PAY_MASK, bIsEnAckPay);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_EN_DPL, NRF_REGF_EN_DPL_MASK, bIsEnDPL);    
    SPI_NRF_WriteReg(NRF_WRITE_REG+FEATURE, uchReg);
}

/**********************************************************************
**函数原型:   void nNrfSetPowerMode(NRF_POWER_MODE enumMode)
**入口参数:   工作模式
**返 回 值:   无
**功    能:   电源开关
**说    明:   
************************************************************************/
static void nNrfSetPowerMode(NRF_POWER_MODE enumMode)
{
    uint8_t uchReg = 0;
    
    uchReg = SPI_NRF_ReadReg(CONFIG);
    uchReg = NRF_SET_REG_FIELD_VALUE(uchReg, NRF_REGF_PWR_UP, NRF_REGF_PWR_UP_MASK, enumMode);
    SPI_NRF_WriteReg(NRF_WRITE_REG+CONFIG, uchReg);
}

/**********************************************************************
**函数原型:   vNrfPresetEnhanceShortBurst
**入口参数:   
**返 回 值:   无
**功    能:   参数设置
**说    明:   
************************************************************************/
static void vNrfPresetEnhanceShortBurst(
								 NRF_OPT_MODE optMode, 
								 NRF_RF_CHANNEL rfCh, 
								 NRF_TRAN_SPEED tranSpeed, 
								 NRF_TRAN_POWER tranPower,
                 NRF_RT_CONT rtCount, 
								 NRF_RT_DELAY rtDelay, 
								 NRF_PIPE_ADDR *pstAddr)
{
    //uint8_t uchRecvAddr[4] = {0x00, 0x00, 0x00, 0x00};
    
		NRF_CE_LOW(); //进入待机模式
		
    // Disable auto ACK on all pipes, except PIPE0
    vNrfEnablePipeAutoAck(NRF_RX_PIPE0, enable);
    vNrfEnablePipeAutoAck(NRF_RX_PIPE1, disable);
    vNrfEnablePipeAutoAck(NRF_RX_PIPE2, disable);
    vNrfEnablePipeAutoAck(NRF_RX_PIPE3, disable);
    vNrfEnablePipeAutoAck(NRF_RX_PIPE4, disable);
    vNrfEnablePipeAutoAck(NRF_RX_PIPE5, disable);   

    // Disable RX addresses, except PIPE0
    vNrfEnableRxPipe(NRF_RX_PIPE0, enable);
    vNrfEnableRxPipe(NRF_RX_PIPE1, disable);
    vNrfEnableRxPipe(NRF_RX_PIPE2, disable);
    vNrfEnableRxPipe(NRF_RX_PIPE3, disable);
    vNrfEnableRxPipe(NRF_RX_PIPE4, disable);
    vNrfEnableRxPipe(NRF_RX_PIPE5, disable);   

    // CONFIG - CRC enable, 2-Bit CRC, RX/TX mode
    // CONFIG - CRC enable, 2-Bit CRC, RX/TX mode, disable MAX_RT_IRQ + TX_DS/RX_DR
    vNrfSetCrcPara(enable, NRF_CRC_2BYTE);
    nNrfSetOperationMode(optMode);
    if(optMode == NRF_PRIM_TX)
    {
       vNrfEnableIrq(disable, enable, enable);
    }
    else
    {
       vNrfEnableIrq(enable, disable, enable);
    }
    // Enable auto retry and delay
    vNrfSetRetransmitPara(rtDelay, rtCount);    

    // Set address width to 5 bytes
    vNrfSetAddrWidth(NRF_ADDR_WIDTHS_3BYTE);
    nNrfSetRxPipeAddr(NRF_RX_PIPE0, enable, pstAddr->uchPipe0Addr, pstAddr->uchLen);
    
#if 0
    nNrfSetRxPipeAddr(NRF_RX_PIPE1, enable, pstAddr->uchPipe1Addr, pstAddr->uchLen);
    nNrfSetRxPipeAddr(NRF_RX_PIPE2, enable, pstAddr->uchPipe2Addr + pstAddr->uchLen-1, 1);
#endif
	
    nNrfSetTxAddr(pstAddr->uchPipe0Addr, pstAddr->uchLen);
    vNrfEnableDynamicPayloadLen(NRF_RX_PIPE0, enable);
	
#if 0
    vNrfEnableDynamicPayloadLen(NRF_RX_PIPE1, enable);
    vNrfEnableDynamicPayloadLen(NRF_RX_PIPE2, enable);
#endif

    // Set RF-channel
    vNrfSetFreq(rfCh);
     
    // Setup Data-Rate to 1MBit and RF power to 0db
    vNrfSetTranPower(tranPower);
    vNrfSetTranSpeed(tranSpeed);

    //enable feature
    #if 0
    vNrfEnableFeature(enable, enable, enable);
    #else
    vNrfEnableFeature(disable, enable, enable);
    #endif
	
    // Power up radio
    nNrfSetPowerMode(NRF_POWER_UP);

		NRF_CE_HIGH(); //进入接收状态
}

/**********************************************************************
**函数原型:   void NRF_RX_Mode(NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower)
**入口参数:   传输速率;发射功率
**返 回 值:   无
**功    能:   配置并进入接收模式
**说    明:   
************************************************************************/
static void NRF_RX_Mode(NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower)
{
		NRF_CE_LOW();	
	
		SPI_NRF_WriteReg(NRF_WRITE_REG+SETUP_AW, 0x01);
	
		SPI_NRF_WriteBuf(NRF_WRITE_REG+RX_ADDR_P0, RX_ADR_WIDTH, RX_ADDRESS); //写RX节点地址

		SPI_NRF_WriteReg(NRF_WRITE_REG+EN_AA, 0x01);    //使能通道0的自动应答    

		SPI_NRF_WriteReg(NRF_WRITE_REG+EN_RXADDR, 0x01);//使能通道0的接收地址    

		SPI_NRF_WriteReg(NRF_WRITE_REG+RF_CH, CHANAL);  //设置RF通信频率    

		SPI_NRF_WriteReg(NRF_WRITE_REG+RX_PW_P0, RX_PLOAD_WIDTH); //选择通道0的有效数据宽度 

		SPI_NRF_WriteReg(NRF_WRITE_REG+RF_SETUP, 0x06); //0x06设置TX发射参数,0db增益, 1Mbps,低噪声增益开启
    
		SPI_NRF_WriteReg(NRF_WRITE_REG+CONFIG, 	0x0f);  //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 

		SPI_NRF_WriteReg(NRF_WRITE_REG+STATUS, 0x0e);
	
		SPI_NRF_WriteReg(NRF_WRITE_REG+DYNPD,   0x01);  //使能动态宽度 
		
		SPI_NRF_WriteReg(NRF_WRITE_REG+FEATURE, 0x04);  //选择通道0的动态宽度 
		
	
    NRF_CE_HIGH(); //CE拉高，进入接收模式
}    

/**********************************************************************
**函数原型:   void NRF_TX_Mode(NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower)
**入口参数:   传输速率;发射功率
**返 回 值:   无
**功    能:   配置发送模式
**说    明:   
************************************************************************/
/*
static void NRF_TX_Mode(NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower)
{  
		NRF_CE_LOW();		

		SPI_NRF_WriteBuf(NRF_WRITE_REG+TX_ADDR, TX_ADR_WIDTH, TX_ADDRESS);    //写TX节点地址 
	
		SPI_NRF_WriteBuf(NRF_WRITE_REG+RX_ADDR_P0, RX_ADR_WIDTH, RX_ADDRESS); //设置RX节点地址,主要为了使能ACK   

		SPI_NRF_WriteReg(NRF_WRITE_REG+EN_AA, 0x01);      //使能通道0的自动应答    

		SPI_NRF_WriteReg(NRF_WRITE_REG+EN_RXADDR, 0x01);  //使能通道0的接收地址  

		SPI_NRF_WriteReg(NRF_WRITE_REG+SETUP_RETR, 0x1a); //设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次

		SPI_NRF_WriteReg(NRF_WRITE_REG+RF_CH, CHANAL);    //设置RF频率通道为CHANAL
	
		//Setup Data-Rate to 1MBit and RF power to 0db

		SPI_NRF_WriteReg(NRF_WRITE_REG+RF_SETUP, 0x0f);   //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
		vNrfSetTranPower(tranPower);
		vNrfSetTranSpeed(tranSpeed);

		SPI_NRF_WriteReg(NRF_WRITE_REG+CONFIG, 0x0e);     //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,发射模式,开启所有中断

		NRF_CE_HIGH(); //CE拉高，进入接收模式
}
*/

/**********************************************************************
**函数原型:   uint8_t NRF_Check(void)
**入口参数:   无
**返 回 值:   SUCCESS/ERROR 连接正常/连接失败
**功    能:   主要用于NRF与MCU是否正常连接
**说    明:   
************************************************************************/
static uint8_t NRF_Check(void)
{
		uint8_t test_buf[5] = {0x55, 0x01, 0x02, 0x03, 0xAA};
		uint8_t read_buf[5] = {0};
		uint8_t i = 0; 
	 
		/*写入5个字节的地址.  */  
		SPI_NRF_WriteBuf(NRF_WRITE_REG+TX_ADDR,5, test_buf);
	
		/*读出写入的地址 */
		SPI_NRF_ReadBuf(TX_ADDR,5, read_buf); 
	 
		/*比较*/               
		for(i=0;i<5;i++)
		{
				if(read_buf[i] != test_buf[i])
						break;
		} 
	       
		if(i==5)
		{
				return SUCCESS; //MCU与NRF成功连接 
		}
		else
		{
				return ERROR;   //MCU与NRF不正常连接
		}
}

/**********************************************************************
**函数原型:   uint8_t NRF_Tx_Dat(uint8_t *txbuf)
**入口参数:   存储了将要发送的数据的数组
**返 回 值:   发送结果，成功返回TXDS,失败返回MAXRT或ERROR
**功    能:   用于向NRF的发送缓冲区中写入数据
**说    明:   
************************************************************************/
uint8_t NRF_Tx_Dat(uint8_t *txbuf)
{
		uint8_t state;  

		/*ce为低，进入待机模式1*/
		NRF_CE_LOW();

		/*写数据到TX BUF 最大 32个字节*/						
		SPI_NRF_WriteBuf(WR_TX_PLOAD,TX_PLOAD_WIDTH, txbuf);

    /*CE为高，txbuf非空，发送数据包 */   
		NRF_CE_HIGH();
	  	
	  /*等待发送完成中断 */                            
		//while(NRF_Read_IRQ() != 0); 	
	
		/*读取状态寄存器的值 */                              
		state = SPI_NRF_ReadReg(STATUS);

		/*清除TX_DS或MAX_RT中断标志*/                  
		SPI_NRF_WriteReg(NRF_WRITE_REG+STATUS, state); 	

		SPI_NRF_WriteReg(FLUSH_TX, NOP); //清除TX FIFO寄存器 

		/*判断中断类型*/    
		if(state & MAX_RT) 		 //达到最大重发次数
		{
			 return MAX_RT; 
		}
		else if(state & TX_DS) //发送完成
		{	
				return TX_DS;
		}
		else
		{
			return ERROR;        //其他原因发送失败
		}
} 

/**********************************************************************
**函数原型:   uint8_t NRF_Rx_Dat(volatile NRF_RX_STRCUT *rx_inf)
**入口参数:   用于接收该数据的数组
**返 回 值:   接收结果
**功    能:   用于从NRF的接收缓冲区中读出数据
**说    明:   
************************************************************************/
uint8_t NRF_Rx_Dat(volatile NRF_RX_STRCUT *rx_inf)
{
		uint8_t state; 
		
		/*等待接收中断*/
		//while(NRF_Read_IRQ() != 0); 
	
		/*读取status寄存器的值  */               
		state = SPI_NRF_ReadReg(STATUS);
	 
		/* 清除中断标志*/      
		SPI_NRF_WriteReg(NRF_WRITE_REG+STATUS, state);
	
		/*判断是否接收到数据*/
		if(state & RX_DR) //接收到数据
		{
				rx_inf->rx_len = SPI_NRF_ReadReg(R_RX_PL_WID);
				SPI_NRF_ReadBuf(RD_RX_PLOAD, rx_inf->rx_len, (void*)&rx_inf->rx_data[0]); //读取数据
		
				SPI_NRF_WriteReg(FLUSH_RX, NOP); //清除RX FIFO寄存器
				return RX_DR; 
		}
		else 
		{
				return ERROR; //没收到任何数据
		}
}

/**********************************************************************
**函数原型:   void nNrfInit(NRF_OPT_MODE optMode, NRF_RF_CHANNEL rfCh, NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower)
**入口参数:   工作模式;通信频道;传输速率;发射功率
**返 回 值:   无
**功    能:   24L01设备的初始化
**说    明:   
************************************************************************/
void nNrfInit(NRF_OPT_MODE optMode, NRF_RF_CHANNEL rfCh, NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower)
{
		SPI_NRF_Init();	
		EXIT_NRF_Init();
		
		//验证能否正常读写
		while(NRF_Check() != SUCCESS)
		{
		
		}
	
		if(optMode == NRF_PRIM_RX)
		{	
				memcpy(pstAddr.uchPipe0Addr, RX_ADDRESS, RX_ADR_WIDTH);
				pstAddr.uchLen = RX_ADR_WIDTH;
				
				vNrfPresetEnhanceShortBurst(optMode, 
																		rfCh, 
																		tranSpeed, 
																		tranPower,
																		NRF_RT_DISABLE, 
																		NRF_RT_DELAY_250, 
																		&pstAddr);

				SPI_NRF_WriteReg(FLUSH_RX, NOP); //清除RX FIFO寄存器
				SPI_NRF_WriteReg(FLUSH_TX, NOP); //清除TX FIFO寄存器
				SPI_NRF_WriteReg(NRF_WRITE_REG+STATUS, 0x70); //清除STATUS寄存器 清楚相应的中断位
		}
}

/**********************************************************************
**函数原型:   void nNrf_test(void)
**入口参数:   无
**返 回 值:   无
**功    能:   2.4GHz测试
**说    明:   
************************************************************************/
void nNrf_test(void)
{	
		SPI_NRF_Init();	
		EXIT_NRF_Init();
		
		//验证能否正常读写
		while(NRF_Check() != SUCCESS)
		{
		
		}
		
#if 0
		memcpy(pstAddr.uchPipe0Addr, RX_ADDRESS, RX_ADR_WIDTH);
		pstAddr.uchLen = RX_ADR_WIDTH;
		
		vNrfPresetEnhanceShortBurst(NRF_PRIM_RX, 
																NRF_RF_CH_1, 
																NRF_TRAN_SPEED_1Mbps, 
																NRF_TRAN_POWER_0dbm,
																NRF_RT_DISABLE, 
																NRF_RT_DELAY_250, 
																&pstAddr);
#else		
		NRF_RX_Mode(NRF_TRAN_SPEED_1Mbps, NRF_TRAN_POWER_0dbm);		
#endif		
		
		//NRF_Rx_Dat(nrf_rx_inf.rx_data);
}
