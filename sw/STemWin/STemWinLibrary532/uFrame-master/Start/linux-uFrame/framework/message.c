/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��message.c
	����		�����ļ�ʵ������Ϣ����ģ��
	�汾		��0.1
	����		����迡
	��������	��2010.11
******************************************************************************/

//ҵ��ƽ̨����ͷ�ļ�
#include "framework/config.h"

//C��ͷ�ļ�
#include <pthread.h>

//����ƽ̨ͷ�ļ�
#include "sge_core/msg.h"
#include "sge_core/error.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/message.h"
#include "framework/list.h"

/*************************************************
  �ṹ���Ͷ���
*************************************************/
typedef struct {
	struct list_head list;
	struct BASE *obj;
} sub_node_t;

/*************************************************
  ��̬ȫ�ֱ�������
*************************************************/
static struct {
	pthread_mutex_t mutex;
	struct list_head list_busy;	//��Ч������Ϣ����ͷ
	struct list_head list_idle;	//���ж�����Ϣ����ͷ
	sub_node_t nodepool[CFG_MESSAGE_SUBSCRIBE_MAX];
} subinfo[CFG_MESSAGE_TYPE_MAX];



/*************************************************
  API����ʵ��
*************************************************/
/******************************************************************************
*	����:	message_init
*	����:	��Ϣ����ģ���ʼ��
*	����:
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
			-ERR_BUSY		- 	ģ���ѳ�ʼ��
*	˵��:	�˺�����framework_init���á�
 ******************************************************************************/
int message_init (void)
{
	int ret = 0;
	int i,j;
	//����ƽ̨��Ϣ����ģ���ʼ��
	ret = msg_init();
	if(ret) {
		goto error;
	}
	for (i=0; i<CFG_MESSAGE_TYPE_MAX; i++) {
		//��������ʼ��
		if (pthread_mutex_init(&subinfo[i].mutex, NULL)) {
			ret = -ERR_SYS;
			goto error;
		}
		//�����ʼ���������ж�����Ϣ�ռ���������������
		INIT_LIST_HEAD(&subinfo[i].list_busy);
		INIT_LIST_HEAD(&subinfo[i].list_idle);
		for (j=0; j<CFG_MESSAGE_SUBSCRIBE_MAX; j++) {
			list_add(&subinfo[i].nodepool[j].list, &subinfo[i].list_idle);
		}
	}
error:
	return ret;
}

/******************************************************************************
*	����:	message_subscribe
*	����:	������Ϣ
*	����:	obj				-	ҵ��ģ�����ָ��
			type			-	��Ϣ����ö��ֵ
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		- 	��Ч����
			-ERR_NOMEM		- 	�޿��ö��Ŀռ�
*	˵��:
 ******************************************************************************/
int message_subscribe (struct BASE *obj, u16 type)
{
	int ret = 0;
	sub_node_t *pnode;	//�������Ľṹָ��
	struct list_head *plist_busy;	//æ����ͷ
	struct list_head *plist_idle;	//������ͷ

	//������Ч���ж�
	if ((NULL == obj) || (type >= CFG_MESSAGE_TYPE_MAX)) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��û�����
	if (pthread_mutex_lock (&subinfo[type].mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//д�붩����Ϣ
	plist_busy = &subinfo[type].list_busy;
	plist_idle = &subinfo[type].list_idle;
	if (list_empty(plist_idle)) {
		ret = -ERR_NOMEM;
		goto error1;
	}
	pnode = list_first_entry(plist_idle, sub_node_t, list);
	pnode->obj = obj;
	list_del (&pnode->list);
	list_add (&pnode->list, plist_busy);

error1:
	//����
	if (pthread_mutex_unlock (&subinfo[type].mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	message_unsubscribe
*	����:	ȡ��������Ϣ
*	����:	obj				-	ҵ��ģ�����ָ��
			type			-	��Ϣ����ö��ֵ
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		- 	��Ч����
			-ERR_NORECORD	- 	����Ϣδ����
*	˵��:
 ******************************************************************************/
int message_unsubscribe (struct BASE *obj, u16 type)
{
	int ret = 0;
	int flag = 0;
	sub_node_t *pnode;
	struct list_head *plist_busy;
	struct list_head *plist_idle;

	//������Ч���ж�
	if ((NULL == obj) || (type >= CFG_MESSAGE_TYPE_MAX)) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��û�����
	if (pthread_mutex_lock (&subinfo[type].mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//���Ҳ�ɾ��������Ϣ
	plist_busy = &subinfo[type].list_busy;
	plist_idle = &subinfo[type].list_idle;
	list_for_each_entry(pnode, plist_busy, list) {
		if (pnode->obj == obj) {
			flag = 1;
			break;
		}
	}
	if (flag) {
		list_del (&pnode->list);
		list_add (&pnode->list, plist_idle);
	}
	else {
		ret = -ERR_NORECORD;
		goto error1;
	}
error1:
	//����
	if (pthread_mutex_unlock (&subinfo[type].mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	message_dispatch
*	����:	�ɷ���Ϣ
*	����:	msg				-	��Ϣ����
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		- 	��Ч����
 			-ERR_NOMEM		-	��Ϣ������
*	˵��:
 ******************************************************************************/
int message_dispatch (message_t *msg)
{
	int ret = 0;
	u16 type;
	u8 msgid;	//��Ϣ���к�
	msg_t tmpmsg;		//���ڵײ���Ϣ�������Ϣ����
	sub_node_t *pnode;
	struct list_head *plist_busy;

	type = msg->type;
	//������Ч���ж�
	if (type >= CFG_MESSAGE_TYPE_MAX) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��û�����
	if (pthread_mutex_lock (&subinfo[type].mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//���ݶ�����Ϣ�ɷ���Ϣ
	plist_busy = &subinfo[type].list_busy;
	tmpmsg.type = msg->type;
	tmpmsg.wpara = msg->wpara;
	tmpmsg.lpara = msg->lpara;
	list_for_each_entry(pnode, plist_busy, list) {
		msgid = pnode->obj->thread;
		tmpmsg.priv = (u32)(pnode->obj);
		ret = msg_send(msgid, &tmpmsg, 0);
		if (ret) {
			goto error1;
		}
	}
error1:
	//����
	if (pthread_mutex_unlock (&subinfo[type].mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}
