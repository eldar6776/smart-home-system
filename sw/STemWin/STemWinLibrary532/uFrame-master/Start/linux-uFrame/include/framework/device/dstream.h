/******************************************************************************
	项目名称	：SGE800计量智能终端业务平台
	文件		：dstream.h
	描述		：本文件定义了数据流设备模块的接口
	版本		：0.1
	作者		：路冉冉
	创建日期	：2010.12
******************************************************************************/
#ifndef _DSTREAM_H
#define _DSTREAM_H

#include "sge_core/typedef.h"
#include "sge_core/comport.h"
	
/*************************************************
  宏定义
*************************************************/
#define DSTREAM_ZB		0		//载波
#define DSTREAM_IRDA	1		//红外
#define DSTREAM_DEBUG	2		//调试
#define DSTREAM_USBD	3		//USB从口
#define DSTREAM_JC		4		//底板
#define DSTREAM_485_1	5		//1#485口
#define DSTREAM_485_2	6		//2#485口
#define DSTREAM_485_3	7		//3#485口

/*************************************************
  结构类型定义
*************************************************/
typedef struct {
/******************************************************************************
*	函数:	open
*	功能:	数据流设备打开函数
*	参数:	cfg				-	串口参数指针
*	返回:	0				-	成功
			-ERR_NOFILE		-	没有此路径
			-ERR_BUSY		-	设备已经打开,可以继续操作
			-ERR_INVAL		- 	参数出错
			-ERR_OTHER		-	创建线程出错
			-ERR_SYS		-	系统错误
*	说明:	可以直接打开8种数据流设备，初次打开需要配置串口参数
 ******************************************************************************/
	int (*open)(comport_config_t *cfg);
/******************************************************************************
*	函数:	config
*	功能:	数据流设备参数配置函数
*	参数:	cfg				-	串口参数指针
*	返回:	0				-	成功
			-ERR_NOFILE		-	没有此路径
			-ERR_BUSY		-	设备忙，已经打开
			-ERR_INVAL		- 	参数出错
			-ERR_SYS		-	系统错误
*	说明:	无
 ******************************************************************************/
	int (*config)(comport_config_t *cfg);
/******************************************************************************
*	函数:	send
*	功能:	数据流设备数据发送函数
*	参数:	buf				-	数据发送指针
*			count			-	数据大小
*	返回:	0				-	成功
			-ERR_NOFILE		-	没有此路径
			-ERR_BUSY		-	设备忙，已经打开
			-ERR_INVAL		- 	参数出错
			-ERR_NOINIT		-	没有打开设备
*	说明:	无
 ******************************************************************************/
	int (*send)(u8 *buf, u32 count);
} dstream_device_t;

extern dstream_device_t dstream_device[];

//备用电池电压操作函数
int mcu_batvolt_open (void);
int mcu_batvolt_check (void);

#endif		//_DSTREAM_H
