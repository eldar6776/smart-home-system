#ifndef _CONFIG_H
#define _CONFIG_H

#include "../include/pinio.h"

//��������
//#define CFG_DEBUG 1

#ifdef CFG_DEBUG
#define CFG_DEBUG_DIN
#define CFG_DEBUG_PULSE
#define CFG_DEBUG_POWERCHECK
#define CFG_DEBUG_ADC
#define CFG_DEBUG_RTC
#define CFG_DEBUG_TIMER
#define CFG_DEBUG_GPIO

#define CFG_DEBUG_COMPORT
#define CFG_DEBUG_NET
#define CFG_DEBUG_GSERIAL

#define CFG_DEBUG_THREAD
#define CFG_DEBUG_MSG
#endif

//ADת��ģ������
#define CFG_ADC_MODULE				1			//�Ƿ�����
#define CFG_ADC_NUM					8			//��ǰADCͨ������
#define CFG_ADC_0					0			//0��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_1					1			//1��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_2					2			//2��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_3					3			//3��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_4					4			//4��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_5					5			//5��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_6					6			//6��ͨ����Ӧ��Ӳ��ͨ��
#define CFG_ADC_7					7			//7��ͨ����Ӧ��Ӳ��ͨ��

//��ʱ��ģ������
#define CFG_TIMER_MODULE			1			//�Ƿ�����
#define CFG_TIMER_PWM_S				1			//�������ʱ��Դ���߾���
#define CFG_TIMER_MEASURE_S			2			//Ƶ�ʲ���ʱ��Դ���;���

//RTCģ������
#define CFG_RTC_MODULE				1				//�Ƿ�����

//IO����ģ������
#define CFG_GPIO_MODULE				1				//�Ƿ�����

//�����ϵ���ģ������
#define CFG_POWERCHECK_MODULE			1			//�Ƿ�����
#define CFG_POWERCHECK_IO				PIN_PC12	//�Ƿ�����

//����ģ��
#define CFG_NET_MODULE              1
#define CFG_NET_SERVPORT            3333                 //�˿ں�
#define CFG_NET_BACKLOG             8                    //����
#define CFG_NET_MAXSIZE             8                    //�������ͻ�����--���Ϊ64

//��������	
#define CFG_COMPORT_MODULE		1			//�Ƿ�����

//USB�ӿ����⴮������	
#define CFG_GSERIAL_MODULE		1			//�Ƿ�����

//�̲߳���ģ������
#define CFG_THREAD_MODULE			1				//�Ƿ�����
#define CFG_THREAD_MAX				32				//���֧���߳���

//��Ϣ����ģ������
#define CFG_MSG_MODULE				1				//�Ƿ�����
#define CFG_MSG_MAX					32				//���֧����Ϣ������
#define CFG_MSG_SIZE				32				//��Ϣ�����������ɵ���Ϣ��


#endif /* _CONFIG_H */
