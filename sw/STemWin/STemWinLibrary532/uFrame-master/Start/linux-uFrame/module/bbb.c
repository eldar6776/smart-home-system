/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��bbb.c
	����		������ҵ��ģ��-״̬��
	�汾		��0.1
	����		����迡
	��������	��2010.12
******************************************************************************/

//C��ͷ�ļ�

//����ƽ̨ͷ�ļ�
#include "sge_core/rtc.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/debug.h"
#include "framework/framework.h"
#include "framework/systime.h"
#include "framework/message.h"
#include "framework/device/led.h"

//ҵ��ģ��ͷ�ļ�
#include "bbb.h"

static int flag = 0;

int bbb_initmodel(struct BASE * this)
{
	int ret = 0;
	struct bbbModule *obj;

	obj = (struct bbbModule *)this;
//ע��ģ��
	ret = module_register(this);
	if (ret) {
		goto error;
	}
//״̬��ģ���ʼ��
	ret = led_init();
	if (ret) {
		goto error;
	}
//ע��1�����ڶ�ʱ
	ret = systime_register_period(this, 1, 1);
	if (ret < 0) {
		goto error;
	}
	else {
		obj->tmhdr_1s = ret;
	}

	ret = 0;
error:
	return ret;
}

int bbb_initdata(struct BASE * this)
{
	//...;
	return 0;
}

int bbb_msghandle(struct BASE * this, message_t *msg)
{
	struct bbbModule *obj;

	obj = (struct bbbModule *)this;

	switch (msg->type) {
	case MSG_TIME:
		if (msg->lpara == obj->tmhdr_1s) {
			if (flag) {
				led_on(LED_TXDJL, 0, 0, 0);
				led_on(LED_WARN, 0, 0, 0);
				led_off(LED_RXDJL);
				led_off(LED_RUN);
				flag = 0;
			}
			else {
				led_on(LED_RXDJL, 0, 0, 0);
				led_on(LED_RUN, 0, 0, 0);
				led_off(LED_TXDJL);
				led_off(LED_WARN);
				flag = 1;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

struct BASEFT bbb_ft = {bbb_initmodel, bbb_initdata, bbb_msghandle};
