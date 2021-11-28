/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ҵ��ƽ̨
	�ļ�		��  key.c
	����		��  ���ļ������˰���ģ��Ľӿ�
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2010.12
******************************************************************************/
//������ͷ�ļ�
#include "framework/config.h"
	
//ģ�����ÿ���
#ifdef CFG_KEY_DEVICE

//C��ͷ�ļ�
#include <sys/select.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>

//����ƽ̨ͷ�ļ�
#include "sge_core/gpio.h"
#include "sge_core/error.h"
#include "sge_core/pinio.h"
#include "sge_core/thread.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/device/key.h"
#include "framework/message.h"
#include "framework/base.h"
	
/*************************************************
  ��̬ȫ�ֱ������궨��
*************************************************/
typedef struct {
	u8	status;				//keyֵ״̬
	u8	level;				//��ǰio�ڵ�ƽ
	u8	chn;				//keyͨ����Ӧ��io������ͨ��
	u8	type;				//IO�ߵ��밴���Ƿ���֮�����ϵ��1-�߰��£�0-���ͷ�
	u8	value;				//��ֵ
	u8	msg_type;			//��Ϣ����
}key_info_t;

#define KEYSTAT_UP		0				//��ǰ�����ͷ�
#define KEYSTAT_DOWN	1				//��ǰ��������
#define KEYSTAT_LONG	2				//��ǰ��������

#define NON_BLOCK		0

#define MAX_KEY_CHN 16					//���key����

static u8 	key_count = 0;				//ģ��򿪼���
static key_info_t	key[MAX_KEY_CHN ];
static int key_get(u8 timeout);

#ifdef LINUX_SIM
/******************************************************************************
*	����:	thread_key_scan
*	����:	ɨ�谴�������߳�
*	����:
*	����:
*	˵��:
 ******************************************************************************/
static void * thread_key_scan(void * arg)
{
	int ret=-1;
	message_t msg_key ;
	struct input_event event;
	int fd = open("/tmp/keyFifo", O_RDONLY);
	if (fd<0){
		fprintf(stderr, "open key fifo fail\n");
		pthread_exit(0);
	}

	//�������Ϊperiod����������Ϊlast�ĵ�ƽ
	while(1){
		if (read(fd, &event, sizeof(struct input_event))==
				sizeof(struct input_event)){
			msg_key.wpara 	= 	event.code;
			msg_key.type	=	MSG_KEY;
			message_dispatch(&msg_key);
		}
	}
	pthread_exit(0);
}
/******************************************************************************
*	����:	key_init
*	����:	����ģ���ʼ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_BUSY		-	�Ѿ���
	˵��:�����������󲿷ַ���gpio.h��gpio_init��gpio_set�����Ĵ���
 ******************************************************************************/
int key_init(void)
{
	int ret = -1;
	int i;
	pthread_t key_id;
	if(key_count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}

	ret = thread_create_base(&key_id, thread_key_scan, NULL, THREAD_MODE_REALTIME, 80);
	if((ret < 0) ){
		fprintf(stderr, "== %s %d== \n", __FUNCTION__, __LINE__);
		goto err;
	}

	key_count = 1;
	ret = 0;
err:
	return ret;
}
#else
/*************************************************
  API
*************************************************/
/******************************************************************************
*	����:	thread_key_scan
*	����:	ɨ�谴�������߳�
*	����:
*	����:
*	˵��:
 ******************************************************************************/
static void * thread_key_scan(void * arg)
{
	int ret=-1;
	message_t msg_key ;
	//�������Ϊperiod����������Ϊlast�ĵ�ƽ
	while(1){
		ret = key_get(NON_BLOCK);
		if(ret > 0){
			if(ret < 100){
				msg_key.wpara 	= 	key[ret].value;
				msg_key.type	=	key[ret].msg_type;
				message_dispatch(&msg_key);
			}else{
				ret -= 100;
				msg_key.wpara 	= 	key[ret].value + 100;
				msg_key.type	=	key[ret].msg_type;
				message_dispatch(&msg_key);
			}
		}
	}
	pthread_exit(0);
}
/******************************************************************************
*	����:	key_init
*	����:	����ģ���ʼ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_BUSY		-	�Ѿ���
	˵��:�����������󲿷ַ���gpio.h��gpio_init��gpio_set�����Ĵ���
 ******************************************************************************/
int key_init(void)
{
	int ret = -1;
	int i;
	pthread_t key_id;
	if(key_count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}

	//����ͨ��
#ifdef CFG_KEY_ENTER
	key[6].chn 		= 	CFG_KEY_ENTER ;
	key[6].type	 	= 	CFG_KEY_ENTER_TYPE;
	key[6].value 	= 	KEY_ENTER;
	key[6].msg_type	= 	MSG_KEY;
#endif
#ifdef CFG_KEY_CANCEL
	key[1].chn 		= 	CFG_KEY_CANCEL ;
	key[1].type		= 	CFG_KEY_CANCEL_TYPE;
	key[1].value 	= 	KEY_CANCEL;
	key[1].msg_type	= 	MSG_KEY;
#endif
#ifdef CFG_KEY_LEFT
	key[2].chn		= 	CFG_KEY_LEFT  ;
	key[2].type		= 	CFG_KEY_LEFT_TYPE;
	key[2].value 	= 	KEY_LEFT;
	key[2].msg_type	= 	MSG_KEY;
#endif
#ifdef CFG_KEY_UP
	key[3].chn 		= 	CFG_KEY_UP ;
	key[3].type 	= 	CFG_KEY_UP_TYPE;
	key[3].value 	= 	KEY_UP;
	key[3].msg_type	= 	MSG_KEY;
#endif
#ifdef CFG_KEY_RIGHT
	key[4].chn 		= 	CFG_KEY_RIGHT  ;
	key[4].type 	= 	CFG_KEY_RIGHT_TYPE;
	key[4].value 	= 	KEY_RIGHT;
	key[4].msg_type	= 	MSG_KEY;
#endif
#ifdef CFG_KEY_DOWN
	key[5].chn 		= 	CFG_KEY_DOWN ;
	key[5].type 	= 	CFG_KEY_DOWN_TYPE;
	key[5].value 	= 	KEY_DOWN;
	key[5].msg_type	= 	MSG_KEY;
#endif
#ifdef CFG_KEY_PROGREM
	key[7].chn 		= 	CFG_KEY_PROGREM  ;
	key[7].type 	= 	CFG_KEY_PROGREM_TYPE;
	key[7].value 	=	KEY_PROGREM;
	key[7].msg_type	= 	MSG_SKEY;
#endif

	//��gpio��
	ret = gpio_init();
	if((ret < 0) && (ret != -ERR_BUSY)){
		goto err;
	}
	for(i = 1; i <= CFG_KEY_NUM; i++){
		ret = gpio_set(key[i].chn,GPIO_IN,0,0);
		if(ret < 0){
			goto err;
		}
		key[i].status = KEYSTAT_UP;			//ȫ����ʼ��Ϊ̧��״̬
	}
	//��������ɨ���̣߳�ʵʱ�̣߳����ȼ�80-90
	ret = thread_create_base(&key_id, thread_key_scan, NULL, THREAD_MODE_REALTIME, 80);
	if((ret < 0) ){
		goto err;
	}

	key_count = 1;
	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	key_get
*	����:	��ȡ������Ч״̬
*	����:	timeout			-	��ʱʱ�䣨��λΪ��,Ϊ0��ʾ������������
*	����:	0				-	�޼�����
*			0~15			-	��Ӧ���Ű���
*			100~115			-	��Ӧ���ų���
			-ERR_NOINIT		-	û�г�ʼ��
*	˵��:�����������󲿷ַ���gpio.h��gpio_input_get�����Ĵ���
 ******************************************************************************/
static int key_get(u8 timeout)
{
	int ret = -1;
	int i;
	u32 delay = 0;		//��ʱʱ���ۻ�

	fd_set rfds;		//select��ʱ
	struct timeval tv;
	int fd = 1;

    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);

	if(key_count == 0){
		return -ERR_NOINIT;
	}

scan:
	//ɨ�谴��
	for(i = 1; i <= CFG_KEY_NUM; i++){
		ret = gpio_input_get(key[i].chn,&key[i].level);
		if(ret < 0){
			goto err;
		}
	}

	//����ɨ������,���м�����ͬʱ���£��򷵻ؼ���С��ֵ
	for(i = 1; i <= CFG_KEY_NUM; i++){
		switch(key[i].status){

			case KEYSTAT_DOWN:
				if(key[i].level != key[i].type){
					key[i].status = KEYSTAT_UP;	//�ͷ�
					if(CFG_KEY_MODE == 0){
						return i;
					}

				}else{
					if(delay >= CFG_KEY_LONG){
						delay = 0;
						key[i].status = KEYSTAT_LONG;
						return (100+i);			//������ʼ
					}
				}
				break;

			case KEYSTAT_LONG:
				if(key[i].level != key[i].type){
					key[i].status = KEYSTAT_UP;
					delay = 0;				//��������
				}
				if(delay >= CFG_KEY_PER){
					delay = 0;
					return (100+i);			//�����ڼ�
				}
				break;

			case KEYSTAT_UP:
				if(key[i].level == key[i].type){
					key[i].status = KEYSTAT_DOWN;
					if(CFG_KEY_MODE == 1){
						return (i);					//��������,���·���
					}

				}
				break;
			default:
				key[i].status = KEYSTAT_UP;
				break;
		}
	}

	tv.tv_sec = 0;
	tv.tv_usec = CFG_KEY_SCAN_DELAY*1000;				//��ʱ
	select (0, NULL, NULL, NULL, &tv);
	delay += CFG_KEY_SCAN_DELAY;

	if((timeout <= 0) || (delay < timeout*1000)){		//����ģʽ
		goto scan;
	}else{
		ret = 0;		//��ʱ�޼�����
	}
err:
	return ret;
}


/******************************************************************************
*	����:	key_close
*	����:	����ģ��رպ���
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_NOINIT		-	û�г�ʼ��
*	˵��:�����������󲿷ַ���gpio.h��gpio_close�����Ĵ���
 ******************************************************************************/
//static int key_close(void)
//{
//	int ret = -1;
//
//	if(key_count == 0){
//		return -ERR_NOINIT;
//	}
//
//	ret = gpio_close();
//	if(ret < 0){
//		return ret;
//	}
//	key_count = 0;
//	ret = 0;
//	return ret;
//}
#endif
#endif
