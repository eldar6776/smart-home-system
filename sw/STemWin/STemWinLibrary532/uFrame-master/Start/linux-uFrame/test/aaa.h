/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��aaa.h
	����		������ҵ��ģ��-ƽ̨ʱ�����
	�汾		��0.1
	����		����迡
	��������	��2010.11
******************************************************************************/

#ifndef _AAA_H
#define _AAA_H

#include "framework/base.h"

struct aaaModule
{
	struct BASE base;
	int tmhdr_4s;
	int tmhdr_9s;
	int tmhdr_clock;
};

extern struct BASEFT aaa_ft;

#endif
