/*****************************************************************************
	许继电气股份有限公司			版权：2008-2015

	本源代码及其相关文档为河南许昌许继电气股份有限公司独家所有，未经明文许
	可不得擅自修改或发布，否则将追究相关的法律责任。

						河南许昌许继股份有限公司
						www.xjgc.com
						(0374) 321 2924
*****************************************************************************/


/******************************************************************************
	项目名称	：  SGE800计量智能终端平台
	文件		：  swin.h
	描述		：  本文件定义了遥信及脉冲检测模块的接口
	版本		：  0.1
	作者		：  路冉冉
	创建日期	：  2010.4
******************************************************************************/
#ifndef _SWIN_H
#define _SWIN_H

#include "sge_core/typedef.h"
#include "sge_core/rtc.h"				//实时时钟时间结构体

/*************************************************
  宏定义
*************************************************/
//脉冲通道号
#define SWIN0       	0		//遥信或脉冲编号0
#define SWIN1       	1		//遥信或脉冲编号1
#define SWIN2       	2		//遥信或脉冲编号2
#define SWIN3       	3		//遥信或脉冲编号3
#define SWIN4       	4		//遥信或脉冲编号4
#define SWIN5       	5		//遥信或脉冲编号5
#define SWIN6       	6		//遥信或脉冲编号6
#define SWIN7       	7		//遥信或脉冲编号7
#define SWIN_DOOR		12		//门节点
#define SWIN_TOP		13		//开表盖
#define SWIN_MID		14		//开中盖
#define SWIN_TAIL		15		//开尾盖

#define PULSE		1		//设置io功能编码-脉冲
#define REMSIG		2		//设置io功能编码-遥信

/*************************************************
  结构类型定义
*************************************************/
typedef struct{
	u8	polar;						//变位极性 1-正跳变，0-负跳变
	st_ymdhmsw_t	time;			//变位时刻
}swin_jump_t;

typedef struct{
	u32	num;
	swin_jump_t	jump[8];			//最大存储8个遥信变位时刻及极性
}swin_time_t;

/*************************************************
  API
*************************************************/
/******************************************************************************
*	函数:	swin_set
*	功能:	设置为遥信或脉冲模式
*	参数:	id				-	开入通道号
			mode			-	具体见头文件配置
*	返回:	0				-	成功
			-ERR_INVAL		-	参数错误
			-ERR_NODEV		-	无此设备或者不需要配置
*	说明:
 ******************************************************************************/
int swin_set(u8 id, u8 mode);
/******************************************************************************
*	函数:	swin_init
*	功能:	脉冲检测模块初始化(需要先配置设备各路的模式，否则报-ERR_CFG)
*	参数:	无
*	返回:	0				-	成功
			-ERR_CFG		-	配置超限,或者没有配置遥信或脉冲模式
			-ERR_BUSY		-	已经打开
			-ERR_SYS		-	系统错误
			-ERR_NOFILE		-	没有此路径
 * 说明:	无
 ******************************************************************************/
int swin_init(void);

#endif  /* _SWIN_H */
