/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��main.c
	����		�����ļ�Ϊ������ڣ�ʵ��ģ��װ��ȹ���
	�汾		��0.1
	����		����迡
	��������	��2010.12
******************************************************************************/

//C��ͷ�ļ�
#include <unistd.h>

//ҵ��ƽ̨ͷ�ļ�
#include "framework/base.h"
#include "framework/debug.h"
#include "framework/framework.h"

//ҵ��ģ��ͷ�ļ�
#include "aaa.h"
#include "disp.h"
#include "device_test.h"

/*************************************************
  ��̬ȫ�ֱ�������
*************************************************/
//�������
static struct aaaModule aaa;
static struct dispModule disp;
static struct device_testModule device_test;
static struct BASE *objects[] = {(struct BASE *)&aaa, (struct BASE *)&disp };

/*************************************************
  ������
*************************************************/
int main()
{
	int ret = 0;
	int i;

	PRINTF("BUILD:%s, %s\n\n", __DATE__, __TIME__);
	//�������ʼ��,�ֹ�дҲ�У��Զ�����Ҳ��
	aaa.base.baseft = &aaa_ft;
	aaa.base.thread = 1;
	aaa.base.prio = 1;
	
	disp.base.baseft = &disp_ft;
	disp.base.thread = 3;
	disp.base.prio = 3;

	//��ʼ��ƽ̨
	ret = framework_init();
	if (ret) {
		//������
		goto error;
	}
	PRINTF("%s:%d\n\n", __FILE__, __LINE__);
	
	//��ʼ��ҵ��ģ��ģ��
	for (i = 0; i < sizeof(objects)/ sizeof(objects[0]); i++)
	{
		ret = objects[i]->baseft->initmodel(objects[i]);
		if (ret) {
			goto error;
		}
	}
	//��ʼ��ҵ��ģ������
	for (i = 0; i < sizeof(objects)/ sizeof(objects[0]); i++)
	{
		ret = objects[i]->baseft->initdata(objects[i]);
		if (ret) {
			goto error;
		}
	}
	
	//����ƽ̨���
	ret = framework_start();
	if (ret) {
		//������
		goto error;
	}
	PRINTF("%s:%d\n\n", __FILE__, __LINE__);

	while(1) {
		sleep(1000);
	}

error:
	PRINTF("main err\n");
	return 0;
}
