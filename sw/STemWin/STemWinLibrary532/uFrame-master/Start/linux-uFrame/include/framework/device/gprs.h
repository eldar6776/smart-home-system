/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ҵ��ƽ̨
	�ļ�		��  gprs.h
	����		��  ���ļ�������GPRS�豸�Ľӿ�
	�汾		��  0.1
	����		��  ��迡
	��������	��  2010.12
******************************************************************************/
#ifndef _GPRS_H
#define _GPRS_H

#include "sge_core/typedef.h"
	
/*************************************************
  �궨��
*************************************************/
//GPRSģ�����ͱ��
#define GPRS_ME3000		0		//ME3000

//����ģʽ����
#define GPRS_MODE_TCP_CLIENT	0
#define GPRS_MODE_TCP_SERVER	1
#define GPRS_MODE_UDP_CLIENT	2
#define GPRS_MODE_UDP_SERVER	3

//��ȡ�������Ͷ���
#define GPRS_FLOW_ALL		0
#define GPRS_FLOW_SEND		1
#define GPRS_FLOW_RECV		2

/*************************************************
  �ṹ���Ͷ���
*************************************************/
typedef struct {
	int (*open)(void);
	int (*connect)(u8 mode, u8 *ip, u16 port);
	int (*disconnect)(int cd);
	int (*senddata)(int cd, u8 *buf, u32 count);
	int (*sendsms)(int cd, u8 *buf, u32 count);
	int (*turnon)(void);
	int (*turnoff)(void);
	int (*getflow)(u8 type);
	int (*getstat)(int cd);
	int (*getsi)(void);
} gprs_device_t;

extern gprs_device_t gprs_device[];

#endif		//_GPRS_H
