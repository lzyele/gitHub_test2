#include "uart.h"
#include <stdarg.h>
#include <string.h>

/* Private function prototypes -----------------------------------------------*/

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
/* Private functions ---------------------------------------------------------*/

#define FIFO_NUM 128
static 	uint8_t  Txd2_Fifo[FIFO_NUM];

/**********************************************************************
**函数原型:   void DMA_Config(void)
**入口参数:   无
**返 回 值:   无
**功    能:   DMA1初始化函数
**说    明:   
************************************************************************/	
void DMA_Config(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
		DMA_InitTypeDef DMA_InitStructure;
	
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_Init(&NVIC_InitStructure);

		DMA_DeInit(DMA1_Channel4);
		DMA_InitStructure.DMA_BufferSize = FIFO_NUM; //传输长度
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //传输方向：内存->外设
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //内存-内存：NO
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Txd2_Fifo; //内存地址
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据库宽度：Byte
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存递增：YES
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //循环模式：NO
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USART2->TDR)); //外设地址
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度：Byte
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设递增：NO
		DMA_InitStructure.DMA_Priority = DMA_Priority_High; //优先级：高
		DMA_Init(DMA1_Channel4, &DMA_InitStructure);
}

/**********************************************************************
**函数原型:   void DMA1_Channel4_5_IRQHandler(void
**入口参数:   无
**返 回 值:   无
**功    能:   DMA1中断函数
**说    明:   
************************************************************************/
void DMA1_Channel4_5_IRQHandler(void)
{
		if(DMA_GetITStatus(DMA1_IT_TC4) != RESET)
		{
		    DMA_ClearITPendingBit(DMA1_IT_GL4);		
		}
}

/**********************************************************************
**函数原型:   void USART2_Configuration(void)
**入口参数:   无
**返 回 值:   无
**功    能:   串口初始化函数
**说    明:   
************************************************************************/
static void USART2_Configuration(void)
{  
		GPIO_InitTypeDef  GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef  NVIC_InitStructure;
	
		RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_DMA1, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
						
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);        
		/*
		*  USART2_TX -> PA2 , USART2_RX -> PA3
		*/                                
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;                 
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure);        

		USART_InitStructure.USART_BaudRate 	 = 115200;						  //设置串口波特率
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//设置数据位
		USART_InitStructure.USART_StopBits 	 = USART_StopBits_1;		//设置停止位
		USART_InitStructure.USART_Parity     = USART_Parity_No;			//设置效验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //设置流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //设置工作模式
		USART_Init(USART2, &USART_InitStructure); //配置入结构体

		USART_Cmd(USART2, ENABLE); //使能串口2
		
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //设置中断
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;		//设置优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;   //使能
		NVIC_Init(&NVIC_InitStructure);

		DMA_Config();
}			

/**********************************************************************
**函数原型:   void USART2_SendByte(uint8_t byte)
**入口参数:   发送字节
**返 回 值:   无
**功    能:   串口2发送一个字节
**说    明:   
************************************************************************/
static void USART2_SendByte(uint8_t byte)
{
		USART_SendData(USART2, byte);
		while(!USART_GetFlagStatus(USART2, USART_FLAG_TXE));
}

/**********************************************************************
**函数原型:   void USART2_SendString(uint8_t *string, uint8_t len)
**入口参数:   发送数组，数组长度
**返 回 值:   无
**功    能:   串口2发送一个数组
**说    明:   
************************************************************************/
/*
static void USART2_SendString(uint8_t *string, uint8_t len)
{	
		for(uint8_t i=0; i<len; i++)
		{
				USART2_SendByte(string[i]);
		}
}
*/

/**********************************************************************
**函数原型:  	void USART2_IRQHandler(void)
**入口参数:   无
**返 回 值:   无
**功    能:   串口2中断函数
**说    明:   
************************************************************************/
void USART2_IRQHandler(void)
{
		uint8_t temp = 0;
		
		//接收中断
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
		{
				temp = USART_ReceiveData(USART2); //读完自动清除RXNE
				USART2_SendByte(temp);
		}
}

PUTCHAR_PROTOTYPE 
{
		/* 将Printf内容发往串口 */
		USART_SendData(USART2, (uint8_t)  ch);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		{
		
		}
		return (ch);
}

/**********************************************************************
**函数原型:   void UART_TTL_Init(void)
**入口参数:   无
**返 回 值:   无
**功    能:   TTL串口初始化
**说    明:   
************************************************************************/
void UART_TTL_Init(void)
{
		USART2_Configuration();
}

/**********************************************************************
**函数原型:   void UART_TTL_SendByte(uint8_t byte)
**入口参数:   发送字节
**返 回 值:   无
**功    能:   TTL串口发送一个字节
**说    明:   
************************************************************************/
void UART_TTL_SendByte(uint8_t byte)
{
		USART2_SendByte(byte);
}

/**********************************************************************
**函数原型:   void UART_TTL_SendString(uint8_t *string, uint8_t len)
**入口参数:   发送数组，数组长度
**返 回 值:   无
**功    能:   TTL串口发送一个数组
**说    明:   
************************************************************************/
void UART_TTL_SendString(uint8_t *string, uint8_t len)
{
#if 0		
		USART2_SendString(string, len);
#else		
		DMA_ClearFlag(DMA1_FLAG_GL4);
		DMA_Cmd(DMA1_Channel4, DISABLE);
		USART_DMACmd(USART2,USART_DMAReq_Tx, DISABLE);
  
		DMA1_Channel4->CNDTR = (uint16_t)len;
		memcpy(Txd2_Fifo, string, len);
  
		USART_DMACmd(USART2,USART_DMAReq_Tx, ENABLE);
		USART_ClearFlag(USART2, USART_FLAG_TC);
		DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
		DMA_Cmd(DMA1_Channel4,ENABLE);
#endif	
}

/**********************************************************************
**函数原型:   void USART4_Configuration(void)
**入口参数:   无
**返 回 值:   无
**功    能:   串口初始化函数
**说    明:   
************************************************************************/
static void USART4_Configuration(void)
{  
		GPIO_InitTypeDef  GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef  NVIC_InitStructure;
	
		RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOC,   ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
						
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_0);
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_0);        
		/*
		*  USART4_TX -> PC10 , USART4_RX -> PC11
		*/                                
     
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11;                 
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF; 
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOC, &GPIO_InitStructure); 
		
		USART_InitStructure.USART_BaudRate 	 = 115200;						  //设置串口波特率
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//设置数据位
		USART_InitStructure.USART_StopBits 	 = USART_StopBits_1;		//设置停止位
		USART_InitStructure.USART_Parity     = USART_Parity_No;			//设置效验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //设置流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //设置工作模式
		USART_Init(USART4, &USART_InitStructure); //配置入结构体

		USART_Cmd(USART4, ENABLE); //使能串口4
		
		USART_ITConfig(USART4, USART_IT_RXNE, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = USART3_6_IRQn; //设置中断
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;			//设置优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;   	//使能
		NVIC_Init(&NVIC_InitStructure);		
}			

/**********************************************************************
**函数原型:   void USART4_SendByte(uint8_t byte)
**入口参数:   发送字节
**返 回 值:   无
**功    能:   串口4发送一个字节
**说    明:   
************************************************************************/
static void USART4_SendByte(uint8_t byte)
{
		USART_SendData(USART4, byte);
		while(!USART_GetFlagStatus(USART4, USART_FLAG_TXE));
}

/**********************************************************************
**函数原型:   void USART4_SendString(uint8_t *string, uint8_t len)
**入口参数:   发送数组，数组长度
**返 回 值:   无
**功    能:   串口4发送一个数组
**说    明:   
************************************************************************/
static void USART4_SendString(uint8_t *string, uint8_t len)
{
		for(uint8_t i=0; i<len; i++)
		{
				USART4_SendByte(string[i]);
		}
}

/**********************************************************************
**函数原型:  	void USART3_6_IRQHandler(void)
**入口参数:   无
**返 回 值:   无
**功    能:   串口4中断函数
**说    明:   
************************************************************************/
void USART3_6_IRQHandler(void)
{
		uint8_t temp = 0;
		
		//接收中断
		if(USART_GetITStatus(USART4, USART_IT_RXNE) != RESET)
		{
				temp = USART_ReceiveData(USART4); //读完自动清除RXNE
				USART2_SendByte(temp);
		}
}

#define	ENABLE_485_RE	GPIOB->BRR  = GPIO_Pin_5; /* 接收 */
#define	ENABLE_485_DE	GPIOB->BSRR = GPIO_Pin_5; /* 发送 */

/**********************************************************************
**函数原型:   void RS485_Init(void)
**入口参数:   无
**返 回 值:   无
**功    能:   RS485串口初始化
**说    明:   
************************************************************************/
void RS485_Init(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;   
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 	
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	
		USART4_Configuration();
}

/**********************************************************************
**函数原型:   void RS485_TX_Delay(void) 
**入口参数:   无
**返 回 值:   无
**功    能:   RS485发送延时
**说    明:   
************************************************************************/
static void RS485_TX_Delay(void) 
{
		uint16_t cnt = 1024;
		while(cnt--);
}

/**********************************************************************
**函数原型:   void RS485_SendString(uint8_t *string, uint8_t len)
**入口参数:   发送数组，数组长度
**返 回 值:   无
**功    能:   RS485发送一个数组
**说    明:   
************************************************************************/
void RS485_SendString(uint8_t *string, uint8_t len)
{
    ENABLE_485_DE;

		USART4_SendString(string, len);
		RS485_TX_Delay();
		
		ENABLE_485_RE;
}



