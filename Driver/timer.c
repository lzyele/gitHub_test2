#include "timer.h"
#include "devcfg.h"
#include "io.h"
#include "rf125k.h"

#define  TIM1Clock  (SystemCoreClock >>1)

/**********************************************************************
**函数原型:   void TIM1_Config(uint8_t uchDutyCycle)
**入口参数:   无
**返 回 值:   无
**功    能:   定时器1初始化函数
**说    明:   
************************************************************************/
void TIM1_Config(uint16_t uchDutyCycle)
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		TIM_OCInitTypeDef  			TIM_OCInitStructure;
		GPIO_InitTypeDef 				GPIO_InitStructure;
		NVIC_InitTypeDef 				NVIC_InitStructure;

		uint16_t TimerPeriod = 0;
		uint16_t Channel1Pulse = 0;

		/* GPIOA, GPIOB and GPIOE Clocks enable */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		/* GPIOA Configuration: Channel 1, 2, 3, 4 and Channel 1N as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_2);

	  /* Enable the TIM1 gloabal Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	
		/* TIM1 Configuration ---------------------------------------------------
		Generate 7 PWM signals with 4 different duty cycles:
		TIM1 input clock (TIM1CLK) is set to APB2 clock (PCLK2)    
		=> TIM1CLK = PCLK2 = SystemCoreClock
		TIM1CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
		SystemCoreClock is set to 48 MHz for STM32F0xx devices

		The objective is to generate 7 PWM signal at 17.57 KHz:
		 - TIM1_Period = (SystemCoreClock / 17570) - 1
		The channel 1 and channel 1N duty cycle is set to 50%
		The channel 2 and channel 2N duty cycle is set to 37.5%
		The channel 3 and channel 3N duty cycle is set to 25%
		The channel 4 duty cycle is set to 12.5%
		The Timer pulse is calculated as follows:
		 - ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100

		Note: 
		SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f0xx.c file.
		Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
		function to update SystemCoreClock variable value. Otherwise, any configuration
		based on this variable will be incorrect. 
		----------------------------------------------------------------------- */
		/* Compute the value to be set in ARR regiter to generate signal frequency at 125 Khz */
		if(uchDutyCycle>99 || uchDutyCycle <1) return; //参数不在（1-99）就错误返回
	
		TimerPeriod = (SystemCoreClock / 125000 ) - 1; //4us 定时 AS3933 Tclk是384us，29页和35也有介绍
		/* Compute CCR1 value to generate a duty cycle at 50% for channel 1 and 1N */
		Channel1Pulse = (uint16_t) (((uint32_t) uchDutyCycle * (TimerPeriod - 1)) / 100);
		/* Compute CCR2 value to generate a duty cycle at 37.5%  for channel 2 and 2N */
		//Channel2Pulse = (uint16_t) (((uint32_t) 500 * (TimerPeriod - 1)) / 1000);
		/* Compute CCR3 value to generate a duty cycle at 25%  for channel 3 and 3N */
		//  Channel3Pulse = (uint16_t) (((uint32_t) 25 * (TimerPeriod - 1)) / 100);
		/* Compute CCR4 value to generate a duty cycle at 12.5%  for channel 4 */
		//  Channel4Pulse = (uint16_t) (((uint32_t) 125 * (TimerPeriod- 1)) / 1000);

		/* TIM1 clock enable */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);

		/* Time Base configuration */
		TIM_TimeBaseStructure.TIM_Prescaler = 0 ;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

		TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
		TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
		/* Channel 1, 2,3 and 4 Configuration in PWM mode */
		TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM2;
		TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
		TIM_OCInitStructure.TIM_Pulse        = Channel1Pulse;
		TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_Low;
		TIM_OCInitStructure.TIM_OCNPolarity  = TIM_OCNPolarity_High;
		TIM_OCInitStructure.TIM_OCIdleState  = TIM_OCIdleState_Set;
		TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

		TIM_OC1Init(TIM1, &TIM_OCInitStructure);

		/* TIM1 Main Output Enable */
		TIM_CtrlPWMOutputs(TIM1, ENABLE);
		
		/* TIM1 counter enable */
		TIM_Cmd(TIM1, ENABLE);
}

/**********************************************************************
**函数原型:   void Set_TIM1_Frequency(uint32_t frequency)
**入口参数:   设置频率
**返 回 值:   无
**功    能:   设置定时器1频率
**说    明:   
************************************************************************/
void Set_TIM1_Frequency(uint32_t frequency)
{
		static uint32_t TimerPeriod = 0;
		static uint16_t Channel2Pulse = 0;

		/* Compute the value to be set in ARR regiter to generate signal frequency */
		TimerPeriod = ( TIM1Clock/ frequency ) - 1;
		/* Compute CCR2 value to generate a duty cycle at 37.5%  for channel 2  */
		Channel2Pulse = (uint16_t) (((uint32_t) 500 * (TimerPeriod - 1)) / 1000);

		TIM_SetAutoreload(TIM1, TimerPeriod);
		TIM_SetCompare2(TIM1, Channel2Pulse);
}

/**********************************************************************
**函数原型:   void TIM1_BRK_UP_TRG_COM_IRQHandler(void) 
**入口参数:   无
**返 回 值:   无
**功    能:   定时器1中断函数
**说    明:   
************************************************************************/
void TIM1_BRK_UP_TRG_COM_IRQHandler(void) //TIM1的中断函数
{

		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
		{
				TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
				vPwmIrq();
		}
}

#define TIM3_PRESCALER_VALUE (48-1)
#define TIM3_PERIOD_TIMING   (10000-1) 

/**********************************************************************
**函数原型:   void TIM_INT_Config(void)
**入口参数:   设置频率
**返 回 值:   无
**功    能:   设置定时器1频率
**说    明:   
************************************************************************/
static void TIM3_INT_Config(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;

		/* TIM3 clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

		/*  TIM3 中断嵌套设计*/
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}

/**********************************************************************
**函数原型:   void TIM3_Config(void)
**入口参数:   无
**返 回 值:   无
**功    能:   定时器3初始化
**说    明:   
************************************************************************/
void TIM3_Config(void)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
			
		TIM3_INT_Config();
		
		/* Time 定时器基础设置 */
		TIM_TimeBaseStructure.TIM_Period        = TIM3_PERIOD_TIMING;   //定时周期(自动从装载寄存器ARR的值)
		TIM_TimeBaseStructure.TIM_Prescaler     = TIM3_PRESCALER_VALUE; //预分频值
		TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;   //同上计数模式
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;					//时钟分频因子	
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                

		/* TIM 中断使能 */
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		
		TIM_Cmd(TIM3, ENABLE);
}

/**********************************************************************
**函数原型:   void TIM3_IRQHandler(void)
**入口参数:   无
**返 回 值:   无
**功    能:   定时器3中断函数
**说    明:   
************************************************************************/
void TIM3_IRQHandler(void)
{
		uint32_t unInterval = 60;
		
		//__disable_irq();
		
		if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
							
				unInterval = (uchDevCfgGetTriggerInterval() <<1);
				
				if(0 == (ulSysTick % unInterval))
				{
						g_uchIsTriggerLowSignal = true;
				}
				
				if((g_uchPwm1OptFlg == 0) && (g_uchPwm2OptFlg == 0))
				{    

				}	
				ulSysTick++;
		}
		
		//__enable_irq();
}
