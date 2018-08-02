#ifndef  __DEV_CFG_H_
#define  __DEV_CFG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "nrf24l01.h"


#ifndef false
#define false  0
#endif

#ifndef true
#define true  1
#endif

#ifndef enable
#define enable  1
#endif

#ifndef disable
#define disable  0
#endif

#ifndef bool
typedef uint8_t bool;
#endif


#define FIRE_SOFE_VER "ZCR813-1.1-151127"

/*射频频率设置见 NRF24L01.H*/
/*WG接口定义*/
typedef enum
{
    WG_MODE_WG_26         = 0,  //wg26
    WG_MODE_WG_34         = 1,  //wg34
    WG_MODE_EXT_SWITCH    = 6,  //拨码控制
    WG_MODE_FORBIT_OUTPUT = 7   //禁止wg输出
}CFG_WG_MODE;

/*串口波特率*/
typedef enum
{
    UART_BAUD_115200 = 0,
    UART_BAUD_57600  = 1,
    UART_BAUD_38400  = 2,
    UART_BAUD_9600   = 3
}CFG_UART_BAUD;

/*串口主动输出ID的形式*/
typedef enum
{
    DISABLE_UART_OUTPUT_ID 				 = 0,  //禁止输出ID
    OUTPUT_ID_MODE_RS232_OUTPUT_ID = 1,  //使能RS232输出
    OUTPUT_ID_MODE_RS485_OUTPUT_ID = 2   //使能RS485输出
}CFG_UART_OUTPUT_ID_MODE;

//监听启用/暂停
typedef enum
{
    DISABLE_LISTEN_CHN  = 0,/*停止监听*/
    ENABLE_LISTEN_CHN   = 1 /*启用监听*/
}CFG_LISTEN_CHN;

/*射频频率设置见 NRF24L01.H*/
typedef enum
{
    REP_SND_ID_INTERVAL_125ms = 0,
    REP_SND_ID_INTERVAL_250ms = 1,
    REP_SND_ID_INTERVAL_500ms	= 2,
    REP_SND_ID_INTERVAL_1s    = 3,
    REP_SND_ID_INTERVAL_2s    = 4,
    REP_SND_ID_INTERVAL_3s    = 5,
    REP_SND_ID_INTERVAL_4s    = 6,
    REP_SND_ID_INTERVAL_10s   = 7,  
}CFG_REP_SND_ID_INTERVAL;

/*是否启用监听*/
typedef enum
{
    DISABLE_LISTEN_CHANNEL = 0,
    ENABLE_LISTEN_CHANNEL  = 1
}CFG_LISTEN_CHANNEL;

//射频参数设备参数位偏移
#define PARA_CFGF_TRAN_POWER  0
#define PARA_CFGF_TRAN_SPEED  2
#define PARA_CFGF_TRAN_FREQ   4
#define PARA_CFGF_RADIO_REVS  7

#define PARA_CFGF_TRAN_POWER_MASK  (0x3<<PARA_CFGF_TRAN_POWER)
#define PARA_CFGF_TRAN_SPEED_MASK  (0x3<<PARA_CFGF_TRAN_SPEED)
#define PARA_CFGF_TRAN_FREQ_MASK   (0xff<<PARA_CFGF_TRAN_FREQ)

//接口参数
#define PARA_CFGF_WG_MODE           0
#define PARA_CFGF_UART_BAUD         3
#define PARA_CFGF_FORBIT_UART_OUT   5

#define PARA_CFGF_WG_MODE_MASK         (0x7<<PARA_CFGF_WG_MODE)
#define PARA_CFGF_UART_BAUD_MASK       (0x3<<PARA_CFGF_UART_BAUD)
#define PARA_CFGF_FORBIT_UART_OUT_MASK (0x3<<PARA_CFGF_FORBIT_UART_OUT)

/*
[0]：启用监听卡片
  0：监听射频信道数据
  1：暂定监听信道数据
*/
#define PARA_CFGF_ACTIVE_LISTEN_CHN       0
#define PARA_CFGF_ACTIVE_BEEP             1

#define PARA_CFGF_ACTIVE_LISTEN_CHN_MASK  (1<<PARA_CFGF_ACTIVE_LISTEN_CHN)
#define PARA_CFGF_ACTIVE_BEEP_MASK          (1<< PARA_CFGF_ACTIVE_BEEP)

/*防重读间隔 nParaInterval*/
#define PARA_CFGF_REP_SND_ID_INTERVAL          0
#define PARA_CFGF_REP_SND_ID_INTERVAL_MASK     (0xf<<PARA_CFGF_REP_SND_ID_INTERVAL)

/*设置寄存器FIELD 值*/
#define DEV_CFG_GET_PARA_VALUE(paraValue, fieldOffset, mask)  ((paraValue & mask)>>fieldOffset)
#define DEV_CFG_SET_PARA_VALUE(paraValue, fieldOffset, mask, fieldValue)     ((paraValue&(~mask)) | (fieldValue<<fieldOffset))

/*读卡模式*/
/*固定读卡模式*/
/*软件控制读卡模式*/
/*外部触发读卡模式*/
typedef enum
{
    READ_CARD_MODE_STATIC_FIXED  = 1,    
    READ_CARD_SOFTWARE_CTR = 2,
    READ_CARD_EXT_TRIGGER_MODE = 3
}CFG_READ_CARD_MODE;

#define PARA_CFGF_READ_CARD_MODE        0
#define PARA_CFGF_READ_CARD_MODE_MASK   (0xf <<PARA_CFGF_READ_CARD_MODE)

typedef enum
{
    CFG_DEV_TYPE_TRI_FREQ  = 1, /*三频卡*/
    CFG_DEV_TYPE_DUAL_FREQ = 2  /*双频卡*/
}CFG_DEV_TYPE; /*设备的类型*/

typedef enum
{
    TYPE_SND_CUR_POSITION = 0, 				     /*触发卡片发送当前位置信息*/
    TYPE_SAMPLE_POINT_IN_OUT_MODE = 1, 		 /*触发卡片采集当前位置信息*/
    TYPE_SAMPLE_PONIT_PARKING_MODE = 2, 	 /*停车场模式*/ 
    TYPE_SAMPLE_POINT_IN_OUT_MODE_2 = 0x81 /*触发卡片采集当前位置信息, 长时间停留生成记录*/
}LOW_FREQ_TIRGGER_TYPE;/*触发模式*/

/*参数定义*/
typedef struct
{
    uint16_t usCrc;									 /*校验值*/
    uint8_t uchUpdataTimes;					 /*更新次数*/
    uint8_t uchFill[1];
    uint32_t unParaRadio; 					 /*射频参数*/
    uint32_t unParaInterface; 			 /*接口参数*/
    uint32_t unParaInterval; 				 /*间隔参数*/
    uint32_t unReadCardMode; 				 /*读卡模式*/
    uint32_t unReadCardFlg; 				 /*读卡标记位*/
    uint16_t usDevAddr;      				 /*设备地址*/
    uint8_t uchLossValue;    				 /*衰减器的值*/
    uint8_t uchDevMode;      				 /*设备类型,用来区分是双频卡还是三频卡*/
    uint8_t uchLenRecvAddr;  				 /*接收地址长度*/
    uint8_t uchRecvAddr[5];  				 /*接收地址*/
    uint8_t uchLenAck;    					 /**/
    uint8_t uchAckPayload[32]; 		 	 /**/
    uint8_t uchTriggerMode; 				 /*触发器模式*/
    uint8_t uchTriggerLowFreq; 			 /*触发器低频频率*/
    uint8_t uchTriggerInterval; 		 /*触发周期*/
    uint8_t uchTriggerDistance; 		 /*触发器距离*/
    uint8_t uchTriggerSubAddr; 			 /*触发器子地址*/
    uint8_t uchTriggerIsActive; 		 /*是否主动触发*/
    uint8_t uchTriggerIntervalInOut; /*进出间隔时间*/
    uint8_t uchTriggerSigDuty;       /*低频信号占空比*/
        
/*
    CFG_INDEX_TRIGGER_MODE                = 80, //触发器模式
    CFG_INDEX_TRIGGER_LOW_FREQ_INDEX      = 81, //低频的触发频率
    CFG_INDEX_TRIGGER_LOW_FREQ_DISTANCE   = 82, //覆盖距离
    CFG_INDEX_TRIGGER_SND_INTERVAL        = 84, //触发周期间隔
    CFG_INDEX_TRIGGER_SUB_ADDR            = 86, //触发器接收字地址
    CFG_INDEX_TRIGGER_IS_ACTIVE           = 87  //主动触发or被动触发
*/
}ST_DEV_CFG_PARA, *pST_DEV_CFG_PARA;

typedef enum
{
    DEV_CFG_STATUS_SUC = 0,
    DEV_CFG_STATUS_ERR_VALUE = -1
}DEV_CFG_STATUS_RET;

DEV_CFG_STATUS_RET nDevCfgSaveParaToFlash(void); /*保存设备参数到flash中*/
DEV_CFG_STATUS_RET nDevCfgSetDefaultPara(void);  /*设置缺省参数*/
DEV_CFG_STATUS_RET nDevCfgLoadCfgParaFromFlash(void); /*从FLASH中读取设备的参数*/

void nDevCfgParaPrintf(void);

DEV_CFG_STATUS_RET nDevCfgSetDevType(bool bIsSaveToFlash, CFG_DEV_TYPE type); /*设置设备类型*/
uint8_t uchDevCfgGetDevType(void);
	
DEV_CFG_STATUS_RET nDevCfgGenAckPack(bool bIsSaveToFlash);

DEV_CFG_STATUS_RET nDevCfgSetTranPower(bool bIsSaveToFlash, bool bIsEffectNow, NRF_TRAN_POWER tranPower); /*设置2.4发送功率*/
NRF_TRAN_POWER nDevCfgGetTranPower(void); /*获取传输功率*/
DEV_CFG_STATUS_RET nDevCfgSetTranSpeed(bool bIsSaveToFlash, bool bIsEffectNow, NRF_TRAN_SPEED tranSpeed); /*设置传输速率*/
NRF_TRAN_SPEED nDevCfgGetTranSpeed(void); /*获取传输速率*/
DEV_CFG_STATUS_RET nDevCfgSetRfFreq(bool bIsSaveToFlash, bool bIsEffectNow, NRF_RF_CHANNEL rfChannel);		 /*设置传输频率*/
NRF_RF_CHANNEL nDevCfgGetRfFreq(void); 	 /*获取传输频率*/
DEV_CFG_STATUS_RET nDevCfgSetDualFreqRecvAddr(void); /*设置双频卡的接收地址*/

DEV_CFG_STATUS_RET nDevCfgSetWgMode(bool bIsSaveToFlash, CFG_WG_MODE wgMode);   /*设置WG模式*/
CFG_WG_MODE nDevCfgGetWgMode(void); /*读取WG模式*/

DEV_CFG_STATUS_RET nDevCfgSetUartBaud(bool bIsSaveToFlash, CFG_UART_BAUD baud); /*设置串口波特率*/
CFG_UART_BAUD nDevCfgGetUartBaud(void); /*获取串口波特率*/

DEV_CFG_STATUS_RET nDevCfgSetUartOutputIdMode(bool bIsSaveToFlash, CFG_UART_OUTPUT_ID_MODE cfgUartMode);
CFG_UART_OUTPUT_ID_MODE nDevCfgGetUartOutputIdMode(void);

DEV_CFG_STATUS_RET nDevCfgSetRepSndIdInterval(bool bIsSaveToFlash, CFG_REP_SND_ID_INTERVAL interval); /*设置放重读间隔时间*/
CFG_REP_SND_ID_INTERVAL nDevCfgGetRepSndIdInterval(void); /*获取防重读间隔时间*/

DEV_CFG_STATUS_RET nDevCfgSetReadCardMode(bool bIsSaveToFlash,  CFG_READ_CARD_MODE cfgReadCardMode);
CFG_READ_CARD_MODE nDevCfgGetReadCardMode(void);

DEV_CFG_STATUS_RET nDevCfgEnableListenFlg(bool bIsSaveToFlash, bool bFlag); /*设置是否启用读卡*/

bool bDevCfgGetEnableListenFlg(void); /*读取是否启用读卡标记*/

DEV_CFG_STATUS_RET nDevCfgEnableBeepFlg(bool bIsSaveToFlash, bool bFlag);
bool bDevCfgGetEnableBeepFlg(void);

DEV_CFG_STATUS_RET nDevCfgSetDevAddr(bool bIsSaveToFlash, uint16_t usAddr); /*设置设备地址*/
uint16_t usDevCfgGetDevAddr(void);

DEV_CFG_STATUS_RET nDevCfgSetLossValue(bool bIsSaveToFlash, bool bIsEffectNow,  uint8_t uchValue);
uint8_t uchDevCfgGetLossValue(void);

DEV_CFG_STATUS_RET nDevCfgSetAckPayload(bool bIsSaveToFlash, uint8_t uchLen, uint8_t *puchPayload); /*设置ACK PayLoad*/
DEV_CFG_STATUS_RET nDevCfgGetAckPayload(uint8_t *puchLen, uint8_t *puchPayload); /*读取ACK PayLoad*/

DEV_CFG_STATUS_RET nDevCfgSetRecvAddr(bool bIsSaveToFlash, uint8_t uchLen, uint8_t *puchAddr);
DEV_CFG_STATUS_RET nDevCfgGetRecvAddr(uint8_t *puchLen, uint8_t *puchAddr);

DEV_CFG_STATUS_RET nDevCfgSetTriggerMode(bool bIsSaveToFlash, LOW_FREQ_TIRGGER_TYPE triggerMode);
LOW_FREQ_TIRGGER_TYPE uchDevCfgGetTriggerMode(void);


DEV_CFG_STATUS_RET nDevCfgSetTriggerLowFreq(bool bIsSaveToFlash, uint8_t uchFreq);
uint8_t uchDevCfgGetTriggerLowFreq(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerInterval(bool bIsSaveToFlash, uint8_t uchInterval);
uint8_t uchDevCfgGetTriggerInterval(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerDistance(bool bIsSaveToFlash, uint8_t uchDistance);
uint8_t uchDevCfgGetTriggerDistance(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerSubAddr(bool bIsSaveFlash, uint8_t uchSubAddr);
uint8_t uchDevCfgGetTriggerSubAddr(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerActiveFlg(bool bIsSaveFlash, uint8_t uchFlg);
uint8_t uchDevCfgTriggerActiveFlg(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerDistance(bool bIsSaveToFlash, uint8_t uchDistance);
uint8_t uchDevCfgGetTriggerDistance(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerInOutInterval(bool bIsSaveFlash, uint8_t uchInterval);
uint8_t uchDevCfgTriggerInOutInterval(void);

DEV_CFG_STATUS_RET nDevCfgSetTriggerDuty(bool bIsSaveFlash, uint8_t uchDuty);
uint8_t uchDevCfgGetTriggerDuty(void);

#ifdef __cplusplus
}
#endif

#endif

