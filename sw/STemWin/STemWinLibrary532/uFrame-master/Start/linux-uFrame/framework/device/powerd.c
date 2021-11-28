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
	�ļ�		��  powercheck.c
	����		��  ���ļ������˵����ϵ���ӿ�
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2010.01
******************************************************************************/
//������ͷ�ļ�
#include "framework/config.h"

//ģ�����ÿ���
#ifdef CFG_POWERD_DEVICE

//C��ͷ�ļ�
#include <stdio.h>
//#include <fcntl.h> 		//ioctl
//#include <string.h> 		//memset
//#include <unistd.h>		//close
//#include <signal.h>
//#include <sys/ioctl.h>

//����ƽ̨ͷ�ļ�
#include "sge_core/powercheck.h"
#include "sge_core/error.h"
#include "sge_core/pinio.h"
#include "sge_core/thread.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/device/powerd.h"
#include "framework/message.h"
#include "framework/base.h"

/*************************************************
  ��̬ȫ�ֱ������궨��
*************************************************/

/*************************************************
  API����ʵ��
*************************************************/
/******************************************************************************
*	����:	thread_power_detect
*	����:	ɨ�谴�������߳�
*	����:
*	����:
*	˵��:
 ******************************************************************************/
static void * thread_power_detect(void * arg)
{
	int ret=-1;
	message_t msg_power = {
		.type = MSG_POWER,
	};
	//�������Ϊperiod����������Ϊlast�ĵ�ƽ
	while(1){
		ret = powercheck_check();
		if(ret == 0){
			msg_power.wpara 	= 	POWER_DOWN;
			message_dispatch(&msg_power);
		}else if(ret == 1){
			msg_power.wpara 	= 	POWER_UP;
			message_dispatch(&msg_power);
		}
	}
	pthread_exit(0);
}
/******************************************************************************
*	����:	powerd_init
*	����:	�������豸��ʼ��
*	����:	mode			-	������ģʽ
*	����:	0				-	�ɹ�
			-ERR_NOFILE		-	û�д�·��
			-ERR_BUSY		-	�豸æ���Ѿ���
			-ERR_SYS		-	��ʼ����ʧ�ܣ����ڴ治�㣻�Ѿ���ʼ��
*	˵��:	��
 ******************************************************************************/
int powerd_init ()
{
	int ret = -1;
	pthread_t powerd_id;

	ret = powercheck_init(POWERCHECK_MODE_BLOCK_UPDOWN);
//	printf("%s,%s,%d:ret = %d!\n",__FILE__,__FUNCTION__, __LINE__,ret);
	if(ret < 0 ){
		goto err;
	}

	ret = powercheck_setwaittime(0);
	if(ret < 0 ){
		goto err;
	}

	//�����������̣߳�ʵʱ�̣߳����ȼ�80-90
	ret = thread_create_base(&powerd_id, thread_power_detect, NULL, THREAD_MODE_REALTIME, 89);
	if((ret < 0) ){
		goto err;
	}
err:
	return ret;
}

#endif
