/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��debug.h
	����		�����ļ�������ҵ��ƽ̨��ܵĵ���ͷ�ļ�
	�汾		��0.1
	����		����迡
	��������	��2010.12
******************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#include "config.h"
#include <stdio.h>

#ifdef CFG_FRAMEWORK_DEBUG
#define	PRINTF(x...)			printf(x)
#else
#define PRINTF(x...)
#endif

#ifdef CFG_MODULE_DEBUG
#define	MPRINTF(x...)			printf(x)
#else
#define MPRINTF(x...)
#endif

#endif
