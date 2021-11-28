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
	�ļ�		��msg.c
	����		�����ļ�ʵ������Ϣ����ģ���е�API����
	�汾		��0.1
	����		����迡
	��������	��2009.12
******************************************************************************/
//������ͷ�ļ�
#include "private/config.h"

//ģ�����ÿ���
#ifdef CFG_MSG_MODULE

//����ͷ�ļ�
#include "private/debug.h"

//��������ͷ�ļ�

//C��ͷ�ļ�
#include <stdio.h>                      //printf
#include <semaphore.h>                  //sem_t
#include <string.h>
#include <pthread.h>                    //pthread_mutex


//�ṩ���û���ͷ�ļ�
#include "include/msg.h"
#include "include/error.h"


/*************************************************
  ��̬ȫ�ֱ�������
*************************************************/
#define CFG_MSG_REALSIZE    (CFG_MSG_SIZE + 1)

typedef struct {
    msg_t msg;
    u8 prio;
} msg_prio_t;

static int msg_inited;

static struct {
    sem_t sem;
    pthread_mutex_t mutex;
    u32 head;
    u32 tail;
    msg_prio_t message[CFG_MSG_REALSIZE];
} msg_info[CFG_MSG_MAX];

/*************************************************
  API����ʵ��
*************************************************/
/******************************************************************************
*	����:	msg_init
*	����:	��Ϣ����ģ���ʼ��
*	����:	��
*	����:	0				-	�ɹ�
 			-ERR_SYS		- 	ϵͳ�쳣
 			-ERR_BUSY		- 	ģ���ѳ�ʼ��
*	˵��:
 ******************************************************************************/
int msg_init(void)
{
    int ret = 0;
    int i;

    if (0 != msg_inited) {
        ret = -ERR_BUSY;
        goto error;
    }
//��ʼ���ź����ͻ�����
    for (i=0; i<CFG_MSG_MAX; i++) {
        if (sem_init(&msg_info[i].sem, 0, 0)) {
            ret = -ERR_SYS;
            goto error;
        }
        if (pthread_mutex_init(&msg_info[i].mutex, NULL)) {
            ret = -ERR_SYS;
            goto error;
        }
    }
    msg_inited = 1;
error:
    return ret;
}

/******************************************************************************
*	����:	msg_send
*	����:	������Ϣ
*	����:	id				-	��Ϣ���к�
 			msg				-	��Ϣ�����ݴ��룩
 			prio			-	���ȼ���0Ϊ��ͣ�99Ϊ��ߣ�
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ�쳣
 			-ERR_NOINIT		-	ģ��δ��ʼ��
 			-ERR_NODEV		-	�޴���Ϣ����
 			-ERR_INVAL		-	��������
 			-ERR_NOMEM		-	��Ϣ������
*	˵��:
 ******************************************************************************/
int msg_send(u8 id, msg_t *msg, u8 prio)
{
    int ret = 0;

    if (0 == msg_inited) {
        ret = -ERR_NOINIT;
        goto error;
    }
    if (id > CFG_MSG_MAX - 1) {
        ret = -ERR_NODEV;
        goto error;
    }
    if (prio > MSG_PRIO_MAX) {
        ret = -ERR_INVAL;
        goto error;
    }
//��û�����
    if (pthread_mutex_lock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
//�ж���Ϣ�����Ƿ���
    if (msg_info[id].head == (msg_info[id].tail + 1) % CFG_MSG_REALSIZE) {
        ret = -ERR_NOMEM;
        goto error1;
    }
//д����Ϣ
    memcpy(&msg_info[id].message[msg_info[id].tail].msg, msg, sizeof(msg_t));
    msg_info[id].message[msg_info[id].tail].prio = prio;
//����β+1
    msg_info[id].tail = (msg_info[id].tail + 1) % CFG_MSG_REALSIZE;
error1:
//����
    if (pthread_mutex_unlock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
//�����ź���
    if (sem_post (&msg_info[id].sem)) {
        ret = -ERR_SYS;
        goto error;
    }
error:
    return ret;
}

/******************************************************************************
*	����:	msg_recv
*	����:	������Ϣ
*	����:	id				-	��Ϣ���к�
 			msg				-	��Ϣ�����ݴ�����
 			timeout			-	��ʱʱ�䣨0Ϊ���õȴ���0xffffΪ���ȴ�������Ϊ�ȴ�������
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ�쳣
 			-ERR_NODEV		-	�޴���Ϣ����
 			-ERR_NOINIT		-	ģ��δ��ʼ��
 			-ERR_TIMEOUT	-	��ʱδ���յ���Ϣ
*	˵��:
 ******************************************************************************/
int msg_recv(u8 id, msg_t *msg, u16 timeout)
{
    int ret = 0;
//�ȴ�ʱ��
    struct timespec to;

    if (0 == msg_inited) {
        ret = -ERR_NOINIT;
        goto error;
    }
    if (id > CFG_MSG_MAX - 1) {
        ret = -ERR_NODEV;
        goto error;
    }
//�ȴ��ź���
    if (MSG_RECV_BLOCK == timeout) {
        if (sem_wait (&msg_info[id].sem)) {
            ret = -ERR_SYS;
            goto error;
        }
    }
    else if (MSG_RECV_NONBLOCK == timeout) {
        if (sem_trywait (&msg_info[id].sem)) {
            ret = -ERR_TIMEOUT;             //����Ϣ
            goto error;
        }
    }
    else {
        to.tv_sec = time(NULL) + timeout;
        to.tv_nsec = 0;
        if (sem_timedwait (&msg_info[id].sem, &to)) {
            ret = -ERR_TIMEOUT;
            goto error;
        }
    }

//��û�����
    if (pthread_mutex_lock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
//�ж϶����Ƿ�Ϊ��
    if (msg_info[id].head == msg_info[id].tail) {
        ret = -ERR_SYS;
        goto error1;
    }
//��ȡ��Ϣ
    memcpy(msg, &msg_info[id].message[msg_info[id].head].msg, sizeof(msg_t));
//����ͷ+1
    msg_info[id].head = (msg_info[id].head + 1) % CFG_MSG_REALSIZE;

error1:
//����
    if (pthread_mutex_unlock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
error:
    return ret;
}

/******************************************************************************
*	����:	msg_recv_prio
*	����:	����������Ϣ
*	����:	id				-	��Ϣ���к�
 			msg				-	��Ϣ�����ݴ�����
 			timeout			-	��ʱʱ�䣨0Ϊ���õȴ���0xffffΪ���ȴ�������Ϊ�ȴ�������
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ�쳣
 			-ERR_NODEV		-	�޴���Ϣ����
 			-ERR_NOINIT		-	ģ��δ��ʼ��
 			-ERR_TIMEOUT	-	��ʱδ���յ���Ϣ
*	˵��:
 ******************************************************************************/
int msg_recv_prio(u8 id, msg_t *msg, u16 timeout)
{
    int ret = 0;
    int i;
//Ѱ�����ȼ���ߵ���Ϣ
    u32 index, head, tail;
    s8 prio = -1;
//�ȴ�ʱ��
    struct timespec to;

    if (0 == msg_inited) {
        ret = -ERR_NOINIT;
        goto error;
    }
    if (id > CFG_MSG_MAX - 1) {
        ret = -ERR_NODEV;
        goto error;
    }
//�ȴ��ź���
    if (MSG_RECV_BLOCK == timeout) {
        if (sem_wait (&msg_info[id].sem)) {
            ret = -ERR_SYS;
            goto error;
        }
    }
    else if (MSG_RECV_NONBLOCK == timeout) {
        if (sem_trywait (&msg_info[id].sem)) {
            ret = -ERR_TIMEOUT;             //����Ϣ
            goto error;
        }
    }
    else {
        to.tv_sec = time(NULL) + timeout;
        to.tv_nsec = 0;
        if (sem_timedwait (&msg_info[id].sem, &to)) {
            ret = -ERR_TIMEOUT;
            goto error;
        }
    }

//��û�����
    if (pthread_mutex_lock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
    head = msg_info[id].head;
    tail = msg_info[id].tail;
    index = head;
//�ж϶����Ƿ�Ϊ��
    if (head == tail) {
        ret = -ERR_SYS;
        goto error1;
    }
//Ѱ�����ȼ���ߵ���Ϣ��indexΪ�ҵ�����Ϣ
    i = head;
    do {
    	if ((s8)(msg_info[id].message[i].prio) > prio) {
            prio = (s8)(msg_info[id].message[i].prio);
            index = i;
        }
        i = (i + 1) % CFG_MSG_REALSIZE;
    }
    while(i != tail);
//������Ϣ�����к�ʣ�����Ϣ
    memcpy(msg, &msg_info[id].message[index].msg, sizeof(msg_t));
    if (index == head) {
        msg_info[id].head = (head + 1) % CFG_MSG_REALSIZE;
    }
    else if (index > head) {
        memmove(&msg_info[id].message[head] + 1, &msg_info[id].message[head], (index - head) * sizeof(msg_prio_t));
    	msg_info[id].head = (head + 1) % CFG_MSG_REALSIZE;
    }
    else {
        memmove(&msg_info[id].message[index], &msg_info[id].message[index] + 1, (tail - index - 1) * sizeof(msg_prio_t));
        if(0 == tail) {
            msg_info[id].tail = CFG_MSG_REALSIZE - 1;
        }
        else {
            msg_info[id].tail = tail - 1;
        }
    }
error1:
//����
    if (pthread_mutex_unlock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
error:
    return ret;
}

/******************************************************************************
*	����:	msg_clear
*	����:	�����Ϣ
*	����:	id				-	��Ϣ���к�
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ�쳣
 			-ERR_NODEV		-	�޴���Ϣ����
 			-ERR_NOINIT		-	ģ��δ��ʼ��
*	˵��:
 ******************************************************************************/
int msg_clear(u8 id)
{
    int ret = 0;

    if (0 == msg_inited) {
        ret = -ERR_NOINIT;
        goto error;
    }
    if (id > CFG_MSG_MAX - 1) {
        ret = -ERR_NODEV;
        goto error;
    }
//��λ�ź���
    if (sem_init(&msg_info[id].sem, 0, 0)) {
		ret = -ERR_SYS;
		goto error;
	}
//��û�����
    if (pthread_mutex_lock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
//�����Ϣ����
    msg_info[id].head = msg_info[id].tail;
//����
    if (pthread_mutex_unlock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
error:
    return ret;
}

/******************************************************************************
*	����:	msg_getsize
*	����:	�����Ϣ����
*	����:	id				-	��Ϣ���к�
*	����:	>0				-	��Ϣ����
 			-ERR_SYS		-	ϵͳ�쳣
 			-ERR_NODEV		-	�޴���Ϣ����
 			-ERR_NOINIT		-	ģ��δ��ʼ��
*	˵��:
 ******************************************************************************/
int msg_getsize(u8 id)
{
    int ret = 0;
    int size;

    if (0 == msg_inited) {
        ret = -ERR_NOINIT;
        goto error;
    }
    if (id > CFG_MSG_MAX - 1) {
        ret = -ERR_NODEV;
        goto error;
    }
//��û�����
    if (pthread_mutex_lock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
//�����Ϣ���г���
    size = (int)msg_info[id].tail - (int)msg_info[id].head;
    if(size < 0) {
        size = size + CFG_MSG_REALSIZE;
    }
//����
    if (pthread_mutex_unlock (&msg_info[id].mutex)) {
        ret = -ERR_SYS;
        goto error;
    }
    ret = size;
error:
    return ret;
}

#endif /* CFG_MSG_MODULE */
