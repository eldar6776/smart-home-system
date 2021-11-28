
#ifndef _TIMERLIB_H
#define _TIMERLIB_H

//define ioctl command 
#define TIMER_IOC_MAGIC 0xE2
 
#define SET_DELAY		_IO(TIMER_IOC_MAGIC,  0)	//����TIMERΪ��ʱ����
#define SET_MEASURE		_IO(TIMER_IOC_MAGIC,  1)	//����TIMERΪƵ�ʲ�������
#define SET_PWM			_IO(TIMER_IOC_MAGIC,  2)	//����TIMERΪ������ƹ���
#define SET_PULSE		_IO(TIMER_IOC_MAGIC,  3)	//����TIMERΪ��Ͽ��빦��

#define SET_CLOCK		_IO(TIMER_IOC_MAGIC,  4)	//����timerʱ��
#define TCSTART			_IO(TIMER_IOC_MAGIC,  5)	//����TIMER
#define TCSTOP			_IO(TIMER_IOC_MAGIC,  6)	//ֹͣTIMER

//����SETCLOCK ��Ӧarg����, ��ʱ��Ƶ��ѡ��
#define MCKD2		0 		//��ʱ��ʱ������Ƶ��MCK/2		
#define MCKD8		1		//��ʱ��ʱ������Ƶ��MCK/8		
#define MCKD32		2		//��ʱ��ʱ������Ƶ��MCK/32		
#define MCKD128		3		//��ʱ��ʱ������Ƶ��MCK/128		
#define SCK32KIHZ	4		//��ʱ��ʱ������Ƶ��32.768K
#define ETCLK0		5		//��ʱ��ʱ�������ⲿ0
#define ETCLK1		6		//��ʱ��ʱ�������ⲿ1
#define ETCLK2		7		//��ʱ��ʱ�������ⲿ2

//����TCSTART ��Ӧarg����, ����ʱ����
#define TIMER0		0		//�˿�0
#define TIMER1		1		//�˿�1
#define TIMER2		2		//�˿�2
#define TIMER3		3		//�˿�3
#define TIMER4		4		//�˿�4
#define TIMER5		5		//�˿�5

#endif  /* _TIMERLIB_H */
