/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��config.h
	����		�����ļ�������ҵ��ƽ̨��ܵ�����ͷ�ļ�
	�汾		��0.1
	����		����迡
	��������	��2010.12
******************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

//��������
#define CFG_FRAMEWORK_DEBUG
#define CFG_MODULE_DEBUG

//��Ϣ����ģ��������
#define CFG_MESSAGE_SUBSCRIBE_MAX 	8		//ÿ����Ϣ�������Ķ��Ĵ���
#define CFG_MESSAGE_TYPE_MAX		64		//��Ϣ��������

//ʱ�����ģ��������
#define CFG_SYSTIME_TIMER_MAX		64		//����ע������ʱ������

//��ܳ�ʼ��ģ��������
#define CFG_FRAMEWORK_THREAD_MAX	32		//��ܳ�ʼ��ʱ֧�ֵ�����߳���

//ƽ̨�豸������

//����
#define CFG_KEY_DEVICE 			1				//�Ƿ�����
#define CFG_KEY_NUM				7				//��ǰKEYͨ������

#define CFG_KEY_LONG			3000			//������ʼ��Ӧ�������msΪ������λ����ע�ⲻҪ���ڰ�����ȡapi�ĳ�ʱ��
#define CFG_KEY_PER				500				//�������ؼ������msΪ������λ����ע�ⲻҪС�ڰ���ɨ��ʱ��
#define CFG_KEY_SCAN_DELAY		100				//����ɨ����ʱ����λms���Ƽ�20~100ms�䡣
#define CFG_KEY_MODE			1				//������Ӧģʽ��1-���·��أ�0-�ͷŷ���

#define CFG_KEY_ENTER			PIN_PC8			//key0
#define CFG_KEY_ENTER_TYPE		0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

#define CFG_KEY_CANCEL			PIN_PA25		//key1
#define CFG_KEY_CANCEL_TYPE		0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

#define CFG_KEY_LEFT			PIN_PA22		//key2
#define CFG_KEY_LEFT_TYPE		0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

#define CFG_KEY_UP				PIN_PA6			//key3
#define CFG_KEY_UP_TYPE			0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

#define CFG_KEY_RIGHT			PIN_PB31		//key4
#define CFG_KEY_RIGHT_TYPE		0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

#define CFG_KEY_DOWN			PIN_PA7			//key5
#define CFG_KEY_DOWN_TYPE		0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

#define CFG_KEY_PROGREM			PIN_PB30		//PG key
#define CFG_KEY_PROGREM_TYPE 	0				//IO�ߵ��밴���Ƿ���֮�����ϵ��1�߰��£�0�Ͱ���

//״̬��ģ������
#define CFG_LED_DEVICE			1				//�Ƿ�����
#define CFG_LED_NUM				4				//��ǰLEDͨ������

#define CFG_LED_0				PIN_PB18		//D9 WARN LED
#define CFG_LED_TYPE0			0				//IO�ߵ���LED����֮�����ϵ��1������0����

#define CFG_LED_1				PIN_PA10		//D5 LED RXDJL ���
#define CFG_LED_TYPE1			0				//IO�ߵ���LED����֮�����ϵ��1������0����

#define CFG_LED_2				PIN_PA9			//D5 LED TXDJL �̹�
#define CFG_LED_TYPE2			0				//IO�ߵ���LED����֮�����ϵ��1����������

#define CFG_LED_3				PIN_PA11		//D10 RUN LED
#define CFG_LED_TYPE3			0				//IO�ߵ���LED����֮�����ϵ��1������0����

//�̵����豸����
#define CFG_RELAY_DEVICE		1				//�Ƿ�����
#define CFG_RELAY_NUM			1				//��ǰRELAYͨ������

#define CFG_RELAY_0				PIN_PB17		//0��ͨ����Ӧ��Ӳ��ͨ��
//#undef  CFG_RELAY_D0
//#define CFG_RELAY_D0			PIN_PB19		//0��ͨ��˫���Ӧ��Ӳ��ͨ��
#define CFG_RELAY_TYPE0			0				//IO�ߵ���̵�������֮�����ϵ��1�߶�����0�Ͷ���

//�������豸����
#define CFG_POWERD_DEVICE		1				//�Ƿ�����

//ң��������������
#define CFG_SWIN_DEVICE			1				//�Ƿ�����
#define CFG_SWIN_TIMER			5				//���ڲ�����������Ķ�ʱ����
#define CFG_SWIN_FILTER_TIME	5				//���������������λms��
#define CFG_SWIN_FILTER_NUM		5				//������������
#define CFG_SWIN_READ_CYCLE		1000			//��ȡ��������ң�����ڣ���λms Ĭ��1s

#define CFG_SWIN_DOOR			PIN_PB16			//�Žӵ��źŶ�ӦIO
#define CFG_SWIN_LID_TOP		PIN_PB16			//������źŶ�ӦIO
#define CFG_SWIN_LID_MID		PIN_PB16			//���и��źŶ�ӦIO
#define CFG_SWIN_LID_TAIL		PIN_PB16		//��β���źŶ�ӦIO

#define CFG_SWIN_NUM			1				//ң��������������
#define CFG_SWIN_0				PIN_PC8			//0��ң�Ż�����ͨ����Ӧ��io��

//#define CFG_SWIN_1				PIN_PA5		//1��ң�Ż�����ͨ����Ӧ��io��
//#define CFG_SWIN_2				PIN_PA6		//2��ң�Ż�����ͨ����Ӧ��io��
//#define CFG_SWIN_3				PIN_PA7		//3��ң�Ż�����ͨ����Ӧ��io��
//#define CFG_SWIN_4				PIN_PB8		//4��ң�Ż�����ͨ����Ӧ��io��
//#define CFG_SWIN_5				PIN_PB9		//1��ң�Ż�����ͨ����Ӧ��io��
//#define CFG_SWIN_6				PIN_PB10	//2��ң�Ż�����ͨ����Ӧ��io��
//#define CFG_SWIN_7				PIN_PB11	//3��ң�Ż�����ͨ����Ӧ��io��

//�������豸����
#define CFG_DSTREAM_DEVICE 	1
#define DSTREAM_ZB_COM		5		//�ز���Ӧ���ڲ�com��
#define DSTREAM_IRDA_COM	12		//����
#define DSTREAM_DEBUG_COM	13		//����
#define DSTREAM_USBD_COM	7		//USB�ӿ�
#define DSTREAM_JC_COM		6		//�װ�
#define DSTREAM_485_1_COM	3		//1#485�� �ɼ�485
#define DSTREAM_485_2_COM	2		//2#485�� ����485
#define DSTREAM_485_3_COM	11		//3#485�� ����485 �ӵ�Ƭ��

//GPRS�豸����
#define CFG_GPRS_DEVICE 	1

//�����豸����
#define CFG_ETHER_DEVICE 	1


#endif
