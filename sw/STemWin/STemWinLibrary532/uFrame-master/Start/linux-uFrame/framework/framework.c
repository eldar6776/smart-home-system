/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��framework.c
	����		�����ļ�ʵ���˿�ܳ�ʼ���ӿ�
	�汾		��0.1
	����		����迡
	��������	��2010.12
******************************************************************************/

//ҵ��ƽ̨����ͷ�ļ�
#include "framework/config.h"

//C��ͷ�ļ�

//����ƽ̨ͷ�ļ�
#include "sge_core/msg.h"
#include "sge_core/thread.h"
#include "sge_core/error.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/debug.h"
#include "framework/framework.h"
#include "framework/systime.h"
#include "framework/message.h"
#include "framework/dbserver.h"

/*************************************************
  �궨��
*************************************************/


/*************************************************
  �ṹ���Ͷ���
*************************************************/



/*************************************************
  ��̬ȫ�ֱ�������
*************************************************/
static struct {
	u8 state;
	u8 prio;
} thread_createinfo[CFG_FRAMEWORK_THREAD_MAX];



/*************************************************
  API����ʵ��
*************************************************/
//ƽ̨�̺߳���
static void* framework_thread (void *arg)
{
	u8 id;
	msg_t msg;
	struct BASE *obj;

	id = *(u8 *)arg;
	while(1) {
		if (msg_recv (id, &msg, 0)) {
			//������
		}
		//dispatch����Ϣ����
		if (0 != msg.priv) {
			obj = (struct BASE *)msg.priv;
			obj->baseft->msghandle (obj, (message_t *)&msg);
		}
		//��dispatch����Ϣpriv����Ϊ0
		else {
			//ֱ�ӷ��͵���Ϣ����Ŀǰ����֧�֣�ֱ�Ӷ���
		}
	}
}

/******************************************************************************
*	����:	framework_init
*	����:	ƽ̨��ܳ�ʼ����ʱ�������Ϣ���ȡ����ݷ��ʣ��豸���ֵĳ�ʼ����ҵ��ģ����ã�
*	����:
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
 			-ERR_BUSY		- 	ģ���ѳ�ʼ��
			-ERR_NODEV		-	�޴��豸
*	˵��:	�˺�����main.c���á�
 ******************************************************************************/
int framework_init (void)
{
	int ret = 0;

	//ʱ�����ģ��
	ret = systime_init();
	if (ret) {
		goto error;
	}
	//��Ϣ����ģ��
	ret = message_init();
	if (ret) {
		goto error;
	}
#if 0
	//���ݷ���ģ��
	ret = dbserver_init();
	if (ret) {
		goto error;
	}
#endif

error:
	return ret;
}


/******************************************************************************
*	����:	framework_start
*	����:	ƽ̨���������������ҵ��ģ���Ӧ���̣߳�
*	����:
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
*	˵��:	�˺�����main.c���á�
 ******************************************************************************/
static u8 msgids[CFG_FRAMEWORK_THREAD_MAX]={0};
int framework_start (void)
{
	int ret = 0;
	int i;

	//����ҵ��ģ���ע���������ҵ��ģ���߳�
	for (i=0; i<CFG_FRAMEWORK_THREAD_MAX; i++) {
		//״̬Ϊ1��ʾ��Ҫ�����߳�
		if (1 == thread_createinfo[i].state) {
			msgids[i]= i;
			//ʵʱ�߳�
			if (0 != thread_createinfo[i].prio){
				ret = thread_create(i, framework_thread, &msgids[i], THREAD_MODE_REALTIME, thread_createinfo[i].prio);
				if (ret) {
					goto error;
				}
			}
			//��ͨ�߳�
			else {
				ret = thread_create(i, framework_thread, &msgids[i], THREAD_MODE_NORMAL, 0);
				if (ret) {
					goto error;
				}
			}
		}
	}

error:
	return ret;
}


/******************************************************************************
*	����:	module_register
*	����:	ҵ��ģ��ע�ᣨΪ�˻��ҵ��ģ��������߳���Ϣ��
*	����:	obj				-	ҵ��ģ�����ָ��
*	����:	0				-	�ɹ�
 			-ERR_INVAL		-	��������
*	˵��:
 ******************************************************************************/
int module_register (struct BASE *obj)
{
	int ret = 0;
	//������Ч���ж�
	if ((obj->thread >= CFG_FRAMEWORK_THREAD_MAX) || (obj->prio > 99)) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��������߳���Ϣ
	if (thread_createinfo[obj->thread].state == 0) {
		thread_createinfo[obj->thread].prio = obj->prio;
		thread_createinfo[obj->thread].state = 1;
	}
	else {
		if (obj->prio > thread_createinfo[obj->thread].prio) {
			thread_createinfo[obj->thread].prio = obj->prio;
		}
	}
error:
	return ret;
}

	
	
	
	
	
	
