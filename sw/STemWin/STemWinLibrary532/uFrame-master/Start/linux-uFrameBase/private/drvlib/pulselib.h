
#ifndef _PSLIB_H
#define _PSLIB_H

#define PS_IOC_MAGIC 0xE3
 
#define SET_TC		_IO(PS_IOC_MAGIC,  0)	//ѡ�����Ǹ���ʱ����Ϊ5ms��ʱ
#define SET_COUNT	_IO(PS_IOC_MAGIC,  1)	//����TIMERΪƵ�ʲ�������
#define ADD_IO		_IO(PS_IOC_MAGIC,  2)	//����TIMERΪ������ƹ���
#define PSTART		_IO(PS_IOC_MAGIC,  3)	//����timerʱ��



#endif  /* _PSLIB_H */

