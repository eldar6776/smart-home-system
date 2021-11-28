/*****************************************************************************
	��̵����ɷ����޹�˾			��Ȩ��2008-2015

	��Դ���뼰������ĵ�Ϊ���������̵����ɷ����޹�˾�������У�δ��������
	�ɲ��������޸Ļ򷢲�������׷����صķ������Ρ�

						���������̹ɷ����޹�˾
						www.xjgc.com
						(0374) 321 2924
*****************************************************************************/


/******************************************************************************
	��Ŀ����	��SGE800���������ն�ƽ̨
	�ļ�		��debug.h
	����		�����ļ�������ƽ̨���Է���
	�汾		��0.1
	����		����迡
	��������	��2009.12
******************************************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H

//������ģ���ӡ����
#ifdef CFG_DEBUG_DIN
#define	DINPRINTF(x...)			printf("DIN:" x)
#else
#define DINPRINTF(x...)
#endif

//������ģ���ӡ����
#ifdef CFG_DEBUG_PULSE
#define	PULSEPRINTF(x...)		printf("PULSE:" x)
#else
#define PULSEPRINTF(x...)
#endif

//������ģ���ӡ����
#ifdef CFG_DEBUG_POWERCHECK
#define	POWERCHECKPRINTF(x...)	printf("POWERCHECK:" x)
#else
#define POWERCHECKPRINTF(x...)
#endif

//ADת��ģ���ӡ����
#ifdef CFG_DEBUG_ADC
#define	ADCPRINTF(x...)			printf("ADC:" x)
#else
#define ADCPRINTF(x...)
#endif

//RTCģ���ӡ����
#ifdef CFG_DEBUG_RTC
#define	RTCPRINTF(x...)			printf("RTC:" x)
#else
#define RTCPRINTF(x...)
#endif

//��ʱ��ģ���ӡ����
#ifdef CFG_DEBUG_TIMER
#define	TIMERPRINTF(x...)		printf("TIMER:" x)
#else
#define TIMERPRINTF(x...)
#endif

//IOģ���ӡ����
#ifdef CFG_DEBUG_GPIO
#define	GPIOPRINTF(x...)		printf("GPIO:" x)
#else
#define GPIOPRINTF(x...)
#endif

//����ģ���ӡ����
#ifdef CFG_DEBUG_COMPORT
#define	COMPORTPRINTF(x...)		printf("COMPORT:" x)
#else
#define COMPORTPRINTF(x...)
#endif

//����ģ���ӡ����
#ifdef CFG_DEBUG_NET
#define	NETPRINTF(x...)			printf("NET:" x)
#else
#define NETPRINTF(x...)
#endif

//USB�ӿ����⴮��ģ���ӡ����
#ifdef CFG_DEBUG_GSERIAL
#define	USBDPRINTF(x...)		printf("GSERIAL:" x)
#else
#define USBDPRINTF(x...)
#endif

//�߳�ģ���ӡ����
#ifdef CFG_DEBUG_THREAD
#define	THREADPRINTF(x...)		printf("THREAD:" x)
#else
#define THREADPRINTF(x...)
#endif

//��Ϣģ���ӡ����
#ifdef CFG_DEBUG_MSG
#define	MSGPRINTF(x...)			printf("MSG:" x)
#else
#define MSGPRINTF(x...)
#endif

#endif /* _DEBUG_H */
