/*****************************************************************************
	��̵����ɷ����޹�˾			��Ȩ��2008-2015

	��Դ���뼰������ĵ�Ϊ���������̵����ɷ����޹�˾�������У�δ��������
	�ɲ��������޸Ļ򷢲�������׷����صķ������Ρ�

						���������̹ɷ����޹�˾
						www.xjgc.com
						(0374) 321 2924
*****************************************************************************/


/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ƽ̨
	�ļ�		��  rtc.c
	����		��  ���ļ�ʵ����ʵʱʱ��ģ���е�API����
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2009.12
******************************************************************************/
//������ͷ�ļ�
#include "private/config.h"

//ģ�����ÿ���
#ifdef CFG_RTC_MODULE	

//C��ͷ�ļ�
#include <stdio.h>			//printf
#include <fcntl.h>			//open
#include <unistd.h>			//read,write
#include <pthread.h>		//pthread�⺯��
#include <sys/ioctl.h>
#include <time.h>

//�ṩ���û���ͷ�ļ�
#include "include/rtc.h"
#include "include/error.h"

//��������ͷ�ļ�
#include "private/drvlib/rtclib.h"
#include "private/debug.h"

/*************************************************
  ��̬ȫ�ֱ������궨��
*************************************************/
static int fd;					//����RTC�����ļ�������
static u8 rtc_count = 0;		//ģ��򿪼���
static pthread_mutex_t mutex;	//������

/*************************************************
  API����ʵ��
*************************************************/

/******************************************************************************
*	����:	rtc_init
*	����:	��ʼ��RTCģ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_NODEV		-	�޴��豸
			-ERR_BUSY		-	�豸æ���Ѿ���
			-ERR_SYS		-	��ʼ����ʧ�ܣ����ڴ治�㣻�Ѿ���ʼ����������û���٣���������ַ��Ч��
*	˵��:	��
 ******************************************************************************/
int rtc_init (void)
{
#ifdef LINUX_SIM
	return 0;
#endif
	int ret = -1;
	if(rtc_count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}
	
	fd = open("/dev/rtc", O_RDWR | O_NOCTTY);
	if (fd < 0){
		ret = -ERR_NOFILE;		//û�д�·��
		goto err;
	} 
	
	rtc_count = 1;
	//��ʼ��������	
	if (pthread_mutex_init(&mutex, NULL)) {
		ret = -ERR_SYS;
		goto err;
	}
	
	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	rtc_gettime
*	����:	��ȡʵʱʱ��
*	����:	time			-	ʱ�ӣ����ݴ�����
*	����:	0				-	�ɹ�
			-ERR_INVAL		-	��������
			-ERR_NOINIT		-	�豸����δ��ʼ��
			-ERR_SYS		-	ϵͳ����
*	˵��:	��
 ******************************************************************************/
#ifdef LINUX_SIM
int rtc_gettime (st_ymdhmsw_t *pTime)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	pTime->sec  = tm->tm_sec;
	pTime->min  = tm->tm_min;
	pTime->hour = tm->tm_hour;
	pTime->day  = tm->tm_mday;
	pTime->mon  = tm->tm_mon + 1;
	pTime->year = tm->tm_year + 1900;
	pTime->wday = tm->tm_wday + 1;
	return 0;
}
#else
int rtc_gettime (st_ymdhmsw_t *time)
{
	int ret = -1;
	struct rtc_time dt;
	
	if(rtc_count == 0)			//�豸û��ʼ��
		return -ERR_NOINIT;
	
	if (!time) {		
		return -ERR_INVAL;
	}
	
	ret = ioctl(fd, RTC_RD_TIME, &dt);
	if(ret < 0){
		return -ERR_SYS;
		
	}

	//����ʱ��ת��
	time->sec  = dt.tm_sec;
	time->min  = dt.tm_min;
	time->hour = dt.tm_hour;
	time->day = dt.tm_mday;
	time->mon  = dt.tm_mon + 1;
	time->year = dt.tm_year - 100;		//��ת��
	time->wday = dt.tm_wday + 1;

	ret = 0;
	return ret;
}

#endif

/******************************************************************************
*	����:	rtc_settime
*	����:	����ʵʱʱ��
*	����:	time			-	ʱ�ӣ����ݴ��룩
*	����:	0				-	�ɹ�
			-ERR_INVAL		-	��������
			-ERR_NOINIT		-	�豸����δ��ʼ��
			-ERR_SYS		-	ϵͳ����
			-ERR_OTHER		-	���������߳������ܽ����Ĵ���
*	˵��:	��
 ******************************************************************************/
int rtc_settime (st_ymdhmsw_t *time)
{
	int ret = -1;
	struct rtc_time dt;
	
	if(rtc_count == 0)			//�豸û��ʼ��
		return -ERR_NOINIT;
	
	//�������
	if (!time) {		
		return -ERR_INVAL;
	}
	//������
	if(((time->year%4==0)&&(time->year%100!=0))||(time->year%400==0)){
		if((time->day > 29)&&(time->mon == 2)){
			return -ERR_INVAL;
		}
	}else{
		if((time->day > 28)&&(time->mon == 2)){
			return -ERR_INVAL;
		}
	}
		
	if(time->sec 	< 0 || time->sec 	> 59 ||			//�뷶Χ���
		time->min 	< 0 || time->min 	> 59 ||			//�ַ�Χ���
		time->hour 	< 0 || time->hour 	> 23 ||			//ʱ��Χ���
		time->day 	< 1 || time->day 	> 31 ||			//�췶Χ���
		time->mon 	< 1 || time->mon 	> 12 ||			//�·�Χ���
		((time->mon == 4 || time->mon == 6 || time->mon == 9 || time->mon == 11)&&(time->day > 30))||	//30���·ݼ��
		time->year 	< 0 || time->year 	> 255||			//�귶Χ���
		time->wday 	< 0 || time->wday	> 7 ){		//���ڷ�Χ���
//			(~(((time->year%4==0)&&(time->year%100!=0))||(time->year%400==0)))&&(time->day > 28)&&(time->mon == 2) ||	//2��28��
//			((((time->year%4==0)&&(time->year%100!=0))||(time->year%400==0))&&(time->day > 29)&&(time->mon == 2))){//����2��29��
		return -ERR_INVAL;
	}

	//��û�����
	if (pthread_mutex_lock (&mutex)) {
		return  -ERR_NOINIT;		
	}
	//����ʱ��ת��
	dt.tm_sec	=	time->sec ;
	dt.tm_min	=	time->min ;
	dt.tm_hour	=	time->hour;
	dt.tm_mday	=	time->day ;
	dt.tm_mon 	=	time->mon - 1 ;
	dt.tm_year 	=	time->year + 100;			//��ת��
	dt.tm_wday 	=	time->wday - 1;
	
	ret = ioctl(fd, RTC_SET_TIME, &dt);
	if(ret < 0){
		ret = ERR_SYS;
		goto err;
	}
	
	ret = 0;
err:	//����
	if (pthread_mutex_unlock (&mutex)) {
		ret = -ERR_OTHER;		
	}
	return ret;
}

/******************************************************************************
*	����:	rtc_getstat
*	����:	��ѯʵʱʱ�ӵĹ���״̬���Ƿ�ͣ��
*	����:	stat			-	״̬�����ݴ�����,0-������1-ͣ���
*	����:	0				-	�ɹ�
			-ERR_NOINIT		-	�豸����δ��ʼ��
			-ERR_SYS		-	ϵͳ����
*	˵��:	��
 ******************************************************************************/
int rtc_getstat (u8 *stat)
{
	int ret = -1;

	if(rtc_count == 0)			//�豸û��ʼ��
		return -ERR_NOINIT;

	ret = ioctl(fd, RTC_GET_STAT, stat);
	if(ret < 0){
		return -ERR_SYS;

	}
	ret = 0;
	return ret;
}

/******************************************************************************
*	����:	rtc_close
*	����:	�ر�RTCģ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	ģ��δ��ʼ��
			-ERR_OTHER		-	��ǰ�̲߳�ӵ�л�����������δ��ʼ��
*	˵��:	��
 ******************************************************************************/
int rtc_close (void)
{
	int ret = -1;
	
	if(rtc_count == 0)			//�豸û��ʼ��
		return -ERR_NOINIT;
	
	ret = close(fd);
	if(ret < 0)
		return -ERR_SYS;
	rtc_count = 0;
	
	//���ٻ�����	
	if (pthread_mutex_destroy(&mutex)) {
		ret = -ERR_OTHER;
	}	
	ret = 0;
	return ret;
}

#endif
