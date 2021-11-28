/*****************************************************************************
	��̵����ɷ����޹�˾			��Ȩ��2008-2015

	��Դ���뼰������ĵ�Ϊ���������̵����ɷ����޹�˾�������У�δ��������
	�ɲ��������޸Ļ򷢲�������׷����صķ������Ρ�

						���������̹ɷ����޹�˾
						www.xjgc.com
						(0374) 321 2924
*****************************************************************************/


/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ƽ̨
	�ļ�		��  adc.c
	����		��  ���ļ������˶�ʱ��ģ��Ľӿ�
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2010.01
******************************************************************************/
//������ͷ�ļ�
#include "private/config.h"
	
//ģ�����ÿ���
#ifdef CFG_TIMER_MODULE
	
//C��ͷ�ļ�
#include <stdio.h>
#include <fcntl.h> 		//open ��־	
#include <sys/ioctl.h>	//ioctl
#include <string.h> 	//memset
#include <unistd.h>		//close
#include <pthread.h>	//pthread�⺯��
	
//�ṩ���û���ͷ�ļ�
#include "include/timer.h"
#include "include/error.h"
	
//��������ͷ�ļ�
#include "private/drvlib/timerlib.h"
#include "private/debug.h"
	
/*************************************************
  ��̬ȫ�ֱ������궨��
*************************************************/
#define MAX_TIMER	 6						//���ʱ������
#define TIMER_PRECISION_HIGH_S SCK32KIHZ	//�߾��ȵ�Ƶʱ��Դ
#define TIMER_PRECISION_LOW_S MCKD8		//�;��ȸ�Ƶʱ��Դ
static struct {
	u8 	count;				//ģ��򿪼���
	int fd;					//�ļ�������
	int fun;				//��ʱ������

	
	u32	heart_val;			//��ʱ����������

	u16	pwm_freq;			//pwmƵ��
	u8	pwm_fz;				//pwmռ�ձȷ���
	u8	pwm_fm;				//pwmռ�ձȷ�ĸ

	pthread_mutex_t mutex;	//������
}timer[MAX_TIMER]={{0}};//ģ��򿪼�����ʼ��Ϊ0
/*************************************************
  API
*************************************************/

/******************************************************************************
*	����:	timer_init
*	����:	��ʱ��ģ���ʼ��
*	����:	id				-	��ʱ��ͨ����
			mode			-	��ʱ������ģʽ
*	����:	0				-	�ɹ�
			-ERR_NODEV		-	�޴��豸
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	������Ч
			-ERR_BUSY		-	�Ѿ���
			-ERR_NOFILE		-	û�д�·��
*	˵��:	��ʱ������ģʽ�������ڶ�ʱ��PWM����Ƶ�ʲ�����
 ******************************************************************************/
int timer_init (u8 id, u8 mode)
{
	int ret = -1;	
	char *dev[]={"/dev/atmel_tc0","/dev/atmel_tc1","/dev/atmel_tc2",
				"/dev/atmel_tc3","/dev/atmel_tc4","/dev/atmel_tc5"};

	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}
	if((mode == TIMER_MODE_HEART) || (mode == TIMER_MODE_MEASURE) || (mode == TIMER_MODE_PWM)){
		timer[id].fun = mode;
	}else{
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	//��ʼ��������
	if (pthread_mutex_init(&timer[id].mutex, NULL)) {
		ret = -ERR_SYS;
		goto err;
	}

	//�򿪶�ʱ������
	timer[id].fd = open(dev[id], O_RDONLY);
	if (timer[id].fd < 0){
		ret = -ERR_NOFILE;		//û�д�·��
		goto err;
	}	

	timer[id].count = 1;
	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	timer_heart_setconfig
*	����:	��ʱ������ģʽ������
*	����:	id				-	��ʱ��ͨ����
			interval		-	���ڶ�ʱ�ļ����10ms�ı�������125ms�ı�����
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û��ʼ��
*	˵��:	�����û�йص���ʱ��ʱ���ٴ�������ʱʱ�䣬�ᱨ-ERR_SYS����
 ******************************************************************************/
int timer_heart_setconfig (u8 id, u32 interval)
{
	int ret = -1;
	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_HEART){
		ret = -ERR_NOFUN;
		goto err;
	}

	if(interval < 10 || interval > 1000 || ((interval%10 != 0) &&  (interval%125 != 0)) ){//�жϲ�����Ч��
		ret = -ERR_INVAL;		
		goto err;
	}
	//���ö�ʱ����ʱ��			
	ret = ioctl(timer[id].fd, SET_CLOCK, SCK32KIHZ); 
	if (ret < 0){
		ret = -ERR_SYS;
		goto err;
	} 
	//ֹͣ��ʱ��
	
	//��������
	ret = ioctl(timer[id].fd, SET_DELAY, interval);
	if (ret < 0){		
		ret = -ERR_SYS;
		goto err;
	}
	
	timer[id].heart_val = interval;		
	ret = 0;
err:
	return ret;
}


/******************************************************************************
*	����:	timer_heart_getconfig
*	����:	��ʱ������ģʽ�¶�ȡ����
*	����:	id				-	��ʱ��ͨ����
			interval		-	���ڶ�ʱ�ļ�������ݴ���ָ�룩
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û��ʼ��
			-ERR_NOCFG		-	û������
*	˵��:	��
 ******************************************************************************/
int timer_heart_getconfig (u8 id, u32 *interval)
{
	int ret = -1;
	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_HEART){
		ret = -ERR_NOFUN;
		goto err;
	}
	if(timer[id].heart_val < 10 || timer[id].heart_val > 2000 ||	//�жϲ�����Ч��
			((timer[id].heart_val%10 != 0) &&  (timer[id].heart_val != 125)) ){//�жϲ�����Ч��
		ret = -ERR_NOCFG;		
		goto err;
	}
	*interval = timer[id].heart_val;
	
	ret = 0;
err:
	return ret;
}


/******************************************************************************
*	����:	timer_heart_start
*	����:	���ڶ�ʱģʽ���������ڴ���
*	����:	id				-	��ʱ��ͨ����
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û��ʼ��
			-ERR_NOCFG		-	û������
*	˵��:	��
 ******************************************************************************/
int timer_heart_start(u8 id)
{
	int ret = -1;
	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_HEART){
		ret = -ERR_NOFUN;
		goto err;
	}
	//�ж��Ƿ�������
	if(timer[id].heart_val == 0 ){
		ret = -ERR_NOCFG;
		goto err;
	}
	//������ʱ��
	ret = ioctl(timer[id].fd, TCSTART, 0);
	if (ret < 0){
		ret = -ERR_SYS;
		goto err;
	}
	ret = 0;
err:
	return ret;
	
}

/******************************************************************************
*	����:	timer_heart_wait
*	����:	���ڶ�ʱģʽ�µȴ����ڴ���
*	����:	id				-	��ʱ��ͨ����
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û��ʼ��
*	˵��:	��
 ******************************************************************************/
int timer_heart_wait (u8 id)
{
	int ret = -1, a;
	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_HEART){
		ret = -ERR_NOFUN;
		goto err;
	}
		
	ret = read(timer[id].fd, (char *)&a, 2);	//��ȡ��ʱ��
	if (ret < 0){
		ret = -ERR_SYS;
		goto err;
	}
	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	timer_pwm_setconfig
*	����:	���û�ı�PWM���ܵ�Ƶ�ʣ�ռ�ձȵȲ���
*	����:	id				-	��ʱ����
			freq			-	�������Ƶ��
			fz				-	ռ�ձȷ���
			fm				-	ռ�ձȷ�ĸ
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û��ʼ��
			-ERR_OTHER		-	��������ϵͳ�߳����Ĵ���
*	˵��:	���ı�Ƶ�����ʱ�����ֲ���Ϊ0ʱ����ʾ���ı���ֵ
 ******************************************************************************/
int timer_pwm_setconfig (u8 id, u16 freq, u8 fz, u8 fm)
{
	int ret = -1;	
	
	long long arg;
	
	//��û�����
	if (pthread_mutex_lock (&timer[id].mutex)) {
		return	-ERR_NOINIT;		
	}
	if(fz == 0 || fm == 0){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	
	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_PWM){
		ret = -ERR_NOFUN;
		goto err;
	}
	//����Ƶ������ʱ��??????
	ret = ioctl(timer[id].fd, SET_CLOCK, SCK32KIHZ); 
	if (ret < 0){
		ret = -ERR_SYS;
		goto err;
	} 

	//ֹͣ��ʱ��(�������ò���ϵͳ����ʱ���Զ�ֹͣ��ʱ��)

	//���ò���
	arg = freq;
	arg = (arg<<32) | ((fz&0xff)<<16) | (fm&0xffff);	//����arg����
	
	ret = ioctl(timer[id].fd, SET_PWM, &arg);
	if (ret < 0){
		ret = -ERR_SYS;
		goto err;
	}

	//������ʱ��	
	ret = ioctl(timer[id].fd, TCSTART, 0);
	if (ret < 0){
		ret = -ERR_SYS;
		goto err;
	}
	timer[id].pwm_freq = freq;
	timer[id].pwm_fz = fz;
	timer[id].pwm_fm = fm;
	ret = 0;
err:
	if (pthread_mutex_unlock (&timer[id].mutex)) {
		ret = -ERR_OTHER;	
	}
	return ret;
}


/******************************************************************************
*	����:	timer_pwm_getconfig
*	����:	��ȡPWM���ܵ�Ƶ�ʣ�ռ�ձȵȲ���
*	����:	id				-	��ʱ����
			freq			-	�������Ƶ�ʣ����ݴ�����
			fz				-	ռ�ձȷ��ӣ����ݴ�����
			fm				-	ռ�ձȷ�ĸ�����ݴ�����
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_NOINIT		-	û�г�ʼ��
*	˵��:	��
 ******************************************************************************/
int timer_pwm_getconfig (u8 id, u16 *freq, u8 *fz, u8 *fm)
{
	int ret = -1;	
	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_PWM){
		ret = -ERR_NOFUN;
		goto err;
	}

	*freq = timer[id].pwm_freq ;
	*fz = timer[id].pwm_fz ;
	*fm = timer[id].pwm_fm ;
	
	ret = 0;
err:
	return ret;
}


/******************************************************************************
*	����:	timer_measure_read
*	����:	��ȡƵ�ʲ������ܵ�Ƶ��
*	����:	id				-	��ʱ��ͨ����
			freq			-	��ȡ��Ƶ��
*	����:	0				-	�ɹ�
			-ERR_NOFUN		-	�޴˹���
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û��ʼ��
*	˵��:	��
 ******************************************************************************/
int timer_mesure_read(u8 id, u16 *freq)
{
	int ret = -1;	

	if(id < 0 || id >= MAX_TIMER){
		ret = -ERR_INVAL;		//������Ч
		goto err;
	}
	if(timer[id].count == 0){
		ret = -ERR_NOINIT;
		goto err;
	}
	//�жϹ�����Ч��
	if(timer[id].fun != TIMER_MODE_MEASURE){
		ret = -ERR_NOFUN;
		goto err;
	}
	//����ʱ��,���ݲ���Ƶ������ʱ��?????????
	
	//��ȡƵ��???????

	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	timer_close
*	����:	�رն�ʱ��
*	����:	id				-	��ʱ��ͨ����
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	������Ч
			-ERR_NOINIT		-	û�г�ʼ��
			-ERR_OTHER		-	���������߳����Ĵ���
*	˵��:	��
 ******************************************************************************/
int timer_close (u8 id)
{
	int ret = -1;
	if(id < 0 || id >= MAX_TIMER){
		return  -ERR_INVAL;		//������Ч		
	}
	if(timer[id].count == 0)
		return -ERR_NOINIT;
	ret = close(timer[id].fd);
	if(ret < 0)
		return -ERR_SYS;
	
	timer[id].count = 0;
	//���ٻ�����	
	if (pthread_mutex_destroy(&timer[id].mutex)) {
		ret = -ERR_OTHER;
	}

	ret = 0;
	return ret;
}


#endif 		//CFG_TIMER_MODULE
