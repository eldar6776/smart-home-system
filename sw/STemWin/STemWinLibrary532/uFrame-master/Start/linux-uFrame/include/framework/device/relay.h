/******************************************************************************
	项目名称	：  SGE800计量智能终端平台
	文件		：  relay.h
	描述		：  本文件定义了继电器s设备的操作接口
	版本		：  0.1
	作者		：  路冉冉
	创建日期	：  2010.12
******************************************************************************/
#ifndef _RELAY_H
#define _RELAY_H

#include "sge_core/typedef.h"
	
/*************************************************
  静态全局变量及宏定义
*************************************************/
//继电器
#define RELAY0		0		//端口0

/*************************************************
  API
*************************************************/
/******************************************************************************
*	函数:	relay_init
*	功能:	继电器模块初始化
*	参数:	无
*	返回:	0				-	成功
			-ERR_CFG		-	配置超限
			-ERR_BUSY		-	已经打开
			-ERR_SYS		-	系统错误
			-ERR_NOFILE		-	没有此路径
 ******************************************************************************/
int relay_init(void);

/******************************************************************************
*	函数:	relay_on
*	功能:	继电器条件动作
*	参数:	id				-	继电器编号
*			delay			-	延时时间，单位为ms，为0表示没有延时
			last			-	持续时间，单位为ms，为0表示没有持续
			period			-	周期时间，单位为ms，为0表示没有周期
*	返回:	0				-	成功
			-ERR_INVAL		-	参数错误
			-ERR_NOINIT		-	没有初始化
			-ERR_NODEV		-	没有此设备
			-ERR_CFG		-	配置出错
			-ERR_SYS		-	系统错误
*	说明:	3个时间全为0时，表示一直动作, 此处时间精度不高，应>20ms.
 ******************************************************************************/
int relay_on(u8 id, u32 delay, u32 last, u32 period);

/******************************************************************************
*	函数:	relay_off
*	功能:	继电器断开
*	参数:	id				-	继电器号
*	返回:	0				-	成功
			-ERR_SYS		-	系统错误
			-ERR_CFG		-	配置IO高低与继电器动作之间的联系出错
*	说明:
 ******************************************************************************/
int relay_off(u8 id);

/******************************************************************************
*	函数:	relay_check
*	功能:	检查继电器状态
*	参数:	id				-	继电器通道号
*	返回:	1				-	动作
			0				-	不动作
			-ERR_TIMEOUT	-	超时
			-ERR_NODEV 		-	无此设备
			-ERR_NOINIT		-	锁没有初始化化
			-ERR_OTHER:		-	其他关于线程互斥锁的错误
			-ERR_SYS		-	系统错误
*	说明:
 ******************************************************************************/
int relay_check(u8 id);

#endif		//_RELAY_H
