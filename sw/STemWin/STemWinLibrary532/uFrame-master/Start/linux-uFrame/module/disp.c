/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��disp.c
	����		����ʾʾ��ģ��
	�汾		��0.1
	����		����迡
	��������	��2010.12
******************************************************************************/

//C��ͷ�ļ�

//����ƽ̨ͷ�ļ�
#include "sge_core/rtc.h"
#include "sge_gui/GUI.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/debug.h"
#include "framework/framework.h"
#include "framework/systime.h"
#include "framework/message.h"
#include "framework/device/key.h"

//ҵ��ģ��ͷ�ļ�
#include "disp.h"

int disp_initmodel(struct BASE * this)
{
	int ret = 0;
	struct dispModule *obj;

	obj = (struct dispModule *)this;
//ע��ģ��
	ret = module_register(this);
	if (ret) {
		goto error;
	}
//GUIģ���ʼ��
	ret = GUI_Init();
	if (ret) {
		goto error;
	}
//����ģ���ʼ��
	ret = key_init();
	if (ret) {
		goto error;
	}
	LCD_On();
//������Ϣ
	ret = message_subscribe(this, MSG_KEY);
	if (ret) {
		goto error;
	}
	ret = message_subscribe(this, MSG_SKEY);
	if (ret) {
		goto error;
	}

error:
	return ret;
}

int disp_initdata(struct BASE * this)
{
	//...;
	return 0;
}

int disp_msghandle(struct BASE * this, message_t *msg)
{
	struct dispModule *obj;

	obj = (struct dispModule *)this;

	switch (msg->type) {
	case MSG_KEY:
		LCD_Backlight_On();

		switch (msg->wpara) {
		case KEY_UP:
			GUI_DispStringAt("��", 60,40);
			break;
		case KEY_DOWN:
			GUI_DispStringAt("��", 60,40);
			break;
		case KEY_LEFT:
			GUI_DispStringAt("��", 60,40);
			break;
		case KEY_RIGHT:
			GUI_DispStringAt("��", 60,40);
			break;
		case KEY_ENTER:
			GUI_DispStringAt("ǰ", 60,40);
			break;
		case KEY_CANCEL:
			GUI_DispStringAt("��", 60,40);
			break;
		default:
			break;
		}
		//MPRINTF ("%s:%d\n", __FILE__, msg->wpara);
		break;
	case MSG_SKEY:
		LCD_Backlight_Off();
		//MPRINTF ("%s:%d\n", __FILE__, msg->wpara);
		break;
	default:
		break;
	}
	return 0;
}

struct BASEFT disp_ft = {disp_initmodel, disp_initdata, disp_msghandle};
