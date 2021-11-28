/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ƽ̨
	�ļ�		��  led.c
	����		��  ���ļ�������״̬���豸�Ĳ����ӿ�
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2010.12
******************************************************************************/
//������ͷ�ļ�
#include "framework/config.h"
	
//ģ�����ÿ���
#ifdef CFG_LED_DEVICE
//C��ͷ�ļ�
//#include <stdio.h>
//#include <fcntl.h> 		//open ��־
//#include <sys/ioctl.h>	//ioctl
//#include <string.h> 	//memset
#include <sys/select.h>		//select
//#include <signal.h>
	
//����ƽ̨ͷ�ļ�
#include "sge_core/gpio.h"
#include "sge_core/error.h"
#include "sge_core/pinio.h"
#include "sge_core/thread.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/device/led.h"
#include "framework/message.h"
#include "framework/base.h"
	
/*************************************************
  ��̬ȫ�ֱ������궨��
*************************************************/

struct period_arg_t{
	u8 id;
	u32 last;
	u32 period;
};


#define MAX_LED_CHN 8						//���led����
static u8 	led_count = 0;					//ģ��򿪼���
static pthread_mutex_t mutex;				//��ѯled״̬������

static struct {
	u8 chn;					//ledͨ����Ӧ������ͨ��
	u8 type;
	u8 period;				//led���ڶ��������־
	u8 cancel;				//�߳�ȡ����־
	pthread_mutex_t lock;
	pthread_t thread_id;	//���ڶ����߳�id
	struct period_arg_t thread_arg;
}led[CFG_LED_NUM];

/*************************************************
  API
*************************************************/
/******************************************************************************
*	����:	led_act
*	����:	led����
*	����:	id
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_CFG		-	����IO�ߵ���led����֮�����ϵ����
*	˵��:
 ******************************************************************************/
static int led_act(u8 id)
{
	int ret;

	if(led[id].type == 1){
		ret = gpio_output_set(led[id].chn, 1);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
	}else if(led[id].type == 0){
		ret = gpio_output_set(led[id].chn, 0);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
	}else{
		ret = -ERR_CFG;
		goto err;
	}
	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	led_disact
*	����:	ledϨ��
*	����:	id
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_CFG		-	����IO�ߵ���led����֮�����ϵ����
*	˵��:
 ******************************************************************************/
static int led_disact(u8 id)
{
	int ret;

	if(led[id].type == 1){
		ret = gpio_output_set(led[id].chn, 0);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
	}else if(led[id].type == 0){
		ret = gpio_output_set(led[id].chn, 1);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
	}else{
		ret = -ERR_CFG;
		goto err;
	}
err:
	return ret;
}
/******************************************************************************
*	����:	led_init
*	����:	״̬���豸��ʼ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_CFG		-	���ó���
			-ERR_BUSY		-	�Ѿ���
			-ERR_SYS		-	ϵͳ����
			-ERR_NOFILE		-	û�д�·��
 ******************************************************************************/
int led_init(void)
{
	int ret = -1;
	int i;
	if(CFG_LED_NUM > MAX_LED_CHN){
		ret = -ERR_CFG; 	//���ó���
		goto err;
	}
	if(led_count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}

	for(i=0;i<CFG_LED_NUM;i++ ){
		led[i].chn = 0;
		led[i].period = 0;
		led[i].cancel = 0;
		pthread_mutex_init(&led[i].lock, NULL);
	}

//����ͨ��
#ifdef CFG_LED_0
	led[0].chn = CFG_LED_0;
	led[0].type = CFG_LED_TYPE0;
#endif
#ifdef CFG_LED_1
	led[1].chn = CFG_LED_1;
	led[1].type = CFG_LED_TYPE1;
#endif
#ifdef CFG_LED_2
	led[2].chn = CFG_LED_2;
	led[2].type= CFG_LED_TYPE2;
#endif
#ifdef CFG_LED_3
	led[3].chn = CFG_LED_3  ;
	led[3].type = CFG_LED_TYPE3;
#endif
#ifdef CFG_LED_4
	led[4].chn = CFG_LED_4;
	led[4].type = CFG_LED_TYPE4;
#endif
#ifdef CFG_LED_5
	led[5].chn = CFG_LED_5;
	led[5].type = CFG_LED_TYPE5;
#endif
#ifdef CFG_LED_6
	led[6].chn = CFG_LED_6 ;
	led[6].type= CFG_LED_TYPE6;
#endif
#ifdef CFG_LED_7
	led[7].chn = CFG_LED_7;
	led[7].type = CFG_LED_TYPE7;
#endif

	//��gpio��
	ret = gpio_init();
	if((ret < 0) && (ret != -ERR_BUSY)){
		goto err;
	}

	//���ü̵������Ĭ�ϲ�����
	for(i = 0; i < CFG_LED_NUM; i ++){
		if(led[i].chn > 0){					//����io��Ϊ���
			ret = led_disact(i);			//���ò�����
			if(ret < 0){
				goto err;
			}
			ret = gpio_set(led[i].chn, GPIO_OUT, GPIO_ODD, GPIO_PUD);
			if(ret < 0){
				ret = -ERR_SYS;
				goto err;
			}
		}
	}

	if (pthread_mutex_init(&mutex, NULL)) {	//��ʼ��������
		ret = -ERR_SYS;
		goto err;
	}

	led_count = 1;
	ret = 0;
err:
	return ret;
}


/******************************************************************************
*	����:	led_thread_period
*	����:	led���������߳�
*	����:
*	����:
*	˵��:
 ******************************************************************************/
static void * led_thread_period(void * arg)
{
	int ret = -1;
	fd_set rfds;
	struct timeval tv;
	int fd = 1;
	struct period_arg_t *per_arg;

	tv.tv_sec = 0;
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);

	per_arg = (struct period_arg_t *)arg;

	led[per_arg->id].period = 1;	//�����߳����б�־λ�����߳�ȡ���á�
	pthread_mutex_lock(&led[per_arg->id].lock);
	led[per_arg->id].cancel = 0;
	pthread_mutex_unlock(&led[per_arg->id].lock);

	//�����߳����ԣ��յ�cancel�������˳�
//	ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
//	printf("%s,%s,%d;thread %d ;ret = %d\n",__FILE__ ,__FUNCTION__,__LINE__,per_arg->id, ret);
//	ret = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED ,NULL);
//	printf("thread %d ;ret = %d\n",per_arg->id, ret);

	//�������Ϊperiod����������Ϊlast�ĵ�ƽ
	while(1){
		ret = led_act(per_arg->id);
		if(ret < 0){
			break;
		}
		tv.tv_usec = per_arg->last*1000;
		select (0, NULL, NULL, NULL, &tv);

		ret = led_disact(per_arg->id);
		if(ret < 0){
			break;
		}

		if(led[per_arg->id].cancel == 1){
			pthread_mutex_lock(&led[per_arg->id].lock);
			led[per_arg->id].cancel = 0;
			pthread_mutex_unlock(&led[per_arg->id].lock);
			pthread_exit(0);
		}

		tv.tv_usec = (per_arg->period - per_arg->last) * 1000;
		select (0, NULL, NULL, NULL, &tv);

	}
	pthread_exit(0);
}

/******************************************************************************
*	����:	led_on
*	����:	led��������
*	����:	id				-	led���
*			delay			-	��ʱʱ�䣬��λΪms��Ϊ0��ʾû����ʱ
			last			-	����ʱ�䣬��λΪms��Ϊ0��ʾû�г���
			period			-	����ʱ�䣬��λΪms��Ϊ0��ʾû������
*	����:	0				-	�ɹ�
			-ERR_INVAL		-	��������
			-ERR_NOINIT		-	û�г�ʼ��
			-ERR_NODEV		-	û�д��豸
			-ERR_CFG		-	���ó���
			-ERR_SYS		-	ϵͳ����
					2		-	�Ѿ�������˸����Ϩ����˸��led�ٲ���
*	˵��:	3��ʱ��ȫΪ0ʱ����ʾһֱ��, �˴�ʱ�侫�Ȳ��ߣ�Ӧ>20ms.
 ******************************************************************************/
int led_on(u8 id, u32 delay, u32 last, u32 period)
{
	int ret = -1;

	fd_set rfds;					//select��ʱ
	struct timeval tv;
	int fd = 1;

	if(led[id].period == 1){
		ret = 2;			//�Ѿ�������˸����Ϩ����˸��led�ٲ���
		goto err;
	}

	led[id].thread_arg.id = id;
	led[id].thread_arg.last = last;
	led[id].thread_arg.period = period;

    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);

	if(led_count == 0){
		return -ERR_NOINIT;
	}
	if((id < 0) || (id >= MAX_LED_CHN)){		//��Χ���
		return -ERR_NODEV;
	}

	if(led[id].chn == 0){
		return -ERR_CFG;
	}

	//�ӳ�delay ms
	if(delay > 0){
		tv.tv_sec = 0;
		tv.tv_usec = delay*1000;
		select (0, NULL, NULL, NULL, &tv);

	}
	//���ڶ���
	if(period > 0){
		if((last <= 0) || (period <= last) ){
			ret = -ERR_INVAL;
			goto err;
		}

//		ret = pthread_create(&led[id].thread_id, NULL, led_thread_period, (void *)&led[id].thread_arg);
		ret = thread_create_base(&led[id].thread_id, led_thread_period, (void *)&led[id].thread_arg, THREAD_MODE_REALTIME,80);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
	}else{
		//����led��,�ӳ�last ms������led��
		if(last > 0){
			ret = led_act(id);
			if(ret < 0){
				goto err;
			}
			tv.tv_sec = 0;
			tv.tv_usec = last*1000;
			select (0, NULL, NULL, NULL, &tv);
			ret = led_disact(id);
			if(ret < 0){
				goto err;
			}
		}
	}

	//��������Ϊ0ʱ��ֻ��led
	if((delay == 0) && (last == 0) && (period == 0)){
		ret = led_act(id);
		if(ret < 0){
			goto err;
		}
	}

	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	led_off
*	����:	led��
*	����:	id				-	led��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_CFG		-	����IO�ߵ���led����֮�����ϵ����
*	˵��:
 ******************************************************************************/
int led_off(u8 id)
{
	int ret = -1;

	if(led_count == 0){
		return -ERR_NOINIT;
	}
	if((id < 0) || (id >= MAX_LED_CHN)){		//��Χ���
		return -ERR_NODEV;
	}

	if(led[id].chn == 0){
		return -ERR_CFG;
	}

	//���ڶ��������������̣߳����ȴ��߳̽���
	if(led[id].period == 1){
//		ret = pthread_cancel(led[id].thread_id);
//		if(ret < 0){
//			ret = -ERR_SYS;
//			goto err;
//		}
//		ret = pthread_kill(led[id].thread_id, SIGKILL);
//		if(ret < 0){
//			ret = -ERR_SYS;
//			goto err;
//		}
//		ret = pthread_join(led[id].thread_id,NULL);
//		if(ret < 0){
//			ret = -ERR_SYS;
//			goto err;
//		}
		pthread_mutex_lock(&led[id].lock);
		led[id].cancel = 1;
		pthread_mutex_unlock(&led[id].lock);
		led[id].period = 0;

	}
	ret = led_disact(id);
	if(ret < 0){
		goto err;
	}

	ret = 0;
err:
	return ret;
}	

/******************************************************************************
*	����:	led_check
*	����:	���led״̬
*	����:	id				-	ledͨ����
*	����:	1				-	��
			0				-	��
			-ERR_TIMEOUT	-	��ʱ
			-ERR_NODEV 		-	�޴��豸
			-ERR_NOINIT		-	��û�г�ʼ����
			-ERR_OTHER:		-	���������̻߳������Ĵ���
			-ERR_SYS		-	ϵͳ����
*	˵��:
 ******************************************************************************/
int led_check(u8 id)
{
	int ret = -1;

	u8 vp = 0xff;

	if(led_count == 0){
			return -ERR_NOINIT;
	}
	if((id < 0) || (id >= MAX_LED_CHN)){		//din��Χ���
		return -ERR_NODEV;		
	}	
	//��û�����
	if (pthread_mutex_lock (&mutex)) {
		return	-ERR_NOINIT;
	}

	if(led[id].chn == 0){
		return -ERR_CFG;
	}

	//��ȡio�ڵ�ƽ
	ret = gpio_output_get(led[id].chn, &vp);
	if(ret < 0){
		ret = -ERR_SYS;
		goto err;
	}
	if(vp > 1){				//����ֵ��0�� 1����
		ret = -ERR_SYS;
		goto err;
	}

	//�ж�led״̬
	if(led[id].type == 1){
		if(vp == 1){
			ret = 1;
		}else{
			ret = 0;
		}
	}else if(led[id].type== 0){
		if(vp == 0){
			ret = 1;
		}else{
			ret = 0;
		}
	}else{
		ret = -ERR_CFG;
		goto err;
	}
err:	
	if (pthread_mutex_unlock (&mutex)) {
		ret = -ERR_OTHER;	
	}	
	return ret;
}

/******************************************************************************
*	����:	led_close
*	����:	ledģ��رպ���
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û�г�ʼ��
			-ERR_OTHER		-	���������̻߳������Ĵ���
			-ERR_CFG		-	����IO�ߵ���led����֮�����ϵ����
*	˵��:	��
 ******************************************************************************/
//int led_close(void)
//{
//	int ret = -1;
//	int i;
//
//	if(led_count == 0){
//		return -ERR_NOINIT;
//	}
//	for(i = 0; i < CFG_LED_NUM; i ++){
//		if(led[i].chn > 0){
//			ret = led_off(i);					//���ò�����
//			if(ret < 0){
//				return ret;
//			}
//		}
//	}
//
////	ret = close(led_fd);
//	if(ret < 0){
//		return -ERR_SYS;
//	}
//	led_count = 0;
//	if (pthread_mutex_destroy(&mutex)) {
//		ret = -ERR_OTHER;
//	}
//	ret = 0;
//
//	return ret;
//}
#endif
