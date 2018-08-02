#ifndef __NRF24L01_H
#define	__NRF24L01_H

#include "stm32f0xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

#define TX_ADR_WIDTH 		3  	//发射地址宽度
#define TX_PLOAD_WIDTH  32  //发射数据通道有效数据宽度0~32Byte 

#define RX_ADR_WIDTH    3
#define RX_PLOAD_WIDTH  32

#define CHANAL 					97	//频道选择 
	
///////////////////////////////////////////////////////////////
/*PIPE定义*/
typedef enum 
{
    NRF_RX_PIPE0 = 0,
    NRF_RX_PIPE1 = 1,
    NRF_RX_PIPE2 = 2,
    NRF_RX_PIPE3 = 3,
    NRF_RX_PIPE4 = 4,
    NRF_RX_PIPE5 = 5
}NRF_PIPE_NUM;

/*Setup of Address Widths*/
typedef enum
{
    NRF_ADDR_LEN_ILLEGAL  = 0,
    NRF_ADDR_WIDTHS_3BYTE = 1,
    NRF_ADDR_WIDTHS_4BYTE = 2,
    NRF_ADDR_WIDTHS_5BYTE = 3
}NRF_ADDR_WIDTHS;


/*PIPE ADDR*/
typedef enum
{
    NRF_RX_PIPE0_ADDR,
    NRF_RX_PIPE1_ADDR,
    NRF_RX_PIPE2_ADDR,
    NRF_RX_PIPE3_ADDR,
    NRF_RX_PIPE4_ADDR,
    NRF_RX_PIPE5_ADDR,
    NRF_TX_PIPE_ADDR
}NRF_RX_PIPE_STATIC_ADDR;

typedef struct
{
    unsigned char uchLen;
    unsigned char uchPipe0Addr[5];/*pipe 0*/
    unsigned char uchPipe1Addr[5];/*pipe 1*/
    unsigned char uchPipe2Addr[5];/*pipe 2*/    
}NRF_PIPE_ADDR;

typedef enum
{
    NRF_PRIM_TX = 0,
    NRF_PRIM_RX = 1
}NRF_OPT_MODE;

typedef enum
{
    NRF_POWER_DOWN = 0,
    NRF_POWER_UP = 1
}NRF_POWER_MODE;

/**
 * Maximum payload size the NRF is able to send in bytes.
 */
#define NRF_MAX_PAYLOAD		32
/**
 * Retry delay for auto retransmit 
 */
typedef enum
{
    NRF_RT_DELAY_250 = 0,/*250us*/
    NRF_RT_DELAY_500 = 1,
    NRF_RT_DELAY_750 = 2,
    NRF_RT_DELAY_1000 = 3,
    NRF_RT_DELAY_1250 = 4,
    NRF_RT_DELAY_1500 = 5,
    NRF_RT_DELAY_1750 = 6,
    NRF_RT_DELAY_2000 = 7,
    NRF_RT_DELAY_2250 = 8,
    NRF_RT_DELAY_2500 = 9,
    NRF_RT_DELAY_2750 = 10,
    NRF_RT_DELAY_3000 = 11,
    NRF_RT_DELAY_3250 = 12,
    NRF_RT_DELAY_3500 = 13,
    NRF_RT_DELAY_3750 = 14,
    NRF_RT_DELAY_4000 = 15
}NRF_RT_DELAY;

/*
0000’ –Re-Transmit disabled
‘0001’ – Up to 1 Re-Transmit on fail of AA
……
‘1111’ – Up to 15 Re-Transmit on fail of AA
*/
typedef enum
{
    NRF_RT_DISABLE = 0,
    NRF_RT_COUNT_UP_TO_1 = 1,
    NRF_RT_COUNT_UP_TO_2 = 2,
    NRF_RT_COUNT_UP_TO_3 = 3,
    NRF_RT_COUNT_UP_TO_4 = 4,
    NRF_RT_COUNT_UP_TO_5 = 5,
    NRF_RT_COUNT_UP_TO_6 = 6,
    NRF_RT_COUNT_UP_TO_7 = 7,
    NRF_RT_COUNT_UP_TO_8 = 8,
    NRF_RT_COUNT_UP_TO_9 = 9,
    NRF_RT_COUNT_UP_TO_10 = 10,
    NRF_RT_COUNT_UP_TO_11 = 11,
    NRF_RT_COUNT_UP_TO_12 = 12,
    NRF_RT_COUNT_UP_TO_13 = 13,
    NRF_RT_COUNT_UP_TO_14 = 14,
    NRF_RT_COUNT_UP_TO_15 = 15
}NRF_RT_CONT;

typedef enum
{
    NRF_SUCCESS = 0, /*成功*/
    NRF_ERR_INIT = -1,
    NRF_ERR_MAX_RT = -2,  /*发送超时，没收到ACK确认*/
    NRF_ERR_TX_FULL = -3, /*发送缓冲区满*/
    NRF_ERR_RX_FULL = -4, /*接收缓冲区满*/
    NRF_ERR_RX_EMPTY = -5, /*接收缓冲区空*/
    NRF_ERR_RX_FIFO_DATA = -6, /*接收FIFO数据错误*/  
    NRF_ERR_TIME_OUT = -7  /*超时*/
}NRF_RET_RESULT;

typedef enum
{
    NRF_DEBUG_LEVEL_DISABLE = 0,
    NRF_DEBUG_LEVEL_FALT = 1,
    NRF_DEBUG_LEVEL_ERR = 2,
    NRF_DEBUG_LEVEL_INFO = 3
}NRF_DEBUG_LEVEL;

/*
000：2.497G (缺省)
001：2.494G
010：2.491G 
011：2.488G
100：2.485G
101：2.482G
110：2.479G
111：2.476G
*/
typedef enum
{
    NRF_RF_CH_1 = 97,
    NRF_RF_CH_2 = 94,
    NRF_RF_CH_3 = 91,
    NRF_RF_CH_4 = 88,
    NRF_RF_CH_5 = 85,
    NRF_RF_CH_6 = 82,
    NRF_RF_CH_7 = 79,
    NRF_RF_CH_8 = 76
}NRF_RF_CHANNEL;

/*CRC 校验格式*/
typedef enum
{
    NRF_CRC_1BYTE = 0,
    NRF_CRC_2BYTE = 1
}NRF_CRC_TYPE;


/*
2	发射功率	bTranPower	2	
00:-18dbm
01:-12dbm
10:-6dbm
11:0 dbm(缺省)
*/
typedef enum
{
    NRF_TRAN_POWER_NEGATIVE_18dbm = 0,
    NRF_TRAN_POWER_NEGATIVE_12dbm = 1,
    NRF_TRAN_POWER_NEGATIVE_6dbm = 2,
    NRF_TRAN_POWER_0dbm = 3
}NRF_TRAN_POWER;

/*3	传输速率	bTranSpeed	2
00:1Mbps
01:2Mbps（缺省）
10:250kbps
11:不使用
*/
typedef enum
{
    NRF_TRAN_SPEED_1Mbps = 0,
    NRF_TRAN_SPEED_2Mbps = 1,
    NRF_TRAN_SPEED_250Kbps = 2
}NRF_TRAN_SPEED;

typedef enum
{
    NRF_TX_FIFO_LOAD_NEW_PACK = 0,
    NRF_TX_FIFO_LOAD_LAST_PACK = 1
}NRF_TRAN_FIFO_MODE;

/*三频卡记录定义*/
typedef enum
{
    TRACK_ID_MSG_SND_ENTRY_RECORD   = 1,   /*发送进入记录*/
    TRACK_ID_MSG_SND_LEAVE_RECORD   = 2,   /*发送离开记录*/
    TRACK_ID_MSG_SND_INTEND_ENTRY   = 6,   /*发送将要进入的记录*/
    TRACK_IN_MSG_SND_INTEND_LEAVE   = 5,    /*发送将要离开的记录*/
    TRACK_IN_MSG_SND_POSITION_FRQ1  = 8,   /*发送当前位置频率1*/
    TRACK_IN_MSG_SND_POSITION_FRQ2  = 9    /*频率2发送当前位置*/
}CARD_MSG_DEF;

typedef struct
{
		bool 		rx_irq_flag;
		uint8_t rx_data[RX_PLOAD_WIDTH];
		uint8_t rx_len;
		
}NRF_RX_STRCUT;
extern volatile NRF_RX_STRCUT nrf_rx_inf;

void vNrfSetTranPower(NRF_TRAN_POWER power);
void vNrfSetTranSpeed(NRF_TRAN_SPEED speed);
void vNrfSetFreq(NRF_RF_CHANNEL Freq);
void nNrfSetRxPipeAddr(NRF_PIPE_NUM enumPipe, bool uchIsEnable, uint8_t *puchAddr, uint8_t uchLenAddr);

uint8_t NRF_Tx_Dat(uint8_t *txbuf);
uint8_t NRF_Rx_Dat(volatile NRF_RX_STRCUT *rx_inf);
void nNrfInit(NRF_OPT_MODE optMode, NRF_RF_CHANNEL rfCh, NRF_TRAN_SPEED tranSpeed, NRF_TRAN_POWER tranPower);
void nNrf_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __NRF24L01_H */
