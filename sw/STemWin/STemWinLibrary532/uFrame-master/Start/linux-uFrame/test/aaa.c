/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��aaa.c
	����		������ҵ��ģ��-ƽ̨ʱ�����
	�汾		��0.1
	����		����迡
	��������	��2010.11
******************************************************************************/

//C��ͷ�ļ�

//����ƽ̨ͷ�ļ�
#include "sge_core/rtc.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/debug.h"
#include "framework/framework.h"
#include "framework/systime.h"
#include "framework/message.h"

//ҵ��ģ��ͷ�ļ�
#include "enum.h"
#include "aaa.h"



int aaa_initmodel(struct BASE * this)
{
	int ret = 0;
	struct aaaModule *obj;
	st_ymdhmsw_t time;

	obj = (struct aaaModule *)this;
//ע��ģ��
	ret = module_register(this);
	if (ret) {
		goto error;
	}
//����ʱ��
/*
	time.year = 10;
	time.mon = 12;
	time.day = 3;
	time.hour = 16;
	time.min = 33;
	time.sec = 00;
	time.wday = 5;
	ret = systime_set(&time);
	if (ret < 0) {
		goto error;
	}
	ret = rtc_settime(&time);
	if (ret < 0) {
		goto error;
	}
*/

//ע��4�����ڶ�ʱ
	ret = systime_register_period(this, 1, 4);
	if (ret < 0) {
		goto error;
	}
	else {
		obj->tmhdr_4s = ret;
	}
//ע��9�뵹��ʱ��ʱ
	ret = systime_register_timer(this, 9, 3);
	if (ret < 0) {
		goto error;
	}
	else {
		obj->tmhdr_9s = ret;
	}
//ע�����ʱ�䵽��
	systime_get (&time);
	time.min += 1;
	ret = systime_register_clock(this, (st_ymdhms_t *)&time);
	if (ret < 0) {
		goto error;
	}
	else {
		obj->tmhdr_clock = ret;
	}

	ret = 0;
error:
	return ret;
}

int aaa_initdata(struct BASE * this)
{
	//...;
	return 0;
}

int aaa_msghandle(struct BASE * this, message_t *msg)
{
	struct aaaModule *obj;
	st_ymdhmsw_t time;

	obj = (struct aaaModule *)this;

	switch (msg->type) {
	case MSG_TIME:
//		if (msg->lpara == obj->tmhdr_4s) {
//			MPRINTF("%s:4s timer!\n", __FILE__);
//		}
//		else if (msg->lpara == obj->tmhdr_9s) {
//			MPRINTF("%s:9s timer!\n", __FILE__);
//		}
//		else if (msg->lpara == obj->tmhdr_clock) {
//			MPRINTF("%s:clock timer!\n", __FILE__);
//		}
//		systime_get (&time);
//		MPRINTF ("%s:time--%02d:%02d:%02d:%02d:%02d:%02d\n", __FILE__, time.year, time.mon, time.day, time.hour, time.min, time.sec);
		break;
	default:
		break;
	}
	return 0;
}

struct BASEFT aaa_ft = {aaa_initmodel, aaa_initdata, aaa_msghandle};
