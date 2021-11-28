/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ƽ̨
	�ļ�		��  relay.c
	����		��  ���ļ������˼̵����豸�Ĳ����ӿ�
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2010.12
******************************************************************************/
//������ͷ�ļ�
#include "framework/config.h"
	
//ģ�����ÿ���
#ifdef CFG_RELAY_DEVICE
//C��ͷ�ļ�
#include <sys/select.h>		//select

	
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
#define MAX_RELAY_CHN 8						//���̵�������
struct period_arg_t{
	u8 id;
	u32 last;
	u32 period;
};


static u8 	relay_count = 0;				//ģ��򿪼���
static pthread_mutex_t mutex;				//��ѯrelay״̬������

static struct {
	u8 chn;					//relayͨ����Ӧ������ͨ��
	u8 chd;					//�̵���ͨ����Ӧ��˫������ͨ��
	u8 type;
	u8 period;				//relay���ڶ��������־
	u8 cancel;				//�߳�ȡ����־
	pthread_mutex_t lock;
	pthread_t thread_id;	//���ڶ����߳�id
	struct period_arg_t thread_arg;
}relay[CFG_RELAY_NUM];

/*************************************************
  API
*************************************************/
/******************************************************************************
*	����:	relay_act
*	����:	relay����
*	����:	id
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_CFG		-	����IO�ߵ���relay���϶Ͽ�֮�����ϵ����
*	˵��:
 ******************************************************************************/
static int relay_act(u8 id)
{
	int ret;

	if(relay[id].type == 1){
		ret = gpio_output_set(relay[id].chn, 1);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
		if(relay[id].chd > 0){		//˫��ģʽ
			ret = gpio_output_set(relay[id].chd, 0);
			if(ret < 0){
				ret = -ERR_SYS;
				goto err;
			}
		}
	}else if(relay[id].type == 0){
		ret = gpio_output_set(relay[id].chn, 0);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
		if(relay[id].chd > 0){		//˫��ģʽ
			ret = gpio_output_set(relay[id].chd, 1);
			if(ret < 0){
				ret = -ERR_SYS;
				goto err;
			}
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
*	����:	relay_disact
*	����:	relay������
*	����:	id
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_CFG		-	����IO�ߵ���relay����֮�����ϵ����
*	˵��:
 ******************************************************************************/
static int relay_disact(u8 id)
{
	int ret;

	if(relay[id].type == 1){
		ret = gpio_output_set(relay[id].chn, 0);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
		if(relay[id].chd > 0){		//˫��ģʽ
			ret = gpio_output_set(relay[id].chd, 1);
			if(ret < 0){
				ret = -ERR_SYS;
				goto err;
			}
		}
	}else if(relay[id].type == 0){
		ret = gpio_output_set(relay[id].chn, 1);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
		if(relay[id].chd > 0){		//˫��ģʽ
			ret = gpio_output_set(relay[id].chd, 0);
			if(ret < 0){
				ret = -ERR_SYS;
				goto err;
			}
		}
	}else{
		ret = -ERR_CFG;
		goto err;
	}
err:
	return ret;
}
/******************************************************************************
*	����:	relay_init
*	����:	�̵����豸��ʼ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_CFG		-	���ó���
			-ERR_BUSY		-	�Ѿ���
			-ERR_SYS		-	ϵͳ����
			-ERR_NOFILE		-	û�д�·��
 ******************************************************************************/
int relay_init(void)
{
	int ret = -1;
	int i;

	if(CFG_RELAY_NUM > MAX_RELAY_CHN){
		ret = -ERR_CFG; 	//���ó���
		goto err;
	}
	if(relay_count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}

	for(i=0;i<CFG_RELAY_NUM;i++ ){
		relay[i].chn = 0;
		relay[i].chd = 0;
		relay[i].period = 0;
		relay[i].cancel = 0;
		pthread_mutex_init(&relay[i].lock, NULL);
	}

//����ͨ��
#ifdef CFG_RELAY_0
	relay[0].chn = CFG_RELAY_0;
	relay[0].type = CFG_RELAY_TYPE0;
#endif
#ifdef CFG_RELAY_1
	relay[1].chn = CFG_RELAY_1;
	relay[1].type = CFG_RELAY_TYPE1;
#endif
#ifdef CFG_RELAY_2
	relay[2].chn = CFG_RELAY_2;
	relay[2].type= CFG_RELAY_TYPE2;
#endif
#ifdef CFG_RELAY_3
	relay[3].chn = CFG_RELAY_3  ;
	relay[3].type = CFG_RELAY_TYPE3;
#endif
#ifdef CFG_RELAY_4
	relay[4].chn = CFG_RELAY_4;
	relay[4].type = CFG_RELAY_TYPE4;
#endif
#ifdef CFG_RELAY_5
	relay[5].chn = CFG_RELAY_5;
	relay[5].type = CFG_RELAY_TYPE5;
#endif
#ifdef CFG_RELAY_6
	relay[6].chn = CFG_RELAY_6 ;
	relay[6].type= CFG_RELAY_TYPE6;
#endif
#ifdef CFG_RELAY_7
	relay[7].chn = CFG_RELAY_7;
	relay[7].type = CFG_RELAY_TYPE7;
#endif

//˫���ж�
#ifdef CFG_RELAY_D0
	relay[0].chd = CFG_RELAY_D0;
#endif
#ifdef CFG_RELAY_D1
	relay[1].chd = CFG_RELAY_D1;
#endif
#ifdef CFG_RELAY_D2
	relay[2].chd = CFG_RELAY_D2;
#endif
#ifdef CFG_RELAY_D3
	relay[3].chd = CFG_RELAY_D3 ;
#endif
#ifdef CFG_RELAY_D4
	relay[4].chd = CFG_RELAY_D4;
#endif
#ifdef CFG_RELAY_D5
	relay[5].chd = CFG_RELAY_D5;
#endif
#ifdef CFG_RELAY_D6
	relay[6].chd = CFG_RELAY_D6 ;
#endif
#ifdef CFG_RELAY_D7
	relay[7].chd = CFG_RELAY_D7;
#endif
	//��gpio��
	ret = gpio_init();
	if((ret < 0) && (ret != -ERR_BUSY)){
		goto err;
	}

	//���ü̵������Ĭ�ϲ�����
	for(i = 0; i < CFG_RELAY_NUM; i ++){
		if(relay[i].chn > 0){					//����io��Ϊ���
			ret = relay_disact(i);			//���ò�����
			if(ret < 0){
				goto err;
			}
			ret = gpio_set(relay[i].chn, GPIO_OUT, GPIO_ODD, GPIO_PUD);
			if(ret < 0){
				ret = -ERR_SYS;
				goto err;
			}
			if(relay[i].chd > 0){					//����˫��io��Ϊ���
				ret = gpio_set(relay[i].chd, GPIO_OUT, GPIO_ODD, GPIO_PUD);
				if(ret < 0){
					ret = -ERR_SYS;
					goto err;
				}
			}
		}
	}

	if (pthread_mutex_init(&mutex, NULL)) {	//��ʼ��������
		ret = -ERR_SYS;
		goto err;
	}

	relay_count = 1;
	ret = 0;
err:
	return ret;
}


/******************************************************************************
*	����:	relay_thread_period
*	����:	relay���ڶ����߳�
*	����:
*	����:
*	˵��:
 ******************************************************************************/
static void * relay_thread_period(void * arg)
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

	relay[per_arg->id].period = 1;	//�����߳����б�־λ�����߳�ȡ���á�
	pthread_mutex_lock(&relay[per_arg->id].lock);
	relay[per_arg->id].cancel = 0;
	pthread_mutex_unlock(&relay[per_arg->id].lock);

	//�����߳����ԣ��յ�cancel�������˳�
//	ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
//	printf("%s,%s,%d;thread %d ;ret = %d\n",__FILE__ ,__FUNCTION__,__LINE__,per_arg->id, ret);
//	ret = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED ,NULL);
//	printf("thread %d ;ret = %d\n",per_arg->id, ret);

	//�������Ϊperiod����������Ϊlast�ĵ�ƽ
	while(1){
		ret = relay_act(per_arg->id);
		if(ret < 0){
			break;
		}
		tv.tv_usec = per_arg->last*1000;
		select (0, NULL, NULL, NULL, &tv);

		ret = relay_disact(per_arg->id);
		if(ret < 0){
			break;
		}

		if(relay[per_arg->id].cancel == 1){
			pthread_mutex_lock(&relay[per_arg->id].lock);
			relay[per_arg->id].cancel = 0;
			pthread_mutex_unlock(&relay[per_arg->id].lock);
			pthread_exit(0);
		}

		tv.tv_usec = (per_arg->period - per_arg->last) * 1000;
		select (0, NULL, NULL, NULL, &tv);

	}
	pthread_exit(0);
}

/******************************************************************************
*	����:	relay_on
*	����:	relay��������
*	����:	id				-	relay���
*			delay			-	��ʱʱ�䣬��λΪms��Ϊ0��ʾû����ʱ
			last			-	����ʱ�䣬��λΪms��Ϊ0��ʾû�г���
			period			-	����ʱ�䣬��λΪms��Ϊ0��ʾû������
*	����:	0				-	�ɹ�
			-ERR_INVAL		-	��������
			-ERR_NOINIT		-	û�г�ʼ��
			-ERR_NODEV		-	û�д��豸
			-ERR_CFG		-	���ó���
			-ERR_SYS		-	ϵͳ����
					2		-	�Ѿ����ڶ���������ֹͣrelay���ٲ���
*	˵��:	3��ʱ��ȫΪ0ʱ����ʾһֱ����, �˴�ʱ�侫�Ȳ��ߣ�Ӧ>20ms.
 ******************************************************************************/
int relay_on(u8 id, u32 delay, u32 last, u32 period)
{
	int ret = -1;

	fd_set rfds;					//select��ʱ
	struct timeval tv;
	int fd = 1;

	if(relay[id].period == 1){
		ret = 2;			//�Ѿ�������˸����Ϩ����˸��relay�ٲ���
		goto err;
	}

	relay[id].thread_arg.id = id;
	relay[id].thread_arg.last = last;
	relay[id].thread_arg.period = period;

    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);

	if(relay_count == 0){
		return -ERR_NOINIT;
	}
	if((id < 0) || (id >= MAX_RELAY_CHN)){		//��Χ���
		return -ERR_NODEV;
	}

	if(relay[id].chn == 0){
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

//		ret = pthread_create(&relay[id].thread_id, NULL, relay_thread_period, (void *)&relay[id].thread_arg);
		ret = thread_create_base(&relay[id].thread_id, relay_thread_period, (void *)&relay[id].thread_arg, THREAD_MODE_REALTIME,80);
		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}
	}else{
		//����relay����,�ӳ�last ms������relay��
		if(last > 0){
			ret = relay_act(id);
			if(ret < 0){
				goto err;
			}
			tv.tv_sec = 0;
			tv.tv_usec = last*1000;
			select (0, NULL, NULL, NULL, &tv);
			ret = relay_disact(id);
			if(ret < 0){
				goto err;
			}
		}
	}

	//��������Ϊ0ʱ��ֻ����relay
	if((delay == 0) && (last == 0) && (period == 0)){
		ret = relay_act(id);
		if(ret < 0){
			goto err;
		}
	}

	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	relay_off
*	����:	relay�ָ�
*	����:	id				-	relay��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_CFG		-	����IO�ߵ���relay����֮�����ϵ����
*	˵��:
 ******************************************************************************/
int relay_off(u8 id)
{
	int ret = -1;

	if(relay_count == 0){
		return -ERR_NOINIT;
	}
	if((id < 0) || (id >= MAX_RELAY_CHN)){		//��Χ���
		return -ERR_NODEV;
	}

	if(relay[id].chn == 0){
		return -ERR_CFG;
	}

	//���ڶ��������������̣߳����ȴ��߳̽���
	if(relay[id].period == 1){
//		ret = pthread_cancel(relay[id].thread_id);
//		if(ret < 0){
//			ret = -ERR_SYS;
//			goto err;
//		}
//		ret = pthread_kill(relay[id].thread_id, SIGKILL);
//		if(ret < 0){
//			ret = -ERR_SYS;
//			goto err;
//		}
//		ret = pthread_join(relay[id].thread_id,NULL);
//		if(ret < 0){
//			ret = -ERR_SYS;
//			goto err;
//		}
		pthread_mutex_lock(&relay[id].lock);
		relay[id].cancel = 1;
		pthread_mutex_unlock(&relay[id].lock);
		relay[id].period = 0;

	}
	ret = relay_disact(id);
	if(ret < 0){
		goto err;
	}

	ret = 0;
err:
	return ret;
}	

/******************************************************************************
*	����:	relay_check
*	����:	���relay״̬
*	����:	id				-	relayͨ����
*	����:	1				-	��
			0				-	��
			-ERR_TIMEOUT	-	��ʱ
			-ERR_NODEV 		-	�޴��豸
			-ERR_NOINIT		-	��û�г�ʼ����
			-ERR_OTHER:		-	���������̻߳������Ĵ���
			-ERR_SYS		-	ϵͳ����
*	˵��:
 ******************************************************************************/
int relay_check(u8 id)
{
	int ret = -1;

	u8 vp = 0xff;

	if(relay_count == 0){
			return -ERR_NOINIT;
	}
	if((id < 0) || (id >= MAX_RELAY_CHN)){		//din��Χ���
		return -ERR_NODEV;		
	}	
	//��û�����
	if (pthread_mutex_lock (&mutex)) {
		return	-ERR_NOINIT;
	}

	if(relay[id].chn == 0){
		return -ERR_CFG;
	}

	//��ȡio�ڵ�ƽ
	ret = gpio_output_get(relay[id].chn, &vp);
	if(ret < 0){
		ret = -ERR_SYS;
		goto err;
	}
	if(vp > 1){				//����ֵ��0�� 1����
		ret = -ERR_SYS;
		goto err;
	}

	//�ж�relay״̬
	if(relay[id].type == 1){
		if(vp == 1){
			ret = 1;
		}else{
			ret = 0;
		}
	}else if(relay[id].type== 0){
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
*	����:	relay_close
*	����:	relayģ��رպ���
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û�г�ʼ��
			-ERR_OTHER		-	���������̻߳������Ĵ���
			-ERR_CFG		-	����IO�ߵ���relay����֮�����ϵ����
*	˵��:	��
 ******************************************************************************/
//int relay_close(void)
//{
//	int ret = -1;
//	int i;
//
//	if(relay_count == 0){
//		return -ERR_NOINIT;
//	}
//	for(i = 0; i < CFG_RELAY_NUM; i ++){
//		if(relay[i].chn > 0){
//			ret = relay_off(i);					//���ò�����
//			if(ret < 0){
//				return ret;
//			}
//		}
//	}
//
////	ret = close(relay_fd);
//	if(ret < 0){
//		return -ERR_SYS;
//	}
//	relay_count = 0;
//	if (pthread_mutex_destroy(&mutex)) {
//		ret = -ERR_OTHER;
//	}
//	ret = 0;
//
//	return ret;
//}
#endif
