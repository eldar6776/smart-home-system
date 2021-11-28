/******************************************************************************
	��Ŀ����	��  SGE800���������ն�ƽ̨
	�ļ�		��  swin.c
	����		��  ���ļ�������������ģ��Ľӿ�
	�汾		��  0.1
	����		��  ·ȽȽ
	��������	��  2010.12
******************************************************************************/
//������ͷ�ļ�
#include "framework/config.h"
	
//ģ�����ÿ���
#ifdef CFG_SWIN_DEVICE
	
//C��ͷ�ļ�
#include <stdio.h>
#include <fcntl.h> 		//open ��־	
#include <sys/ioctl.h>		//ioctl
#include <string.h> 		//memset
#include <unistd.h>		//close
#include <sys/select.h>	//select

//����ƽ̨ͷ�ļ�
#include "sge_core/device/pulselib.h"
#include "sge_core/device/timerlib.h"
#include "sge_core/device/gpiolib.h"
#include "sge_core/error.h"
#include "sge_core/pinio.h"
#include "sge_core/thread.h"
//#include "sge_core/rtc.h"

//ҵ��ƽ̨ͷ�ļ�
#include "framework/device/swin.h"
#include "framework/message.h"
#include "framework/base.h"
#include "framework/systime.h"

/*************************************************
  ��̬ȫ�ֱ������궨��
*************************************************/
#define PINBASE 32						//���뵽������io�ڵ�ַ��ַ
#define MAX_SWINPIN (8 + 4)				//���io������,8·��ͨ��4·ȷ��
#define SWIN_BUFSIZE 1024				//������������С����λ�ֽ�
#define CUR_SWINPIN (CFG_SWIN_NUM + 4)		//��ǰio������,4·ȷ��

#define CFG_CHECK_FILTER_NUM	3		//��鵱ǰio��״̬��������
#define CFG_CHECK_FILTER_TIME	20		//��鵱ǰio��״̬����ʱ��

#ifndef CFG_SWIN_READ_CYCLE
	#define CFG_SWIN_READ_CYCLE 1000
#endif

//#define CFG_SWIN_IOSTATE0		1			//IO��ʼ״̬��ƽ
//#define CFG_SWIN_IOSTATE_DOOR	1			//IO��ʼ״̬��ƽ
//#define CFG_SWIN_IOSTATE_TOP	1			//IO��ʼ״̬��ƽ
//#define CFG_SWIN_IOSTATE_MID	1			//IO��ʼ״̬��ƽ
//#define CFG_SWIN_IOSTATE_TAIL	1			//IO��ʼ״̬��ƽ



static int fd;							//�����������ļ�������
static int t_fd;						//��ʱ���������ļ�������
static int g_fd;						//gpio�����ļ�������

static u8 swin_count = 0;				//ģ��򿪼���
static pthread_mutex_t mutex;			//������

static u16			swin_buf[SWIN_BUFSIZE];		//�����������
static swin_time_t	swin_result[CUR_SWINPIN];	//������
static u8			swin_pin[CUR_SWINPIN];		//�ܽ�io��ַ
static u32			swin_num[CUR_SWINPIN];		//�ܽ��������
static u16			high[CUR_SWINPIN], low[CUR_SWINPIN];	//�����иߵ�ƽ���ص�ƽ��������
static u8			swin_iostate[CUR_SWINPIN];		//��ǰ״̬������ߵ�ƽ���͵�ƽ
static u8			swin_fun[CUR_SWINPIN]={0};		//��ǰ���ܣ�2-ң��remsig��1-����pulse
static	u8			pulse_change[CUR_SWINPIN], remsig_change[CUR_SWINPIN];

//ң����Ϣ���λ�����

#define SWIN_BUFF_SIZE 256			//ѭ�����д�С
#define SWIN_BUFF_MSG_SIZE 8		//ѭ��������ÿ֡��Ϣ�Ĵ�С
static struct swin_fifo_t {
	u8 buffer[SWIN_BUFF_SIZE];
	u8 in;
	u8 out;
}swin_fifo;
//���λ����������궨��
#define BUF_HEAD 		(swin_fifo.buffer[swin_fifo.in])	//������ͷ
#define BUF_TAIL 		(swin_fifo.buffer[swin_fifo.out])	//������β
#define INCBUF(x,mod) 	((++(x)) & ((mod) - 1))				//mod ������2����

//���ڱ��浽u32�����У�yearΪ128�꣬�뾫ȷ��2s
//struct date_t{
//	u32 sec		:5;
//	u32 min		:6;
//	u32 hour	:5;
//	u32 day		:5;
//	u32 mon		:4;
//	u32 year	:7;
//};
//union swin_time_t{
//	u32 date;
//	struct date_t time;
//};

/*************************************************
  API
*************************************************/
static int swin_process(void);
static int swin_read(u8 id,swin_time_t *p);
static int swin_check(u8 id);
//static void swin_mkmsg(u32 msg_wpara, st_ymdhmsw_t time)
//{
//	;
//}
//����ѭ��������������ң����Ϣ֡
//����������֡��ʽ��8λÿ֡���ӵ�λ����λ�ֱ�Ϊ io�˿ںš����ԡ��ꡢ�¡��ա�ʱ���֡��롣
static void inline swin_msg(u8 chn,swin_time_t *result){
	int num ;
	message_t msg_swin ;
	st_ymdhmsw_t  *swin_time;

	if(swin_fun[chn] == REMSIG){
		if(chn >= CFG_SWIN_NUM)
			chn += SWIN_DOOR - 1;
	//���ͨ������֡�������е�ң����Ϣ
		for(num = 0; num < result->num; num ++){
			BUF_HEAD = chn;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);
			BUF_HEAD = result->jump[num].polar;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);

			swin_time = &result->jump[num].time;
//							printf("%s:%d/%d/%d,%d:%d:%d\n",__FUNCTION__,swin_time->year + 2000,swin_time->mon,swin_time->day,swin_time->hour,swin_time->min,swin_time->sec);
			BUF_HEAD = swin_time->year;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);
			BUF_HEAD = swin_time->mon;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);
			BUF_HEAD = swin_time->day;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);
			BUF_HEAD = swin_time->hour;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);
			BUF_HEAD = swin_time->min;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);
			BUF_HEAD = swin_time->sec;
			swin_fifo.in = INCBUF(swin_fifo.in,SWIN_BUFF_SIZE);

			msg_swin.type 	= MSG_DIN;
			msg_swin.wpara 	= chn;
			msg_swin.lpara 	= (u32)(&BUF_TAIL);
			message_dispatch(&msg_swin);

			swin_fifo.out += SWIN_BUFF_MSG_SIZE - 1;
			swin_fifo.out = INCBUF(swin_fifo.out , SWIN_BUFF_SIZE);
		}
	}else if(swin_fun[chn] == PULSE){
		msg_swin.type 	= MSG_PULSE;
		msg_swin.wpara 	= chn;
		msg_swin.lpara 	= result->num;
		message_dispatch(&msg_swin);
	}
}
/******************************************************************************
*	����:	thread_swin_read
*	����:	ң�������ȡ�߳�
*	����:
*	����:
*	˵��:
 ******************************************************************************/
static void * thread_swin_read(void * arg)
{
	int ret = -1;
	int chn = 0 ;
	u8 	pin;
	swin_time_t swin_result;

	fd_set rfds;
	struct timeval tv;
	int fd = 1;
	tv.tv_sec = 0;
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);

	while(1){
		pin = swin_process();
		if(pin > 0 ){
			for(chn = 0; chn < CUR_SWINPIN; chn ++){
				if((pin & (1 << chn)) > 0){
					ret = swin_read(chn, &swin_result);
					swin_msg(chn,&swin_result);
				}
			}
		}
		tv.tv_usec = CFG_SWIN_READ_CYCLE*1000;
		select (0, NULL, NULL, NULL, &tv);
	}
	pthread_exit(0);
}

/******************************************************************************
*	����:	swin_set
*	����:	����Ϊң�Ż�����ģʽ
*	����:	id				-	����ͨ����
			mode			-	�����ͷ�ļ�����
*	����:	0				-	�ɹ�
			-ERR_INVAL		-	��������
			-ERR_NODEV		-	�޴��豸���߲���Ҫ����

*	˵��:
 ******************************************************************************/
int swin_set(u8 id, u8 mode)
{
	if((id < 0) || (id >= CFG_SWIN_NUM)){		//��Χ���
		return -ERR_NODEV;
	}
	if((mode != REMSIG) && (mode != PULSE)){				//��Χ���
			return -ERR_INVAL;
	}
	swin_fun[id] = mode;
	return 0;
}
/******************************************************************************
*	����:	swin_init
*	����:	ң�ż�������ģ���ʼ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_CFG		-	���ó���,����û������ң�Ż�����ģʽ
			-ERR_BUSY		-	�Ѿ���
			-ERR_SYS		-	ϵͳ����
			-ERR_NOFILE		-	û�д�·��
 * ˵��:	��
 ******************************************************************************/
int swin_init(void)
{
	int ret = -1;
	int i = 0;
	pthread_t swin_id;
	char *dev[]={"/dev/atmel_tc0","/dev/atmel_tc1","/dev/atmel_tc2",
				"/dev/atmel_tc3","/dev/atmel_tc4","/dev/atmel_tc5"};
	
	if(CFG_SWIN_NUM > MAX_SWINPIN){
		ret = -ERR_CFG; 	//���ó���
		goto err;
	}

	for(i = 0; i < CFG_SWIN_NUM; i++){
		if((swin_fun[i] == REMSIG) || (swin_fun[i] == PULSE)){				//��Χ���
			goto start;
		}
	}
	return -ERR_CFG;

start:
	if(swin_count == 1){
		ret = -ERR_BUSY;		//�Ѿ���
		goto err;
	}
	//��ʼ��������
	if (pthread_mutex_init(&mutex, NULL)) {
		ret = -ERR_SYS;
		goto err;
	}

	//��ʼ������
	memset(swin_num, 0, CUR_SWINPIN*sizeof(u32));
	memset(swin_buf, 0, SWIN_BUFSIZE*sizeof(u16));
	memset(high, 0, CUR_SWINPIN*sizeof(u16));
	memset(low, 0, CUR_SWINPIN*sizeof(u16));
	memset(swin_iostate, 0, CUR_SWINPIN*sizeof(u8));
//	memset(swin_fun, 0xff, CUR_SWINPIN*sizeof(u8));
	memset(pulse_change, 0, CUR_SWINPIN*sizeof(u8));
	memset(remsig_change, 0, CUR_SWINPIN*sizeof(u8));

	swin_fifo.in = swin_fifo.out = 0;
	//����gpio�����ַ
#ifdef CFG_SWIN_0
	swin_pin[0] = CFG_SWIN_0 + PINBASE;
//	swin_iostate[0] = CFG_SWIN_IOSTATE0;
#endif
#ifdef CFG_SWIN_1
	swin_pin[1] = CFG_SWIN_1 + PINBASE;
#endif
#ifdef CFG_SWIN_2
	swin_pin[2] = CFG_SWIN_2 + PINBASE;
#endif
#ifdef CFG_SWIN_3
	swin_pin[3] = CFG_SWIN_3 + PINBASE;
#endif
#ifdef CFG_SWIN_4
	swin_pin[4] = CFG_SWIN_4 + PINBASE;
#endif
#ifdef CFG_SWIN_5
	swin_pin[5] = CFG_SWIN_5 + PINBASE;
#endif
#ifdef CFG_SWIN_6
	swin_pin[6] = CFG_SWIN_6 + PINBASE;
#endif
#ifdef CFG_SWIN_7
	swin_pin[7] = CFG_SWIN_7 + PINBASE;
#endif

	swin_pin[CFG_SWIN_NUM] 		= CFG_SWIN_DOOR + PINBASE;       //�Žӵ��źŶ�ӦIO
	swin_fun[CFG_SWIN_NUM] 		= REMSIG;
//	swin_iostate[CFG_SWIN_NUM] = CFG_SWIN_IOSTATE_DOOR;
	swin_pin[CFG_SWIN_NUM + 1]	= CFG_SWIN_LID_TOP + PINBASE;    //������źŶ�ӦIO
	swin_fun[CFG_SWIN_NUM + 1] 	= REMSIG;
//	swin_iostate[CFG_SWIN_NUM + 1] = CFG_SWIN_IOSTATE_TOP;
	swin_pin[CFG_SWIN_NUM + 2]	= CFG_SWIN_LID_MID + PINBASE;    //���и��źŶ�ӦIO
	swin_fun[CFG_SWIN_NUM + 2] 	= REMSIG;
//	swin_iostate[CFG_SWIN_NUM + 2] = CFG_SWIN_IOSTATE_MID;
	swin_pin[CFG_SWIN_NUM + 3]	= CFG_SWIN_LID_TAIL + PINBASE;   //��β���źŶ�ӦIO
	swin_fun[CFG_SWIN_NUM + 3]	 = REMSIG;
//	swin_iostate[CFG_SWIN_NUM + 3] = CFG_SWIN_IOSTATE_TAIL;

	//�򿪶�ʱ������
	t_fd = open(dev[CFG_SWIN_TIMER], O_RDWR | O_NOCTTY );
   	if (t_fd < 0){
   		ret = -ERR_NOFILE;		//û�д�·��
		goto err;
	}
	//���ö�ʱ������ɼ�����
	ret = ioctl(t_fd, SET_PULSE, 0);
	if (ret < 0){
   		ret = -ERR_SYS;
		goto err;
	}
	//���ö�ʱ��ʱ��
	ret = ioctl(t_fd, SET_CLOCK, SCK32KIHZ); 
	if (ret < 0){
   		ret = -ERR_SYS;
		goto err;
	}
	//���ö�ʱ����ʱʱ��
	ret = ioctl(t_fd, SET_DELAY, CFG_SWIN_FILTER_TIME);
	if (ret < 0){
   		ret = -ERR_SYS;
		goto err;
	}
	//�����忪��ģ������
	fd = open("/dev/atmel_ps", O_RDONLY );//| O_NONBLOCK
	if (fd < 0){
   		ret = -ERR_NOFILE;		//û�д�·��
		goto err;
	}  
	
	ret = ioctl(fd, SET_TC, CFG_SWIN_TIMER);
	if (ret < 0){
   		ret = -ERR_SYS;
		goto err;
	}

	//����Ҫ����io��
	for(i = 0; i < CUR_SWINPIN; i ++){
		if(swin_pin[i] > 0){
			ret = ioctl(fd, ADD_IO, swin_pin[i]);
			if (ret < 0){
				ret = -ERR_SYS;		//û�д�·��
				goto err;
			}
		}
	}

	//��gpioģ��
	g_fd = open("/dev/atmel_gpio", O_RDWR | O_NOCTTY);
	if (g_fd < 0){
		ret = -ERR_NOFILE;		//û�д�·��
		goto err;
	}

	//����ң�ź�����˿ڳ�ʼ״̬
	for(i = 0; i < CUR_SWINPIN; i ++){
		if((swin_fun[i] == REMSIG) || (swin_fun[i] == PULSE)){
			ret = swin_check(i);
			if (ret < 0){
				goto err;
			}else{
				swin_iostate[i] = ret;
			}
		}
	}

	 //����
	ret = ioctl(fd, PSTART, 0);
	if (ret < 0){
   		ret = -ERR_SYS;	
		goto err;
	}	
	swin_count = 1;
	//��������ɨ���̣߳�ʵʱ�̣߳����ȼ�80-90
	ret = thread_create_base(&swin_id, thread_swin_read, NULL, THREAD_MODE_REALTIME, 80);
	if((ret < 0) ){
		swin_count = 0;
		goto err;
	}
	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	swin_process
*	����:	��������п���Ľ����������������ң�ű�λʱ�̣�
*	����:
*	����:	>=0				-	��־λ����16λ��ʾ��ͨ���Ƿ��б仯
			-ERR_INVAL		-	��������
			-ERR_SYS		-	ϵͳ����ioctl����ʧ�ܻ�ʵʱʱ�Ӷ�ȡʱ�����
			-ERR_NOINIT		-	����ʵʱʱ��ģ��û�г�ʼ����
			-ERR_OTHER:		-	���������̻߳������Ĵ���
*	˵��:
 ******************************************************************************/
static int swin_process(void)
{
	int ret;
	int i,j,read_byte;
	u16 * p;

	p = swin_buf;


	if(swin_count == 0)
		return -ERR_NOINIT;
	for(j = 0; j < CUR_SWINPIN; j ++){
		if((swin_fun[j] != REMSIG) && (swin_fun[j] != PULSE)){	//��Χ���
			return -ERR_NOCFG;
		}
	}
	//��û�����
	if (pthread_mutex_lock (&mutex)) {
		return	-ERR_NOINIT;
	}
	memset(p,0,SWIN_BUFSIZE*sizeof(u16));
	//��ȡ����
	ret = read(fd, (void*)p, 0);		//retΪ�����ֽ���
	if (ret < 0){
   		ret = -ERR_SYS;
		goto err;
	}
	read_byte = ret;
	for(i = 0; i < read_byte/2 ; i++){				//iΪ��i�β�����jΪ��j·io��
		for(j = 0; j < CUR_SWINPIN; j ++){
			if(swin_fun[j] == REMSIG){

				//�����洢��������-ң��
				if((swin_buf[i] >> j & 0x01) == 1 ){
					if(swin_iostate[j] == 0){
						high[j] ++;
						low[j] = 0;
						if(high[j] >= CFG_SWIN_FILTER_NUM){
							high[j] = 0;
							swin_iostate[j] = 1;

							swin_result[j].num += 1;
							if(swin_result[j].num >= 8){	//ң���źű�λ���8��
								swin_result[j].num = 8;		//ң���źű�λ���8��
							}

							if(remsig_change[j] >= 7){	//ң���źű�λʱ��buf��ʱ������һ��
								remsig_change[j] = 7;	//���һ��ʱ��
							}
							swin_result[j].jump[remsig_change[j]].polar = 1;
							ret =  systime_get(&swin_result[j].jump[remsig_change[j]].time);
							if(ret < 0 ){
								goto err;
							}
							remsig_change[j] += 1;
						}
					}
				}else{
					if(swin_iostate[j] == 1){
						low[j] ++;
						high[j] = 0;
						if(low[j] >= CFG_SWIN_FILTER_NUM){
							low[j] = 0;
							swin_iostate[j] = 0;
							swin_result[j].num += 1;
							if(swin_result[j].num >= 8){	//ң���źű�λ���8��
								swin_result[j].num = 8;		//ң���źű�λ���8��
							}

							if(remsig_change[j] >= 7){		//ң���źű�λʱ��buf��ʱ������һ��
								remsig_change[j] = 7;		//���һ��ʱ�估����
							}
							swin_result[j].jump[remsig_change[j]].polar = 0;
							ret =  systime_get (&swin_result[j].jump[remsig_change[j]].time);
							if(ret < 0 ){
								goto err;
							}
							remsig_change[j] += 1;
						}
					}
				}

			//�����洢��������-������
			}else if(swin_fun[j] == PULSE){
				if((swin_buf[i] >> j & 0x01) == 1 ){
					if(swin_iostate[j] == 0){
						high[j] ++;
						low[j] = 0;
						if(high[j] >= CFG_SWIN_FILTER_NUM){
							swin_iostate[j] = 1;
							swin_num[j] += 1;
							pulse_change[j] = 1;
						}
					}
				}else{
					low[j] ++;
					high[j] = 0;
					if(low[j] >= CFG_SWIN_FILTER_NUM)
						swin_iostate[j] = 0;
				}

			}
		}
	}

	ret = 0;
	for(j = 0; j < CUR_SWINPIN ; j++){
		if((swin_fun[j] == PULSE) && (pulse_change[j] == 1)){	//�������δ���ߵ������������Ӧ·��1
			ret |= 1 << j;
		}
		if((swin_fun[j] == REMSIG) && (remsig_change[j] >= 1)){//�������δ���ߵ�ң�ţ���Ӧ·��1
			ret |= 1 << j;
		}
	}

err:	//����
	if (pthread_mutex_unlock (&mutex)) {
		ret = -ERR_OTHER;
	}
	return ret;
}

/******************************************************************************
*	����:	swin_read
*	����:	����ָ��ͨ���ŵ���������ң�ű�λʱ��
*	����:	id				-	����ͨ����
			p				-	��������ң�ű�λʱ�̣����ݴ�����
*	����:	0				-	�ɹ�
			-ERR_INVAL		-	��������
			-ERR_NODEV		-	�޴��豸
			-ERR_NOINIT		-	ģ��û�г�ʼ����
*	˵��:
 ******************************************************************************/
static int swin_read(u8 id,swin_time_t *p)
{
	int ret;

	if(swin_count == 0)
		return -ERR_NOINIT;

	if((id < 0) || (id >= CUR_SWINPIN)){		//��Χ���
		return -ERR_NODEV;
	}
	if(swin_fun[id] == PULSE){
		memset(p, 0,sizeof(swin_time_t));
		p->num = swin_num[id];
		swin_num[id] = 0;
		pulse_change[id] = 0;
	}else if(swin_fun[id] == REMSIG){
		memcpy(p, &swin_result[id],sizeof(swin_time_t));
		memset(&swin_result[id], 0,sizeof(swin_time_t));
		memset(remsig_change, 0, CUR_SWINPIN*sizeof(u8));
	}else{
		ret = -ERR_INVAL;
		goto err;
	}

	ret = 0;
err:
	return ret;
}

/******************************************************************************
*	����:	swin_check
*	����:	ʵʱ��ѯ��ǰ״̬
*	����:	id				-	����ͨ����
*	����:	0				-	��ǰ״̬Ϊ�͵�ƽ
*			1				-	��ǰ״̬Ϊ�ߵ�ƽ
			-ERR_NODEV		-	�޴��豸
			-ERR_SYS		-	ϵͳ����gpio����ioctl����ʧ��
			-ERR_DEVUNSTAB	-	�豸���ȶ�
*	˵��:
 ******************************************************************************/
static int swin_check(u8 id)
{
	int ret;
	u32 tmp[CFG_CHECK_FILTER_NUM];
	int i;

	fd_set rfds;
	struct timeval tv;
	int fd_t = 1;

	tv.tv_sec = 0;
	tv.tv_usec = CFG_CHECK_FILTER_TIME*1000;
	FD_ZERO (&rfds);
	FD_SET (fd_t, &rfds);


	if((id < 0) || (id >= CUR_SWINPIN)){		//din��Χ���
		return -ERR_NODEV;
	}

	//�˲���ȡio��״̬
	for(i = 0; i < CFG_CHECK_FILTER_NUM; i ++){
		tmp[i] = swin_pin[id];
		ret = ioctl(g_fd, IOGETI, &tmp[i]);

		if(ret < 0){
			ret = -ERR_SYS;
			goto err;
		}


		if((tmp[i] < 0) || (tmp[i] > 1)){
			ret = -ERR_SYS;
			goto err;
		}
		select (0, NULL, NULL, NULL, &tv);
	}
	for(i = 0; i < CFG_CHECK_FILTER_NUM - 1; i ++){
		if(tmp[i]  != tmp[i+1]){
			ret = -ERR_DEVUNSTAB;
			goto err;
		}
	}
	ret = tmp[0];
err:
	return ret;
}

/******************************************************************************
*	����:	swin_close
*	����:	�ر�ң��������ģ��
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_NOINIT		-	û�г�ʼ��
			-ERR_OTHER		-	���������̻߳������Ĵ���
*	˵��:	��
 ******************************************************************************/
//static int swin_close(void)
//{
//	int ret = -1;
//
//	if(swin_count == 0)
//		return -ERR_NOINIT;
//
//	ret = close(fd);
//	if(ret < 0)
//		return -ERR_SYS;
//	ret = close(t_fd);
//	if(ret < 0)
//		return -ERR_SYS;
//	ret = close(g_fd);
//	if(ret < 0)
//		return -ERR_SYS;
//
//	swin_count = 0;
//	//���ٻ�����
//	if (pthread_mutex_destroy(&mutex)) {
//		ret = -ERR_OTHER;
//	}
//	ret = 0;
//	return ret;
//}

#endif
