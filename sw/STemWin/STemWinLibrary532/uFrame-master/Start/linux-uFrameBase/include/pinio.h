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
	�ļ�		��  pinio.h
	����		��  ���ļ�������ƽ̨��IO�ڱ��
	�汾		��  1.0
	����		��  ·ȽȽ
	��������	��  2009.09
******************************************************************************/

#ifndef _PINIO_H
#define _PINIO_H

//IO�ڱ�ź궨��
#define	PIN_PA0		0			//IO�ڱ��
#define	PIN_PA1		1			//IO�ڱ��	
#define	PIN_PA2		2			//IO�ڱ��	
#define	PIN_PA3		3			//IO�ڱ��
#define	PIN_PA4		4			//IO�ڱ��	
#define	PIN_PA5		5			//IO�ڱ��	
#define	PIN_PA6		6			//IO�ڱ��	
#define	PIN_PA7		7			//IO�ڱ��	
#define	PIN_PA8		8			//IO�ڱ��	
#define	PIN_PA9		9			//IO�ڱ��	
#define	PIN_PA10	10			//IO�ڱ��
#define	PIN_PA11	11			//IO�ڱ��
#define	PIN_PA12	12			//IO�ڱ��
#define	PIN_PA13	13			//IO�ڱ��
#define	PIN_PA14	14			//IO�ڱ��
#define	PIN_PA15	15			//IO�ڱ��
#define	PIN_PA16	16			//IO�ڱ��
#define	PIN_PA17	17			//IO�ڱ��
#define	PIN_PA18	18			//IO�ڱ��
#define	PIN_PA19	19			//IO�ڱ��
#define	PIN_PA20	20			//IO�ڱ��
#define	PIN_PA21	21			//IO�ڱ��
#define	PIN_PA22	22			//IO�ڱ��
#define	PIN_PA23	23			//IO�ڱ��
#define	PIN_PA24	24			//IO�ڱ��
#define	PIN_PA25	25			//IO�ڱ��
#define	PIN_PA26	26			//IO�ڱ��
#define	PIN_PA27	27			//IO�ڱ��
#define	PIN_PA28	28			//IO�ڱ��
#define	PIN_PA29	29			//IO�ڱ��
#define	PIN_PA30	30			//IO�ڱ��
#define	PIN_PA31	31			//IO�ڱ��

#define	PIN_PB0		32			//IO�ڱ��
#define	PIN_PB1		33			//IO�ڱ��
#define	PIN_PB2		34			//IO�ڱ��
#define	PIN_PB3		35			//IO�ڱ��	
#define	PIN_PB4		36			//IO�ڱ��
#define	PIN_PB5		37			//IO�ڱ��
#define	PIN_PB6		38			//IO�ڱ��
#define	PIN_PB7		39			//IO�ڱ��
#define	PIN_PB8		40			//IO�ڱ��
#define	PIN_PB9		41			//IO�ڱ��
#define	PIN_PB10	42			//IO�ڱ��
#define	PIN_PB11	43			//IO�ڱ��
#define	PIN_PB12	44			//IO�ڱ��
#define	PIN_PB13	45			//IO�ڱ��
#define	PIN_PB14	46			//IO�ڱ��
#define	PIN_PB15	47			//IO�ڱ��
#define	PIN_PB16	48			//IO�ڱ��
#define	PIN_PB17	49			//IO�ڱ��
#define	PIN_PB18	50			//IO�ڱ��
#define	PIN_PB19	51			//IO�ڱ��
#define	PIN_PB20	52			//IO�ڱ��
#define	PIN_PB21	53			//IO�ڱ��
#define	PIN_PB22	54			//IO�ڱ��
#define	PIN_PB23	55			//IO�ڱ��
#define	PIN_PB24	56			//IO�ڱ��
#define	PIN_PB25	57			//IO�ڱ��
#define	PIN_PB26	58			//IO�ڱ��
#define	PIN_PB27	59			//IO�ڱ��
#define	PIN_PB28	60			//IO�ڱ��
#define	PIN_PB29	61			//IO�ڱ��
#define	PIN_PB30	62			//IO�ڱ��
#define	PIN_PB31	63			//IO�ڱ��

#define	PIN_PC0		64			//IO�ڱ��
#define	PIN_PC1		65			//IO�ڱ��
#define	PIN_PC2		66			//IO�ڱ��
#define	PIN_PC3		67			//IO�ڱ��
#define	PIN_PC4		68			//IO�ڱ��
#define	PIN_PC5		69			//IO�ڱ��
#define	PIN_PC6		60			//IO�ڱ��
#define	PIN_PC7		71			//IO�ڱ��
#define	PIN_PC8		72			//IO�ڱ��
#define	PIN_PC9		73			//IO�ڱ��
#define	PIN_PC10	74			//IO�ڱ��
#define	PIN_PC11	75			//IO�ڱ��
#define	PIN_PC12	76			//IO�ڱ��
#define	PIN_PC13	77			//IO�ڱ��
#define	PIN_PC14	78			//IO�ڱ��
#define	PIN_PC15	79			//IO�ڱ��
#define	PIN_PC16	70			//IO�ڱ��
#define	PIN_PC17	81			//IO�ڱ��
#define	PIN_PC18	82			//IO�ڱ��
#define	PIN_PC19	83			//IO�ڱ��
#define	PIN_PC20	84			//IO�ڱ��
#define	PIN_PC21	85			//IO�ڱ��
#define	PIN_PC22	86			//IO�ڱ��
#define	PIN_PC23	87			//IO�ڱ��
#define	PIN_PC24	88			//IO�ڱ��
#define	PIN_PC25	89			//IO�ڱ��
#define	PIN_PC26	80			//IO�ڱ��
#define	PIN_PC27	91			//IO�ڱ��
#define	PIN_PC28	92			//IO�ڱ��
#define	PIN_PC29	93			//IO�ڱ��
#define	PIN_PC30	94			//IO�ڱ��
#define	PIN_PC31	95			//IO�ڱ��
			
#endif   /* _PINIO_H */

