/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��base.h
	����		�����ļ�������ҵ��ģ�����
	�汾		��0.1
	����		����迡
	��������	��2010.11
******************************************************************************/

#ifndef _BASE_H
#define _BASE_H

#include "sge_core/typedef.h"

/*************************************************
  �ṹ���Ͷ���
*************************************************/
//��Ϣ����
typedef struct{
    u16  type;             //��Ϣ����
    u16  wpara;            //��Ϣ����
    u32  lpara;            //��Ϣ������Ϣ
} message_t;

struct BASE;

//���෽��
struct BASEFT
{
	int (*initmodel)(struct BASE * this);						//��ʼ��ģ��ģ��
	int (*initdata)(struct BASE * this);						//��ʼ��ģ������
	int (*msghandle)(struct BASE * this, message_t *msg);		//ģ�����Ϣ������
};

//���ඨ��
struct BASE
{
	struct BASEFT *baseft;				//���෽��
	u8 thread;							//ģ���Ӧ�̺߳�
	u8 prio;							//ģ�����ȼ�
};

#endif
