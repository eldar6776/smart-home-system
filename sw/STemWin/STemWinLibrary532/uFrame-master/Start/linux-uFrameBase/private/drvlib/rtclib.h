/******************************************************************************
 * ��̵����ɷ����޹�˾                                    ��Ȩ��2008-2015    *
 ******************************************************************************
 * ��Դ���뼰������ĵ�Ϊ���������̵����ɷ����޹�˾�������У�δ��������     *
 * �ɲ��������޸Ļ򷢲�������׷����صķ������Ρ�                           *
 *                                                                            *
 *                       ���������̹ɷ����޹�˾                             *
 *                       www.xjgc.com                                         *
 *                       (0374) 321 2924                                      *
 *                                                                            *
 ******************************************************************************
 * 
 * ��Ŀ����		:	rtc����Ӧ�ò�ͷ�ļ�
 * �ļ���		:	rtclib.h
 * ����			:	Ӧ��ds3231 rtc�����������õ������ݽṹ��ioctl����cmd����
 * �汾			:	1.0.1
 * ����			:	·ȽȽ
 *
 * �޸���ʷ��¼	:
 * --------------------
 * 01a, 18aug2009, Roy modified
 * --------------------
 *
 ******************************************************************************/

#ifndef _RTCLIB_H
#define _RTCLIB_H


/*
 * The struct used to pass data via the following ioctl. Similar to the
 * struct tm in <time.h>, but it needs to be here so that the kernel
 * source is self contained, allowing cross-compiles, etc. etc.
 */
 
struct rtc_time {
	int tm_sec;		 //�룺[0 - 59]
	int tm_min;		 //���ӣ�[0 - 59]
	int tm_hour;		 //ʱ��[0 - 23]
	int tm_mday;		 //�գ�[1 - 31]
	int tm_mon;		 //�£��Դ�һ���������·ݣ�[0 - 11]
	int tm_year;		 //�꣬�Դ�1900����������:
	int tm_wday;		 //�ܣ��������յ�����[0 - 6]
	int tm_yday;
	int tm_isdst;
};

/*	 Rtc_time �ṹ��ʹ��ע�⣺
	 Tm_mon Ϊ����1�µ��·ݣ�����tm_mon = 8 ������������9�£�
	 tm_yearΪ����1900�������������tm_year = 109����������2009�ꡣ
*/



#define RTC_RD_TIME		_IOR('p', 0x09, struct rtc_time) /* Read RTC time   */
#define RTC_SET_TIME	_IOW('p', 0x0a, struct rtc_time) /* Set RTC time    */
#define RTC_GET_STAT	_IOR('p', 0x19, unsigned char) 			/* Read RTC status   */
#endif  /* _RTCLIB_H */

