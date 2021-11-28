/******************************************************************************
	��Ŀ����	��SGE800���������ն�ҵ��ƽ̨
	�ļ�		��dbserver.h
	����		�����ļ����������ݷ���ģ��ӿ�
	�汾		��0.1
	����		������
	��������	��2010.12
******************************************************************************/

#ifndef _DBSERVER_H
#define _DBSERVER_H

#include "base.h"
#include "systime.h"

/*************************************************
  �궨��
*************************************************/



/*************************************************
  �ṹ���Ͷ���
*************************************************/
//������ö�ٶ���
typedef enum {
	data111
} data_enum_t;

//������ö�ٶ���
typedef enum {
	para111
} para_enum_t;



/*************************************************
  API
*************************************************/
//���ݷ���ģ���ʼ��int dbserver_init(void);

/************************������ʵʱ����**************************/
//�� һ�������� һ��ʵʱ����
int read_mp_rt_data (u16 mp, data_enum_t item, s64 *data);
//д һ�������� һ��ʵʱ����
int write_mp_rt_data (u16 mp, data_enum_t item, s64 *data);
//�� һ�������� ����ʵʱ����
int clear_mp_rt_data (u16 mp);
//�� ���в����� ����ʵʱ����
int clear_allmp_rt_data (void);



/************************��������ʷ����**************************/
//�� һ�������� һ��ʱ�� һ��������
int read_mp_day_data (u16 mp, st_ymd_t *tm, data_enum_t item, s64 *data);
//д һ�������� һ��ʱ�� һ��������
int write_mp_day_data (u16 mp, st_ymd_t *tm, data_enum_t item, s64 *data);
//�� һ�������� һ��ʱ�� һ��������
int read_mp_month_data (u16 mp, st_ym_t *tm, data_enum_t item, s64 *data);
//д һ�������� һ��ʱ�� һ��������
int write_mp_month_data (u16 mp, st_ym_t *tm, data_enum_t item, s64 *data);
//�� һ�������� һ��ʱ�� һ����������
int read_mp_curve_data (u16 mp, st_ymdhm_t *tm, data_enum_t item, s64 *data);
//д һ�������� һ��ʱ�� һ����������
int write_mp_curve_data (u16 mp, st_ymdhm_t *tm, data_enum_t item, s64 *data);
//�� һ�������� ������ʷ����
int clear_mp_history_data (u16 mp);
//�� ���в����� ������ʷ����
int clear_allmp_history_data (void);



/************************ֱ��ģ����ʵʱ����**********************/
//�� һ��ֱ��ģ���� һ��ʵʱ����
int read_dc_rt_data (u8 chn, s64 *data);
//д һ��ֱ��ģ���� һ��ʵʱ����
int write_dc_rt_data (u8 chn, s64 *data);
//�� һ��ֱ��ģ���� ����ʵʱ����
int clear_dc_rt_data (u16 mp);
//�� ����ֱ��ģ���� ����ʵʱ����
int clear_alldc_rt_data (void);



/************************ֱ��ģ������ʷ����**********************/
//�� һ��ֱ��ģ���� һ��ʱ�� һ��������
int read_dc_day_data (u8 chn, st_ymd_t *tm, s64 *data);
//д һ��ֱ��ģ���� һ��ʱ�� һ��������
int write_dc_day_data (u8 chn, st_ymd_t *tm, s64 *data);
//�� һ��ֱ��ģ���� һ��ʱ�� һ��������
int read_dc_month_data (u8 chn, st_ym_t *tm, s64 *data);
//д һ��ֱ��ģ���� һ��ʱ�� һ��������
int write_dc_month_data (u8 chn, st_ym_t *tm, s64 *data);
//�� һ��ֱ��ģ���� һ��ʱ�� һ����������
int read_dc_curve_data (u8 chn, st_ymdhm_t *tm, s64 *data);
//д һ��ֱ��ģ���� һ��ʱ�� һ����������
int write_dc_curve_data (u8 chn, st_ymdhm_t *tm, s64 *data);
//�� һ��ֱ��ģ���� ������ʷ����
int clear_dc_history_data (u16 mp);
//�� ����ֱ��ģ���� ������ʷ����
int clear_alldc_history_data (void);



/************************�ܼ���ʵʱ����**************************/
//�� һ���ܼ��� һ��ʵʱ����
int read_zj_rt_data (u8 gp, data_enum_t item, s64 *data);
//д һ���ܼ��� һ��ʵʱ����
int write_zj_rt_data (u8 gp, data_enum_t item, s64 *data);
//�� һ���ܼ��� ����ʵʱ����
int clear_zj_rt_data (u16 mp);
//�� �����ܼ��� ����ʵʱ����
int clear_allzj_rt_data (void);



/************************�ܼ�����ʷ����**************************/
//�� һ���ܼ��� һ��ʱ�� һ��������
int read_zj_day_data (u8 gp, st_ymd_t *tm, data_enum_t item, s64 *data);
//д һ���ܼ��� һ��ʱ�� һ��������
int write_zj_day_data (u8 gp, st_ymd_t *tm, data_enum_t item, s64 *data);
//�� һ���ܼ��� һ��ʱ�� һ��������
int read_zj_month_data (u8 gp, st_ym_t *tm, data_enum_t item, s64 *data);
//д һ���ܼ��� һ��ʱ�� һ��������
int write_zj_month_data (u8 gp, st_ym_t *tm, data_enum_t item, s64 *data);
//�� һ���ܼ��� һ��ʱ�� һ����������
int read_zj_curve_data (u8 gp, st_ymdhm_t *tm, data_enum_t item, s64 *data);
//д һ���ܼ��� һ��ʱ�� һ����������
int write_zj_curve_data (u8 gp, st_ymdhm_t *tm, data_enum_t item, s64 *data);
//�� һ���ܼ��� ������ʷ����
int clear_zj_history_data (u8 gp);
//�� �����ܼ��� ������ʷ����
int clear_allzj_history_data (void);



/************************�¼�����*******************************/

/************************����*****************************/



#endif
