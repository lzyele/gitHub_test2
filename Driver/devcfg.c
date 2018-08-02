#include "devCfg.h"
#include "flash.h"
#include "nrf24l01.h"
#include "tool.h"
#include "uart.h"
#include <string.h>

#define DEV_CFG_PRINT_ENT_FUNCTION()  //printf("ENT %s()\r\n",  __FUNCTION__)
#define DEV_CFG_PRINT_EXIT_FUNCTION() //printf("EXIT %s()\r\n", __FUNCTION__)

#define FLG_FLASH_PARA_DATA           0
#define FLG_FLASH_PARA_BACK_UP_DATA   1

ST_DEV_CFG_PARA g_stDevCfgPara;

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSaveParaToFlash(void)
**入口参数:   无
**返 回 值:   配置是否成功
**功    能:   保存参数
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSaveParaToFlash(void)
{
    uint8_t *puchTmp = NULL;
    int nLenTmp 		 = 0;
    uint16_t usCrc   = 0;
    
    /*每次更新记录均记录下来*/
    g_stDevCfgPara.uchUpdataTimes++;

    usCrc = 0;
    nLenTmp = sizeof(ST_DEV_CFG_PARA)-2;
    puchTmp =  (uint8_t *) &g_stDevCfgPara;
    puchTmp+= 2;	/*不计算LRC字段*/

  	usCrc = crc16(usCrc, puchTmp, nLenTmp);
  	g_stDevCfgPara.usCrc = usCrc;

  	nSaveCfgParaToFlash((uint8_t *) &g_stDevCfgPara, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_DATA);
  	nSaveCfgParaToFlash((uint8_t *) &g_stDevCfgPara, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_BACK_UP_DATA);

		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetDefaultPara(void)
**入口参数:   无
**返 回 值:   配置是否成功
**功    能:   设置缺省参数
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetDefaultPara(void)
{    
#if 1
    uint8_t puchAddr[3] = {0x21, 0x22, 0x23};
#else
    uint8_t puchAddr[3] = {0x44, 0x55, 0x66};
#endif
    
    nDevCfgSetTranPower(false, false, NRF_TRAN_POWER_0dbm);  //发射功率,NRF_TRAN_POWER_0dbm:0dbm
    nDevCfgSetTranSpeed(false, false, NRF_TRAN_SPEED_1Mbps); //传输速率,NRF_TRAN_SPEED_1Mbps:1Mbps
    nDevCfgSetRfFreq   (false, false, NRF_RF_CH_1);					 //传输频率,NRF_RF_CH_1:2.497GHz
    
		nDevCfgSetWgMode   (false, WG_MODE_FORBIT_OUTPUT);			 //WG接口定义,WG_MODE_FORBIT_OUTPUT:禁止wg输出
		
    nDevCfgSetUartBaud (false, UART_BAUD_115200);						 //串口波特率,UART_BAUD_115200:115200
    nDevCfgSetUartOutputIdMode(false, OUTPUT_ID_MODE_RS232_OUTPUT_ID); //串口主动输出ID的形式,OUTPUT_ID_MODE_RS232_OUTPUT_ID:使能RS232输出
		
    nDevCfgEnableBeepFlg(false, false); 											  //蜂鸣器,读卡相关
    nDevCfgSetReadCardMode(false, READ_CARD_MODE_STATIC_FIXED); //读卡模式,READ_CARD_MODE_STATIC_FIXED:固定读卡模式
		
    nDevCfgSetRepSndIdInterval(false, REP_SND_ID_INTERVAL_1s); 	//发送ID间隔,REP_SND_ID_INTERVAL_1s:1s
		
    nDevCfgSetDevAddr(false, 0); //设备地址:0
    //nDevCfgSetAckPayload(false, uchLenPayload, puchPayload);

    nDevCfgSetLossValue(false, false, 0); //衰减值:0
		
    nDevCfgGenAckPack(false); //自动回复

		nDevCfgSetDevType(false, CFG_DEV_TYPE_TRI_FREQ); /*设置设类型*///CFG_DEV_TYPE_TRI_FREQ:三频卡
		
    nDevCfgSetRecvAddr(false, 3, puchAddr); //2.4GHz接收地址

		nDevCfgSetTriggerMode         (false, TYPE_SND_CUR_POSITION);//TYPE_SND_CUR_POSITION:触发卡片发送当前位置信息
    nDevCfgSetTriggerLowFreq      (false, 0);  //触发器低频频率:0
    nDevCfgSetTriggerInterval     (false, 30); //主动触发间隔:30
    nDevCfgSetTriggerDistance     (false, 1);	 //触发距离:1
    nDevCfgSetTriggerSubAddr      (false, 0);  //触发地址:0
    nDevCfgSetTriggerActiveFlg    (false, 1);  //主动触发:1
    nDevCfgSetTriggerDuty         (false, 50); //占空比:50
    nDevCfgSetTriggerInOutInterval(false, 2);	 //进出间隔时间:2
    
		nDevCfgSaveParaToFlash();

		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetDevType(bool bIsSaveToFlash, CFG_DEV_TYPE type)
**入口参数:   是否保存FLASH开关;设备类型
**返 回 值:   配置是否成功
**功    能:   设置设备类型
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetDevType(bool bIsSaveToFlash, CFG_DEV_TYPE type)
{
    g_stDevCfgPara.uchDevMode = (uint8_t) type;

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
		
		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetDevType(void)
**入口参数:   无
**返 回 值:   设备类型
**功    能:   获取设备类型
**说    明:   
************************************************************************/
uint8_t uchDevCfgGetDevType(void)
{
    return g_stDevCfgPara.uchDevMode;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgGenAckPack(bool bIsSaveToFlash)
**入口参数:   是否保存FLASH开关
**返 回 值:   配置是否成功
**功    能:   根据防重读时间设置回复的ACK数据包
**说    明:   
************************************************************************/
/*根据防重读时间设置回复的ACK数据包*/
/*设置防重读时间	6	0110xxxx	1	bRepeatSndInterval*/
/*
typedef enum
{
    REP_SND_ID_INTERVAL_5ms = 0,
    REP_SND_ID_INTERVAL_500ms = 1,
    REP_SND_ID_INTERVAL_1s    = 2,
    REP_SND_ID_INTERVAL_3s    = 3,
    REP_SND_ID_INTERVAL_5s    = 4,
    REP_SND_ID_INTERVAL_10s   = 5,
    REP_SND_ID_INTERVAL_15s   = 6,
    REP_SND_ID_INTERVAL_20s   = 7,
    REP_SND_ID_INTERVAL_60S   = 8,
    REP_SND_ID_INTERVAL_180S  = 9,
    REP_SND_ID_INTERVAL_300S  = 10        
}CFG_REP_SND_ID_INTERVAL;
*/
#define CARD_CMD_REP_SND_INTERVAL     0x6
DEV_CFG_STATUS_RET nDevCfgGenAckPack(bool bIsSaveToFlash)
{
		CFG_REP_SND_ID_INTERVAL interval;
		uint8_t uchCmd[2] = {0x00, 0x00};

		interval = nDevCfgGetRepSndIdInterval();

		uchCmd[0] = 0x00;
		uchCmd[1] = (CARD_CMD_REP_SND_INTERVAL<<4)| interval;
    
		nDevCfgSetAckPayload(bIsSaveToFlash, 2, uchCmd);  //mod by tang_shsh 2014-3-4
		
		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   void nDevCfgParaPrintf(void)
**入口参数:   无
**返 回 值:   无
**功    能:   打印配置信息
**说    明:   
************************************************************************/
void nDevCfgParaPrintf(void)
{
    NRF_TRAN_POWER 						tranPower; /*传输功率*/
    NRF_TRAN_SPEED 				  	tranSpeed; /*传输速率*/
    NRF_RF_CHANNEL 						rfChn;
    CFG_WG_MODE 	 				  	cfgWgMode;
    CFG_UART_OUTPUT_ID_MODE 	cfgUartIdMode;
    CFG_UART_BAUD 				  	cfgUartBaud;
    CFG_REP_SND_ID_INTERVAL 	cfgRepSndIdInterval;
    uint16_t usAddr 					= 0;
    uint8_t uchAckLen 				= 0;
    uint8_t uchAckPayload[32] = {0x00, 0x00, 0x00, 0x00};
    int i = 0;
    
    tranPower 					= nDevCfgGetTranPower();
    tranSpeed 					= nDevCfgGetTranSpeed();
    rfChn 							= nDevCfgGetRfFreq();
    cfgWgMode 					= nDevCfgGetWgMode();
    cfgUartBaud 				= nDevCfgGetUartBaud();
    cfgUartIdMode 			= nDevCfgGetUartOutputIdMode();
    cfgRepSndIdInterval = nDevCfgGetRepSndIdInterval();
    usAddr 							= usDevCfgGetDevAddr();
    nDevCfgGetAckPayload(&uchAckLen, uchAckPayload);
    
    printf("tranPower = %d tranSpeed = %d rfChn= %d  cfgWgMode = %d  cfgUartBaud = %d  cfgUartIdMode = %d  cfgRepSndIdInterval = %d  unAddr = %d",
            tranPower, 
            tranSpeed, 
            rfChn, 
            cfgWgMode, 
            cfgUartBaud,
            cfgUartIdMode,
            cfgRepSndIdInterval,
             usAddr);
    
		printf("uchAckPayload(%d) [ ", uchAckLen);
		for(i=0; i<uchAckLen; i++)
		{
				printf("%02X ", uchAckPayload[i]);
		}
		printf("]\r\n");
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgLoadCfgParaFromFlash(void)
**入口参数:   无
**返 回 值:   配置是否成功
**功    能:   读取FLASH配置信息
**说    明:   
************************************************************************/
/*FLASH 有效，读取FLASH中的数据，FLASH数据无效，将默认的参数写到FLASH 数据区中*/
DEV_CFG_STATUS_RET nDevCfgLoadCfgParaFromFlash(void)
{
    int nLenTmp = 0;
    uint8_t *puchTmp =  NULL;
    ST_DEV_CFG_PARA stParaBackUp;
    uint8_t uchIsDataInvalid = 0;
    uint8_t uchIsBackUpInvalid = 0;

    volatile uint16_t usCrc = 0;
    
    if(0 != nReadCfgParaFromFlash((uint8_t *) &g_stDevCfgPara, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_DATA))
    {
        //printf("nReadCfgParaFromFlash Read  g_stDevCfgPara fail \r\n");
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    if(0 != nReadCfgParaFromFlash((uint8_t *) &stParaBackUp, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_BACK_UP_DATA))
    {
        //printf("nReadCfgParaFromFlash Read  stParaBackUp fail \r\n");        
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    usCrc = 0;
    nLenTmp = sizeof(ST_DEV_CFG_PARA)-2;
    puchTmp =  (uint8_t *) &g_stDevCfgPara;
    puchTmp+= 2;/*不计算LRC字段*/
    
    usCrc = crc16(usCrc, puchTmp, nLenTmp);

    if(usCrc != g_stDevCfgPara.usCrc)
    {
        uchIsDataInvalid = 1; 
    }

    usCrc = 0;
    nLenTmp = sizeof(ST_DEV_CFG_PARA)-2;
    puchTmp =  (uint8_t *) &stParaBackUp;
    puchTmp+= 2;/*不计算LRC字段*/

    usCrc = crc16(usCrc, puchTmp, nLenTmp);

    if(usCrc != stParaBackUp.usCrc)
    {
        uchIsBackUpInvalid = 1; 
    }


    if((1 == uchIsDataInvalid) && (1 == uchIsBackUpInvalid))
    {
        //printf("Flash data is Invaild, Load the default para\r\n");
        nDevCfgSetDefaultPara();
        return DEV_CFG_STATUS_SUC;
    }
    else if((1 == uchIsDataInvalid) && (1 != uchIsBackUpInvalid))
    {
        memcpy((uint8_t *) &g_stDevCfgPara, (uint8_t *)&stParaBackUp, sizeof(ST_DEV_CFG_PARA));
        nSaveCfgParaToFlash((uint8_t *) &g_stDevCfgPara, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_DATA);

        //printf("Flash data g_stDevCfgPara is Invaild, Load stParaBackUp para\r\n");

        return DEV_CFG_STATUS_SUC;
    }
    else if((1 != uchIsDataInvalid) && ( 1 == uchIsBackUpInvalid))
    {
        nSaveCfgParaToFlash((uint8_t *) &g_stDevCfgPara, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_BACK_UP_DATA);
        //printf("Flash data stParaBackUp is Invaild, Copy g_stDevCfgPara  para\r\n");
        return DEV_CFG_STATUS_SUC;
    }

    if( 0 != memcmp((uint8_t *) &g_stDevCfgPara, (uint8_t *)&stParaBackUp, sizeof(ST_DEV_CFG_PARA)))
    {
        nSaveCfgParaToFlash((uint8_t *) &g_stDevCfgPara, sizeof(ST_DEV_CFG_PARA), FLG_FLASH_PARA_BACK_UP_DATA);
        //printf("Flash data stParaBackUp is diff with the g_stDevCfgPara, Copy g_stDevCfgPara  para\r\n");
    }
    
    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTranPower(bool bIsSaveToFlash, bool bIsEffectNow, NRF_TRAN_POWER tranPower)
**入口参数:   是否保存FLASH开关;是否生效;发射功率
**返 回 值:   配置成功与否
**功    能:   设置发射功率并保存入FLASH
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTranPower(bool bIsSaveToFlash, bool bIsEffectNow, NRF_TRAN_POWER tranPower)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    //if((NRF_TRAN_POWER_NEGATIVE_18dbm > tranPower) || (NRF_TRAN_POWER_0dbm < tranPower))
		if(NRF_TRAN_POWER_0dbm < tranPower)
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    
    g_stDevCfgPara.unParaRadio = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaRadio, PARA_CFGF_TRAN_POWER, PARA_CFGF_TRAN_POWER_MASK, tranPower);

    if(true == bIsEffectNow)
    {
        vNrfSetTranPower(tranPower);
    }

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
	
		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   NRF_TRAN_POWER nDevCfgGetTranPower(void)
**入口参数:   无
**返 回 值:   发射功率
**功    能:   获取当前发射功率
**说    明:   
************************************************************************/
NRF_TRAN_POWER nDevCfgGetTranPower(void)
{   
    NRF_TRAN_POWER tranPower;

    DEV_CFG_PRINT_ENT_FUNCTION();

    tranPower = (NRF_TRAN_POWER) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaRadio, PARA_CFGF_TRAN_POWER, PARA_CFGF_TRAN_POWER_MASK);
    
    return tranPower;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTranSpeed(bool bIsSaveToFlash, bool bIsEffectNow, NRF_TRAN_SPEED tranSpeed)
**入口参数:   是否保存FLASH开关;是否生效;传输速率
**返 回 值:   配置成功与否
**功    能:   设置传输速率并保存入FLASH
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTranSpeed(bool bIsSaveToFlash, bool bIsEffectNow, NRF_TRAN_SPEED tranSpeed)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    //if((NRF_TRAN_SPEED_1Mbps > tranSpeed) || (NRF_TRAN_SPEED_250Kbps < tranSpeed))
		if(NRF_TRAN_SPEED_250Kbps < tranSpeed)
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    g_stDevCfgPara.unParaRadio = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaRadio, PARA_CFGF_TRAN_SPEED, PARA_CFGF_TRAN_SPEED_MASK, tranSpeed);

    if(true == bIsEffectNow)
    {
        vNrfSetTranSpeed(tranSpeed);
    }

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
    
    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   NRF_TRAN_SPEED nDevCfgGetTranSpeed(void)
**入口参数:   无
**返 回 值:   传输速率
**功    能:   获取当前传输速率
**说    明:   
************************************************************************/
NRF_TRAN_SPEED nDevCfgGetTranSpeed(void)
{
    NRF_TRAN_SPEED tranSpeed;

    DEV_CFG_PRINT_ENT_FUNCTION();
    
    tranSpeed = (NRF_TRAN_SPEED) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaRadio, PARA_CFGF_TRAN_SPEED, PARA_CFGF_TRAN_SPEED_MASK);
    
    return tranSpeed;    
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetRfFreq(bool bIsSaveToFlash, bool bIsEffectNow, NRF_RF_CHANNEL rfChannel)
**入口参数:   是否保存FLASH开关;是否生效;传输频率
**返 回 值:   配置成功与否
**功    能:   设置传输频率并保存入FLASH
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetRfFreq(bool bIsSaveToFlash, bool bIsEffectNow, NRF_RF_CHANNEL rfChannel)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    if((rfChannel > NRF_RF_CH_1) || rfChannel <NRF_RF_CH_8)
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    
    g_stDevCfgPara.unParaRadio = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaRadio, PARA_CFGF_TRAN_FREQ, PARA_CFGF_TRAN_FREQ_MASK, rfChannel);

    if(true == bIsEffectNow)
    {
        vNrfSetFreq(rfChannel);
    }

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
    
    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   NRF_RF_CHANNEL nDevCfgGetRfFreq(void)
**入口参数:   无
**返 回 值:   传输频率
**功    能:   获取当前传输频率
**说    明:   
************************************************************************/
NRF_RF_CHANNEL nDevCfgGetRfFreq(void)
{
    NRF_RF_CHANNEL rfChannel;

    DEV_CFG_PRINT_ENT_FUNCTION();    
    
    rfChannel = (NRF_RF_CHANNEL) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaRadio, PARA_CFGF_TRAN_FREQ, PARA_CFGF_TRAN_FREQ_MASK);
    
		return rfChannel;       
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetDualFreqRecvAddr(void)
**入口参数:   无
**返 回 值:   配置成功与否
**功    能:   设置接收管道地址
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetDualFreqRecvAddr(void)
{
    uint8_t uchAddr[3] = {0x32, 0x03, 0x00};
    uint8_t uchLen 		 = 3;

    nNrfSetRxPipeAddr(NRF_RX_PIPE0, enable, uchAddr, uchLen);    

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetWgMode(bool bIsSaveToFlash, CFG_WG_MODE wgMode)
**入口参数:   是否保存FLASH开关;韦根模式
**返 回 值:   配置成功与否
**功    能:   设置韦根模式
**说    明:   
************************************************************************/
/*
[0:2] WEIGAN格式
   000：wg26
   001: wg34
   110:拨码控制
   111: 禁止韦根输出
[3:4] UART 波特率
   00：115200
   01: 57600
   10: 38400
   11:9600
[5：6] 是否禁止串口主动输出卡号。
  00：主动输出卡号
  01：禁止主动输出卡号
*/
DEV_CFG_STATUS_RET nDevCfgSetWgMode(bool bIsSaveToFlash, CFG_WG_MODE wgMode)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    if((wgMode != WG_MODE_WG_26) && (wgMode != WG_MODE_WG_34) && (wgMode != WG_MODE_EXT_SWITCH) && (wgMode != WG_MODE_FORBIT_OUTPUT))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    
    g_stDevCfgPara.unParaInterface = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaInterface, PARA_CFGF_WG_MODE, PARA_CFGF_WG_MODE_MASK, wgMode);

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
    
    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   CFG_WG_MODE nDevCfgGetWgMode(void)
**入口参数:   无
**返 回 值:   韦根模式
**功    能:   读取韦根模式
**说    明:   
************************************************************************/
CFG_WG_MODE nDevCfgGetWgMode(void)
{
    CFG_WG_MODE wgMode;

    DEV_CFG_PRINT_ENT_FUNCTION();

    wgMode = (CFG_WG_MODE) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaInterface, PARA_CFGF_WG_MODE, PARA_CFGF_WG_MODE_MASK);

    return wgMode;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetUartBaud(bool bIsSaveToFlash, CFG_UART_BAUD baud)
**入口参数:   是否保存FLASH开关;波特率
**返 回 值:   配置成功与否
**功    能:   设置串口波特率
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetUartBaud(bool bIsSaveToFlash, CFG_UART_BAUD baud)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    if((baud != UART_BAUD_115200) && (baud != UART_BAUD_57600) && (baud != UART_BAUD_38400) && (baud != UART_BAUD_9600))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    
    g_stDevCfgPara.unParaInterface = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaInterface, PARA_CFGF_UART_BAUD, PARA_CFGF_UART_BAUD_MASK, baud);

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
 
    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   static CFG_UART_BAUD nDevCfgGetUartBaud(void)
**入口参数:   无
**返 回 值:   波特率
**功    能:   读取串口波特率
**说    明:   
************************************************************************/
CFG_UART_BAUD nDevCfgGetUartBaud(void)
{
    CFG_UART_BAUD baud;

    DEV_CFG_PRINT_ENT_FUNCTION();   

    baud = (CFG_UART_BAUD) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaInterface, PARA_CFGF_UART_BAUD, PARA_CFGF_UART_BAUD_MASK);

    return baud;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetUartOutputIdMode(bool bIsSaveToFlash, CFG_UART_OUTPUT_ID_MODE cfgUartMode)
**入口参数:   是否保存FLASH开关;串口模式
**返 回 值:   配置成功与否
**功    能:   设置串口模式
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetUartOutputIdMode(bool bIsSaveToFlash, CFG_UART_OUTPUT_ID_MODE cfgUartMode)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    g_stDevCfgPara.unParaInterface = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaInterface, PARA_CFGF_FORBIT_UART_OUT, PARA_CFGF_FORBIT_UART_OUT_MASK, cfgUartMode);

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash(); 
    }

    return DEV_CFG_STATUS_SUC;
}


/**********************************************************************
**函数原型:   CFG_UART_BAUD nDevCfgGetUartBaud(void)
**入口参数:   无
**返 回 值:   串口工作模式
**功    能:   读取串口工作模式
**说    明:   
************************************************************************/
CFG_UART_OUTPUT_ID_MODE nDevCfgGetUartOutputIdMode(void)
{
    CFG_UART_OUTPUT_ID_MODE cfgUartMode;
    cfgUartMode = (CFG_UART_OUTPUT_ID_MODE) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaInterface, PARA_CFGF_FORBIT_UART_OUT, PARA_CFGF_FORBIT_UART_OUT_MASK);

    return cfgUartMode;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetRepSndIdInterval(bool bIsSaveToFlash,  CFG_REP_SND_ID_INTERVAL interval)
**入口参数:   是否保存FLASH开关;防重读间隔时间
**返 回 值:   配置成功与否
**功    能:   设置防重读间隔时间
**说    明:   
************************************************************************/
/*
防重读间隔	nParaInterval	4	[0:3]
0000:5ms(连续重读)
0001:500ms
0010:1S
0011：3S
0100：5S
0101：10S
0110： 15S
0111：20s
1000:  60S
1001: 180S
1010: 300S 
[4:31]：保留
*/
DEV_CFG_STATUS_RET nDevCfgSetRepSndIdInterval(bool bIsSaveToFlash,  CFG_REP_SND_ID_INTERVAL interval)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    
    g_stDevCfgPara.unParaInterval = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unParaInterval, 
                                                           PARA_CFGF_REP_SND_ID_INTERVAL, 
                                                           PARA_CFGF_REP_SND_ID_INTERVAL_MASK, 
                                                           interval);
    nDevCfgGenAckPack(bIsSaveToFlash);

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   CFG_REP_SND_ID_INTERVAL nDevCfgGetRepSndIdInterval(void)
**入口参数:   无
**返 回 值:   防重读间隔时间
**功    能:   读取防重读间隔时间
**说    明:   
************************************************************************/
CFG_REP_SND_ID_INTERVAL nDevCfgGetRepSndIdInterval(void)
{
    CFG_REP_SND_ID_INTERVAL repSndIdInterval;

    DEV_CFG_PRINT_ENT_FUNCTION();    

    repSndIdInterval = (CFG_REP_SND_ID_INTERVAL) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unParaInterval, PARA_CFGF_REP_SND_ID_INTERVAL, PARA_CFGF_REP_SND_ID_INTERVAL_MASK);   

    return repSndIdInterval;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetReadCardMode(bool bIsSaveToFlash,  CFG_READ_CARD_MODE cfgReadCardMode)
**入口参数:   是否保存FLASH开关;读卡模式
**返 回 值:   配置成功与否
**功    能:   设置读卡模式
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetReadCardMode(bool bIsSaveToFlash,  CFG_READ_CARD_MODE cfgReadCardMode)
{
//    DEV_CFG_PRINT_ENT_FUNCTION();
//    
//    if((READ_CARD_MODE_STATIC_FIXED > cfgReadCardMode) || (READ_CARD_EXT_TRIGGER_MODE < cfgReadCardMode))
//    {
//        return DEV_CFG_STATUS_ERR_VALUE;
//    }
//    
//    g_stDevCfgPara.unReadCardMode = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unReadCardMode, 
//                                                          PARA_CFGF_READ_CARD_MODE, 
//                                                          PARA_CFGF_READ_CARD_MODE_MASK, 
//                                                          cfgReadCardMode);

//    if(true == bIsSaveToFlash)
//    {
//        nDevCfgSaveParaToFlash();
//    }

//    /*使能设置*/
//    switch(cfgReadCardMode)
//    {
//        case READ_CARD_MODE_STATIC_FIXED:
//            NRF_SWITCH_TO_ACTIVE();
//            break;
//        case READ_CARD_SOFTWARE_CTR:
//            if(true == bDevCfgGetEnableListenFlg())
//            {
//                NRF_SWITCH_TO_ACTIVE();              
//            }
//            else
//            {
//                NRF_SWITCH_TO_STANDBY();
//            }
//            break;
//        case READ_CARD_EXT_TRIGGER_MODE:
//            if(true == EXT_INT_TRIGGER_IS_ACTIVE())
//            {
//                NRF_SWITCH_TO_ACTIVE();              
//            }
//            else
//            {
//                NRF_SWITCH_TO_STANDBY();
//            }
//            break;
//        default:
//            return DEV_CFG_STATUS_ERR_VALUE;
//    }
		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   CFG_READ_CARD_MODE nDevCfgGetReadCardMode(void)
**入口参数:   无
**返 回 值:   读卡模式
**功    能:   读取读卡模式
**说    明:   
************************************************************************/
CFG_READ_CARD_MODE nDevCfgGetReadCardMode(void)
{
    CFG_READ_CARD_MODE cfgReadCardMode;
    
    cfgReadCardMode = (CFG_READ_CARD_MODE) DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unReadCardMode, PARA_CFGF_READ_CARD_MODE, PARA_CFGF_READ_CARD_MODE_MASK);

    return cfgReadCardMode;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgEnableListenFlg(bool bIsSaveToFlash, bool bFlag)
**入口参数:   是否保存FLASH开关;启用读卡
**返 回 值:   配置成功与否
**功    能:   设置是否启用读卡
**说    明:   
************************************************************************/
/*
[0]：启用监听卡片
  0：监听射频信道数据
  1：暂定监听信道数据
*/
DEV_CFG_STATUS_RET nDevCfgEnableListenFlg(bool bIsSaveToFlash, bool bFlag)
{
//    CFG_READ_CARD_MODE cfgReadCardMode;
//    DEV_CFG_PRINT_ENT_FUNCTION();
//    
//    g_stDevCfgPara.unReadCardFlg = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unReadCardFlg, PARA_CFGF_ACTIVE_LISTEN_CHN, PARA_CFGF_ACTIVE_LISTEN_CHN_MASK, bFlag);
//    //nDevCfgSaveParaToFlash(); 

//    cfgReadCardMode = nDevCfgGetReadCardMode();

//    if(READ_CARD_SOFTWARE_CTR == cfgReadCardMode)
//    {
//        if(bFlag == true)
//        {
//            NRF_SWITCH_TO_ACTIVE();
//        }
//        else
//        {
//            NRF_SWITCH_TO_STANDBY();
//        }
//    }

//    if(true == bIsSaveToFlash)
//    {
//        nDevCfgSaveParaToFlash(); 
//    }

		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   bool bDevCfgGetEnableListenFlg(void)
**入口参数:   无
**返 回 值:   是否启用读卡
**功    能:   读取是否启用读卡
**说    明:   
************************************************************************/
bool bDevCfgGetEnableListenFlg(void)
{
    bool bFlag;

    DEV_CFG_PRINT_ENT_FUNCTION();
    bFlag = DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unReadCardFlg, PARA_CFGF_ACTIVE_LISTEN_CHN, PARA_CFGF_ACTIVE_LISTEN_CHN_MASK);

    return bFlag;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgEnableListenFlg(bool bIsSaveToFlash, bool bFlag)
**入口参数:   是否保存FLASH开关;启用蜂鸣器
**返 回 值:   配置成功与否
**功    能:   设置是否启用蜂鸣器
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgEnableBeepFlg(bool bIsSaveToFlash, bool bFlag)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    g_stDevCfgPara.unReadCardFlg = DEV_CFG_SET_PARA_VALUE(g_stDevCfgPara.unReadCardFlg, PARA_CFGF_ACTIVE_BEEP, PARA_CFGF_ACTIVE_BEEP_MASK, bFlag);

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   bool bDevCfgGetEnableBeepFlg(void)
**入口参数:   无
**返 回 值:   是否启用蜂鸣器
**功    能:   读取是否启用蜂鸣器
**说    明:   
************************************************************************/
bool bDevCfgGetEnableBeepFlg(void)
{
    bool bFlag;
    
    bFlag = DEV_CFG_GET_PARA_VALUE(g_stDevCfgPara.unReadCardFlg, PARA_CFGF_ACTIVE_BEEP, PARA_CFGF_ACTIVE_BEEP_MASK);
    
    return bFlag;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetDevAddr(bool bIsSaveToFlash, uint16_t usAddr)
**入口参数:   是否保存FLASH开关;设备地址
**返 回 值:   配置成功与否
**功    能:   设置设备地址
**说    明:   
************************************************************************/
/*设备地址*/
DEV_CFG_STATUS_RET nDevCfgSetDevAddr(bool bIsSaveToFlash, uint16_t usAddr)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    g_stDevCfgPara.usDevAddr = usAddr;

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint16_t usDevCfgGetDevAddr(void)
**入口参数:   无
**返 回 值:   设备地址
**功    能:   读取设备地址
**说    明:   
************************************************************************/
uint16_t usDevCfgGetDevAddr(void)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    return g_stDevCfgPara.usDevAddr;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetLossValue(bool bIsSaveToFlash, bool bIsEffectNow,  uint8_t uchValue)
**入口参数:   是否保存FLASH开关;是否生效;衰减器的值
**返 回 值:   配置成功与否
**功    能:   设置衰减器的值
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetLossValue(bool bIsSaveToFlash, bool bIsEffectNow,  uint8_t uchValue)
{
    DEV_CFG_PRINT_ENT_FUNCTION();

    g_stDevCfgPara.uchLossValue = uchValue;

    if(true == bIsEffectNow)
    {
        //nAttennuationSetLossValue(uchValue);
    }
    
    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }
		
    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetLossValue(void)
**入口参数:   无
**返 回 值:   衰减器的值
**功    能:   读取衰减器的值
**说    明:   
************************************************************************/
uint8_t uchDevCfgGetLossValue(void)
{
    DEV_CFG_PRINT_ENT_FUNCTION();

    return g_stDevCfgPara.uchLossValue;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetLossValue(bool bIsSaveToFlash, bool bIsEffectNow,  uint8_t uchValue)
**入口参数:   是否保存FLASH开关;ACK PayLoad
**返 回 值:   配置成功与否
**功    能:   设置ACK PayLoad
**说    明:   
************************************************************************/
/*
7	确认返回数据数据(ACK PAYLOAD）	nAckData	0~32	预先存储返回给的卡片的数据，可用于设置卡片参数。
*/
DEV_CFG_STATUS_RET nDevCfgSetAckPayload(bool bIsSaveToFlash, uint8_t uchLen, uint8_t *puchPayload)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    if(uchLen > 32 || (NULL == puchPayload))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    g_stDevCfgPara.uchLenAck = uchLen;
    memcpy(g_stDevCfgPara.uchAckPayload, puchPayload, uchLen);

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgGetAckPayload(uint8_t *puchLen, uint8_t *puchPayload)
**入口参数:   长度;数据
**返 回 值:   配置成功与否
**功    能:   读取ACK PayLoad
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgGetAckPayload(uint8_t *puchLen, uint8_t *puchPayload)
{
    DEV_CFG_PRINT_ENT_FUNCTION();

    *puchLen= g_stDevCfgPara.uchLenAck;
    memcpy(puchPayload, g_stDevCfgPara.uchAckPayload, g_stDevCfgPara.uchLenAck);

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetRecvAddr(bool bIsSaveToFlash, uint8_t uchLen, uint8_t *puchAddr)
**入口参数:   是否保存FLASH开关;地址长度;接收地址
**返 回 值:   配置成功与否
**功    能:   设置接收地址
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetRecvAddr(bool bIsSaveToFlash, uint8_t uchLen, uint8_t *puchAddr)
{
    DEV_CFG_PRINT_ENT_FUNCTION();
    
    if((uchLen > 5) || (NULL == puchAddr) || (uchLen == 0))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    g_stDevCfgPara.uchLenRecvAddr = uchLen;
    memcpy(g_stDevCfgPara.uchRecvAddr, puchAddr, uchLen);

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgGetRecvAddr(uint8_t *puchLen, uint8_t *puchAddr)
**入口参数:   长度;数据
**返 回 值:   配置成功与否
**功    能:   读取接收地址
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgGetRecvAddr(uint8_t *puchLen, uint8_t *puchAddr)
{
    *puchLen= g_stDevCfgPara.uchLenRecvAddr;
    memcpy(puchAddr, g_stDevCfgPara.uchRecvAddr, g_stDevCfgPara.uchLenRecvAddr);

		return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerMode(bool bIsSaveToFlash, LOW_FREQ_TIRGGER_TYPE triggerMode)
**入口参数:   是否保存FLASH开关;触发模式
**返 回 值:   配置成功与否
**功    能:   设置触发模式
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerMode(bool bIsSaveToFlash, LOW_FREQ_TIRGGER_TYPE triggerMode)
{
    #if 0
    if((TYPE_SND_CUR_POSITION > triggerMode) || (TYPE_SAMPLE_PONIT_PARKING_MODE < triggerMode))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    #else
    if((TYPE_SND_CUR_POSITION != triggerMode) && 
       (TYPE_SAMPLE_POINT_IN_OUT_MODE   != triggerMode) &&
       (TYPE_SAMPLE_PONIT_PARKING_MODE  != triggerMode) &&
       (TYPE_SAMPLE_POINT_IN_OUT_MODE_2 != triggerMode))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    #endif
    
    g_stDevCfgPara.uchTriggerMode = triggerMode;

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();        
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   LOW_FREQ_TIRGGER_TYPE uchDevCfgGetTriggerMode(void)
**入口参数:   无
**返 回 值:   触发模式
**功    能:   读取触发模式
**说    明:   
************************************************************************/
LOW_FREQ_TIRGGER_TYPE uchDevCfgGetTriggerMode(void)
{
    return (LOW_FREQ_TIRGGER_TYPE) g_stDevCfgPara.uchTriggerMode;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerLowFreq(bool bIsSaveToFlash, uint8_t uchFreq)
**入口参数:   是否保存FLASH开关;触发器低频频率
**返 回 值:   配置成功与否
**功    能:   设置触发器低频频率
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerLowFreq(bool bIsSaveToFlash, uint8_t uchFreq)
{
    if(uchFreq > 3)
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    g_stDevCfgPara.uchTriggerLowFreq = uchFreq;
    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();  
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetTriggerLowFreq(void)
**入口参数:   无
**返 回 值:   触发器低频频率
**功    能:   读取触发器低频频率
**说    明:   
************************************************************************/
uint8_t uchDevCfgGetTriggerLowFreq(void)
{
    return g_stDevCfgPara.uchTriggerLowFreq;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerInterval(bool bIsSaveToFlash, uint8_t uchInterval)
**入口参数:   是否保存FLASH开关;触发周期
**返 回 值:   配置成功与否
**功    能:   设置触发周期
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerInterval(bool bIsSaveToFlash, uint8_t uchInterval)
{
    g_stDevCfgPara.uchTriggerInterval = uchInterval;

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetTriggerInterval(void)
**入口参数:   无
**返 回 值:   触发周期
**功    能:   读取触发周期
**说    明:   
************************************************************************/
uint8_t uchDevCfgGetTriggerInterval(void)
{
    return g_stDevCfgPara.uchTriggerInterval;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerDistance(bool bIsSaveToFlash, uint8_t uchDistance)
**入口参数:   是否保存FLASH开关;触发器距离
**返 回 值:   配置成功与否
**功    能:   设置触发器距离
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerDistance(bool bIsSaveToFlash, uint8_t uchDistance)
{
    if((0 != uchDistance) && (1 != uchDistance))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    
    g_stDevCfgPara.uchTriggerDistance = uchDistance;

    if(true == bIsSaveToFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetTriggerDistance(void)
**入口参数:   无
**返 回 值:   触发器距离
**功    能:   读取触发器距离
**说    明:   
************************************************************************/
uint8_t uchDevCfgGetTriggerDistance(void)
{
    return g_stDevCfgPara.uchTriggerDistance;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerSubAddr(bool bIsSaveFlash, uint8_t uchSubAddr)
**入口参数:   是否保存FLASH开关;触发器子地址
**返 回 值:   配置成功与否
**功    能:   设置触发器子地址
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerSubAddr(bool bIsSaveFlash, uint8_t uchSubAddr)
{
    g_stDevCfgPara.uchTriggerSubAddr = uchSubAddr;

    if(true == bIsSaveFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetTriggerSubAddr(void)
**入口参数:   无
**返 回 值:   触发器子地址
**功    能:   读取触发器子地址
**说    明:   
************************************************************************/
uint8_t uchDevCfgGetTriggerSubAddr(void)
{
    return g_stDevCfgPara.uchTriggerSubAddr;
}


/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerActiveFlg(bool bIsSaveFlash, uint8_t uchFlg)
**入口参数:   是否保存FLASH开关;是否主动触发
**返 回 值:   配置成功与否
**功    能:   设置是否主动触发
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerActiveFlg(bool bIsSaveFlash, uint8_t uchFlg)
{
    if((0 != uchFlg) && (1 != uchFlg))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }
    
    g_stDevCfgPara.uchTriggerIsActive = uchFlg;

    if(true == bIsSaveFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgTriggerActiveFlg(void)
**入口参数:   无
**返 回 值:   是否主动触发
**功    能:   读取是否主动触发
**说    明:   
************************************************************************/
uint8_t uchDevCfgTriggerActiveFlg(void)
{
    return g_stDevCfgPara.uchTriggerIsActive;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerInOutInterval(bool bIsSaveFlash, uint8_t uchInterval)
**入口参数:   是否保存FLASH开关;进出间隔时间
**返 回 值:   配置成功与否
**功    能:   设置进出间隔时间
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerInOutInterval(bool bIsSaveFlash, uint8_t uchInterval)
{
    
    g_stDevCfgPara.uchTriggerIntervalInOut = uchInterval;

    if(true == bIsSaveFlash)
    {
        nDevCfgSaveParaToFlash();        
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgTriggerInOutInterval(void)
**入口参数:   无
**返 回 值:   进出间隔时间
**功    能:   读取进出间隔时间
**说    明:   
************************************************************************/
uint8_t uchDevCfgTriggerInOutInterval(void)
{
    return g_stDevCfgPara.uchTriggerIntervalInOut;
}

/**********************************************************************
**函数原型:   DEV_CFG_STATUS_RET nDevCfgSetTriggerDuty(bool bIsSaveFlash, uint8_t uchDuty)
**入口参数:   是否保存FLASH开关;低频触发占空比
**返 回 值:   配置成功与否
**功    能:   设置低频触发占空比
**说    明:   
************************************************************************/
DEV_CFG_STATUS_RET nDevCfgSetTriggerDuty(bool bIsSaveFlash, uint8_t uchDuty)
{
    if(((uchDuty & 0x0f) == 0) || (((uchDuty & 0xf0)>>4) == 0))
    {
        return DEV_CFG_STATUS_ERR_VALUE;
    }

    g_stDevCfgPara.uchTriggerSigDuty = uchDuty;

    if(true == bIsSaveFlash)
    {
        nDevCfgSaveParaToFlash();
    }

    return DEV_CFG_STATUS_SUC;
}

/**********************************************************************
**函数原型:   uint8_t uchDevCfgGetTriggerDuty(void)
**入口参数:   无
**返 回 值:   低频触发占空比
**功    能:   读取低频触发占空比
**说    明:   
************************************************************************/
/*低频触发占空比属性*/
uint8_t uchDevCfgGetTriggerDuty(void)
{
    return g_stDevCfgPara.uchTriggerSigDuty;
}
