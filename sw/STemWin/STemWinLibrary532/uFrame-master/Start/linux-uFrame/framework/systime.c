/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��systime.c
	����		�����ļ�ʵ����ϵͳʱ��ģ��
	�汾		��0.1
	����		����迡
	��������	��2010.11
******************************************************************************/

//ҵ��ƽ̨����ͷ�ļ�
#include "framework/config.h"

//C��ͷ�ļ�
#include <pthread.h>
#include <string.h>

//����ƽ̨ͷ�ļ�
#include "sge_core/msg.h"
#include "sge_core/rtc.h"
#include "sge_core/timer.h"
#include "sge_core/thread.h"
#include "sge_core/error.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/debug.h"
#include "framework/systime.h"
#include "framework/message.h"
#include "framework/list.h"

/*************************************************
  �궨��
*************************************************/
//�Ƿ��λ��ʶ
#define FLAG_SEC	(1 << 0)
#define FLAG_MIN	(1 << 1)
#define FLAG_HOUR	(1 << 2)
#define FLAG_DAY    (1 << 3)
#define FLAG_MON    (1 << 4)
#define FLAG_YEAR   (1 << 5)

/*************************************************
  �ṹ���Ͷ���
*************************************************/
typedef struct {
	struct list_head list;
	struct BASE *obj;
	int handle;
	union {
		u8 period;
		st_ymdhms_t clock;
		struct {
			u32 sec;
			u32 cnt;
			u8 num;
		} timer;
	} value;
} timer_node_t;


/*************************************************
  ��̬ȫ�ֱ�������
*************************************************/
//ϵͳʱ��
static struct {
	pthread_mutex_t mutex;
	st_ymdhmsw_t time;
} systime;

//ʱ��ע����Ϣ
static pthread_mutex_t timer_mutex;
static timer_node_t nodepool[CFG_SYSTIME_TIMER_MAX];
static struct list_head list_period_sec;		//������������ͷ
static struct list_head list_period_min;		//������������ͷ
static struct list_head list_period_hour;		//ʱ����������ͷ
static struct list_head list_period_day;		//������������ͷ
static struct list_head list_period_mon;		//������������ͷ
static struct list_head list_period_year;		//������������ͷ
static struct list_head list_clock;				//����ʱ�̵���������ͷ
static struct list_head list_timer;				//����ʱ������ͷ
static struct list_head list_idle;				//��������ͷ

//ÿ���µ��������·ݵĶ�Ӧ��ϵ��
static u8 monthday[2][13] = {
	[0] = {0xff, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	[1] = {0xff, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

/*************************************************
  API����ʵ��
*************************************************/
static void systime_add_sec (u8 *flag)
{
	*flag = 0;
	systime.time.sec++;
	*flag = *flag | FLAG_SEC;
	if (systime.time.sec > 59) {
		systime.time.sec = 0;
		systime.time.min++;
		*flag = *flag | FLAG_MIN;
		if (systime.time.min > 59) {
			systime.time.min = 0;
			systime.time.hour++;
			*flag = *flag | FLAG_HOUR;
			if (systime.time.hour > 23) {
				systime.time.hour = 0;
				systime.time.day++;
				systime.time.wday++;
				*flag = *flag | FLAG_DAY;
				if (systime.time.wday > 7) {
					systime.time.wday = 1;
				}
				if (systime.time.day > monthday[(!(systime.time.year % 4)) && (systime.time.year % 100)][systime.time.mon]) {
					systime.time.day = 1;
					systime.time.mon++;
					*flag = *flag | FLAG_MON;
					if (systime.time.mon > 12) {
						systime.time.mon = 1;
						systime.time.year++;
						*flag = *flag | FLAG_YEAR;
					}
				}
			}
		}
	}
}

//ʱ������̺߳���
static void* systime_thread (void *arg)
{
	int ret;
	u8 flag;		//ʱ���λ��־
	timer_node_t *pnode;
	timer_node_t *pnode_del;
	msg_t tmpmsg;

	tmpmsg.type = MSG_TIME;

	//��ʼ����ʱ��
	ret = timer_init (TIMER0, TIMER_MODE_HEART);
	if (ret) {
		goto error;
	}
	ret = timer_heart_setconfig (TIMER0, 1000);	//1000ms��ʱ
	if (ret) {
		goto error;
	}
	ret = timer_heart_start(TIMER0);
	if (ret) {
		goto error;
	}
	//�߳�ѭ����ʼ
	while(1) {
		//�ȴ���ʱ��
		ret = timer_heart_wait(TIMER0);
		if (ret) {
			goto error;
		}
		//���ϵͳʱ����
		if (pthread_mutex_lock (&systime.mutex)) {
			ret = -ERR_SYS;
			goto error;
		}
		//����ϵͳʱ�Ӳ����ʱ���λ��־
		systime_add_sec(&flag);
		//���ʱ����Ϣע����
		if (pthread_mutex_lock (&timer_mutex)) {
			ret = -ERR_SYS;
			goto error;
		}
		//����ʱ����Ϣ
		if (flag & FLAG_SEC) {
			//������ʱ����Ϣ
			list_for_each_entry(pnode, &list_period_sec, list) {
				if (0 == (systime.time.sec % pnode->value.period)) {
					tmpmsg.lpara = pnode->handle;
					tmpmsg.priv = (u32)(pnode->obj);
					ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
					if (ret) {
						goto error;
					}
				}
			}
			//����ʱ�䵽����Ϣ
			pnode_del = NULL;
			list_for_each_entry(pnode, &list_clock, list) {
				if (pnode_del) {
					list_del (&pnode_del->list);
					list_add (&pnode_del->list, &list_idle);
				}
				if (0 == memcmp((char *)&systime.time, (char *)&pnode->value.clock, 6)) {//st_ymdhms_t����Ϊ8�ֽں���2�������ȱ
					tmpmsg.lpara = pnode->handle;
					tmpmsg.priv = (u32)(pnode->obj);
					ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
					if (ret) {
						goto error;
					}
					pnode_del = pnode;
				}
			}
			if (pnode_del) {
				list_del (&pnode_del->list);
				list_add (&pnode_del->list, &list_idle);
			}
			//����ʱ������Ϣ
			pnode_del = NULL;
			list_for_each_entry(pnode, &list_timer, list) {
				if (pnode_del) {
					list_del (&pnode_del->list);
					list_add (&pnode_del->list, &list_idle);
				}
				pnode->value.timer.cnt--;
				if (0 == pnode->value.timer.cnt) {
					tmpmsg.lpara = pnode->handle;
					tmpmsg.priv = (u32)(pnode->obj);
					ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
					if (ret) {
						goto error;
					}
					//numΪ0xFF��ʾ�����ƴ�������ʱ
					if (0xFF != pnode->value.timer.num) {
						pnode->value.timer.num--;
						if (0 == pnode->value.timer.num) {
							pnode_del = pnode;
						}
						else {
							pnode->value.timer.cnt = pnode->value.timer.sec;
						}
					}
				}
			}
			if (pnode_del) {
				list_del (&pnode_del->list);
				list_add (&pnode_del->list, &list_idle);
			}
			//������ʱ����Ϣ
			if (flag & FLAG_MIN) {
				list_for_each_entry(pnode, &list_period_min, list) {
					if (0 == (systime.time.min % pnode->value.period)) {
						tmpmsg.lpara = pnode->handle;
						tmpmsg.priv = (u32)(pnode->obj);
						ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
						if (ret) {
							goto error;
						}
					}
				}
				//ʱ����ʱ����Ϣ
				if (flag & FLAG_HOUR) {
					list_for_each_entry(pnode, &list_period_hour, list) {
						if (0 == (systime.time.hour % pnode->value.period)) {
							tmpmsg.lpara = pnode->handle;
							tmpmsg.priv = (u32)(pnode->obj);
							ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
							if (ret) {
								goto error;
							}
						}
					}
					//������ʱ����Ϣ
					if (flag & FLAG_DAY) {
						list_for_each_entry(pnode, &list_period_day, list) {
							tmpmsg.lpara = pnode->handle;
							tmpmsg.priv = (u32)(pnode->obj);
							ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
							if (ret) {
								goto error;
							}
						}
						//������ʱ����Ϣ
						if (flag & FLAG_MON) {
							list_for_each_entry(pnode, &list_period_mon, list) {
								tmpmsg.lpara = pnode->handle;
								tmpmsg.priv = (u32)(pnode->obj);
								ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
								if (ret) {
									goto error;
								}
							}
							//������ʱ����Ϣ
							if (flag & FLAG_YEAR) {
								list_for_each_entry(pnode, &list_period_year, list) {
									tmpmsg.lpara = pnode->handle;
									tmpmsg.priv = (u32)(pnode->obj);
									ret = msg_send(pnode->obj->thread, &tmpmsg, 0);
									if (ret) {
										goto error;
									}
								}
							}
						}
					}
				}
			}
		}
		//��ʱ����Ϣע����
		if (pthread_mutex_unlock (&timer_mutex)) {
			ret = -ERR_SYS;
			goto error;
		}
		//��ϵͳʱ����
		if (pthread_mutex_unlock (&systime.mutex)) {
			ret = -ERR_SYS;
			goto error;
		}
	}
error:
	while(1);
}

/******************************************************************************
*	����:	systime_init
*	����:	ϵͳʱ��ģ���ʼ��
*	����:
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
			-ERR_BUSY		- 	ģ���ѳ�ʼ��
			-ERR_INVAL		-	��������
			-ERR_NODEV		-	�޴��豸
*	˵��:	�˺�����framework_init���á�
 ******************************************************************************/
int systime_init (void)
{
	int ret = 0;
	int i;
	pthread_t tid;
	st_ymdhmsw_t tmptime;
//��һ����ϵͳʱ���ʼ��
	if (pthread_mutex_init(&systime.mutex, NULL)) {
		ret = -ERR_SYS;
		goto error;
	}
	//��ȡRTCʱ�䲢��ʼ��ϵͳʱ��
	ret = rtc_init();
	if (ret) {
		goto error;
	}
	ret = rtc_gettime (&tmptime);
	if (ret) {
		goto error;
	}
	ret = systime_set (&tmptime);
	if (ret) {
		goto error;
	}
//�ڶ�����ʱ����Ϣע����Ƴ�ʼ��
	if (pthread_mutex_init(&timer_mutex, NULL)) {
		ret = -ERR_SYS;
		goto error;
	}
	INIT_LIST_HEAD(&list_period_sec);
	INIT_LIST_HEAD(&list_period_min);
	INIT_LIST_HEAD(&list_period_hour);
	INIT_LIST_HEAD(&list_period_day);
	INIT_LIST_HEAD(&list_period_mon);
	INIT_LIST_HEAD(&list_period_year);
	INIT_LIST_HEAD(&list_clock);
	INIT_LIST_HEAD(&list_timer);
	INIT_LIST_HEAD(&list_idle);
	for (i=0; i<CFG_SYSTIME_TIMER_MAX; i++) {
		nodepool[i].handle = i;
		list_add(&nodepool[i].list, &list_idle);
	}
//������������ʱ������߳�
	ret = thread_create_base (&tid, systime_thread, NULL, THREAD_MODE_REALTIME, 90);
	if (ret) {
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	systime_get
*	����:	��ȡϵͳʱ��
*	����:	time			-	ʱ�䣨���ݴ�����
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
*	˵��:
 ******************************************************************************/
int systime_get (st_ymdhmsw_t *time)
{
	int ret = 0;
	//��û�����
	if (pthread_mutex_lock (&systime.mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	memcpy ((char *)time, (char *)&systime.time, sizeof(st_ymdhmsw_t));
	//����
	if (pthread_mutex_unlock (&systime.mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	systime_set
*	����:	����ϵͳʱ��
*	����:	time			-	ʱ�䣨���ݴ��룩
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	��������
*	˵��:
 ******************************************************************************/
int systime_set (st_ymdhmsw_t *time)
{
	int ret = 0;

	//������Ч���ж�
	if ((time->mon < 1)||(time->mon > 12)||(time->day < 1)||(time->day > 31)||(time->hour > 23)||(time->min > 59)||(time->sec > 59)||(time->wday < 1)||(time->wday >7)) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��û�����
	if (pthread_mutex_lock (&systime.mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//����ʱ��
	memcpy ((char *)&systime.time, (char *)time, sizeof(st_ymdhmsw_t));
	//����
	if (pthread_mutex_unlock (&systime.mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	systime_register_period
*	����:	ע�������Զ�ʱ��Ϣ
*	����:	obj				-	ҵ��ģ�����ָ��
			type			-	�������ͣ��롢�֡���...��
			period			-	����
*	����:	>=0				-	ʱ����Ϣ���
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	��������
			-ERR_NOMEM		-	�޿���ע��ռ�
*	˵��:
 ******************************************************************************/
int systime_register_period (struct BASE *obj, u8 type, u8 period)
{
	int ret = 0;
	timer_node_t *pnode;
	//��û�����
	if (pthread_mutex_lock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//��ȡ���ж�����һ���ڵ�
	if (list_empty(&list_idle)) {
		ret = -ERR_NOMEM;
		goto error1;
	}
	pnode = list_first_entry(&list_idle, timer_node_t, list);
	//���ݲ�ͬ�������ʱ����Ϣע����Ϣ
	switch (type) {
	case SYSTIME_REGISTER_PERIOD_SEC:
		if ((period < 1) || (period > 59) || (0 != 60%period)) {
			ret = -ERR_INVAL;
			goto error1;
		}
		pnode->obj = obj;
		pnode->value.period = period;
		list_del (&pnode->list);
		list_add (&pnode->list, &list_period_sec);
		break;
	case SYSTIME_REGISTER_PERIOD_MIN:
		if ((period < 1) || (period > 59) || (0 != 60%period)) {
			ret = -ERR_INVAL;
			goto error1;
		}
		pnode->obj = obj;
		pnode->value.period = period;
		list_del (&pnode->list);
		list_add (&pnode->list, &list_period_min);
		break;
	case SYSTIME_REGISTER_PERIOD_HOUR:
		if ((period < 1) || (period > 23) || (0 != 24%period)) {
			ret = -ERR_INVAL;
			goto error1;
		}
		pnode->obj = obj;
		pnode->value.period = period;
		list_del (&pnode->list);
		list_add (&pnode->list, &list_period_hour);
		break;
	case SYSTIME_REGISTER_PERIOD_DAY:
		if (period != 1) {
			ret = -ERR_INVAL;
			goto error1;
		}
		pnode->obj = obj;
		pnode->value.period = period;
		list_del (&pnode->list);
		list_add (&pnode->list, &list_period_day);
		break;
	case SYSTIME_REGISTER_PERIOD_MON:
		if (period != 1) {
			ret = -ERR_INVAL;
			goto error1;
		}
		pnode->obj = obj;
		pnode->value.period = period;
		list_del (&pnode->list);
		list_add (&pnode->list, &list_period_mon);
		break;
	case SYSTIME_REGISTER_PERIOD_YEAR:
		if (period != 1) {
			ret = -ERR_INVAL;
			goto error1;
		}
		pnode->obj = obj;
		pnode->value.period = period;
		list_del (&pnode->list);
		list_add (&pnode->list, &list_period_year);
		break;
	default:
		ret = -ERR_INVAL;
		goto error1;
	}
	ret = pnode->handle;
error1:
	//����
	if (pthread_mutex_unlock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	systime_register_clock
*	����:	ע��ʱ�䵽����Ϣ
*	����:	obj				-	ҵ��ģ�����ָ��
			time			-	����ʱ�䣨���ݴ��룩
*	����:	>=0				-	ʱ����Ϣ���
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	��������
			-ERR_NOMEM		-	�޿���ע��ռ�
*	˵��:
 ******************************************************************************/
int systime_register_clock (struct BASE *obj, st_ymdhms_t *time)
{
	int ret = 0;
	timer_node_t *pnode;

	//������Ч���ж�
	if ((time->mon < 1)||(time->mon > 12)||(time->day < 1)||(time->day > 31)||(time->hour > 23)||(time->min > 59)||(time->sec > 59)) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��û�����
	if (pthread_mutex_lock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//��ȡ���ж�����һ���ڵ�
	if (list_empty(&list_idle)) {
		ret = -ERR_NOMEM;
		goto error1;
	}
	pnode = list_first_entry(&list_idle, timer_node_t, list);
	//���ʱ����Ϣע����Ϣ
	memcpy ((char *)&pnode->value, (char *)time, sizeof(st_ymdhms_t));
	pnode->obj = obj;
	list_del (&pnode->list);
	list_add (&pnode->list, &list_clock);
	ret = pnode->handle;
error1:
	//����
	if (pthread_mutex_unlock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	systime_register_timer
*	����:	ע�ᵹ��ʱ������Ϣ
*	����:	obj				-	ҵ��ģ�����ָ��
			sec				-	����ʱ��������
			num				-	����ʱ������0xff��ʾ���޴Σ�
*	����:	>=0				-	ʱ����Ϣ���
 			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	��������
			-ERR_NOMEM		-	�޿���ע��ռ�
*	˵��:
 ******************************************************************************/
int systime_register_timer (struct BASE *obj, u32 sec, u8 num)
{
	int ret = 0;
	timer_node_t *pnode;

	//������Ч���ж�
	if ((0 == sec) || (0 == num)) {
		ret = -ERR_INVAL;
		goto error;
	}
	//��û�����
	if (pthread_mutex_lock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//��ȡ���ж�����һ���ڵ�
	if (list_empty(&list_idle)) {
		ret = -ERR_NOMEM;
		goto error1;
	}
	pnode = list_first_entry(&list_idle, timer_node_t, list);
	//���ʱ����Ϣע����Ϣ
	pnode->value.timer.sec = sec;
	pnode->value.timer.cnt = sec;
	pnode->value.timer.num = num;
	pnode->obj = obj;
	list_del (&pnode->list);
	list_add (&pnode->list, &list_timer);
	ret = pnode->handle;
error1:
	//����
	if (pthread_mutex_unlock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}

/******************************************************************************
*	����:	systime_unregister
*	����:	ȡ��ʱ����Ϣע��
*	����:	handle			-	ʱ����Ϣ���
*	����:	0				-	�ɹ�
 			-ERR_SYS		-	ϵͳ����
*	˵��:
 ******************************************************************************/
int systime_unregister (int handle)
{
	int ret = 0;
	timer_node_t *pnode;

	//��û�����
	if (pthread_mutex_lock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
	//�õ���ȡ���Ľڵ�
	pnode = &nodepool[handle];
	//���ʱ����Ϣע����Ϣ
	list_del (&pnode->list);
	list_add (&pnode->list, &list_idle);
	//����
	if (pthread_mutex_unlock (&timer_mutex)) {
		ret = -ERR_SYS;
		goto error;
	}
error:
	return ret;
}
