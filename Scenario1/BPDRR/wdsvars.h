
//���ļ�������BPDRR�����е�ȫ�ֱ������ļ����ݱ��������ͽ�����֯��ÿ����������һ���򵥵�˵����

#ifndef WDSVARS_H
#define WDSVARS_H
#include "wdstypes.h"

EXTERN int	Nhospital,		/* ҽԺ��ʩ���� */
			Nfirefight,		/* ����˨���� */
			Nbreaks,		/* ���ܹܵ����� */
			Nleaks,			/* ©ʧ�ܵ����� */
			Ndecisionvars,	/* ������������ */
			NvarsCrew1,		/* crew1��ʼ����߱��� */
			NvarsCrew2,		/* crew2��ʼ����߱��� */
			NvarsCrew3;		/* crew3��ʼ����߱��� */

EXTERN char *inpfile;		/* ˮ��ģ��inp�ļ�ָ�� */

EXTERN time_t	rawtime;	/* ϵͳ��ǰʱ�� */

EXTERN FILE *ErrFile,		/* ���󱨸��ļ�ָ�� */
            *InFile,		/* data.txt�ļ�ָ�� */
            *SenAnalys,		/* �����ȷ�������ļ�ָ�� */
			*key_solution;

EXTERN	SHospital* Hospitals;			/* ҽԺ��ʩ�ṹ��ָ�� */
EXTERN	SFirefight* Firefighting;		/* ����˨�ṹ��ָ�� */
EXTERN	SBreaks*	BreaksRepository;	/* ���ֿܲ�ָ��(���ڴ洢���б���) */
EXTERN	SLeaks*		LeaksRepository;	/* ©��ܵ��ֿ�ָ��(���ڴ洢����©��ܵ�) */
EXTERN	LinkedList*	ExistSchedule;		/* ���й��̶ӵ���ָ��(��ʼ��) */
EXTERN	LinkedList	decisionlist;		/* ����ܵ��ܵ������޸�ָ��ṹ��(���ڴ洢����ָ��) */
EXTERN	LinkedList	IniVisDemages;		/* ģ�⿪ʼʱ��(6:30)�ɼ�����ܵ�����ָ�� */
EXTERN	float**	ActuralBaseDemand;		/* �ڵ�ʵ����ˮ������ָ�� */
EXTERN	Sercaplist  SerCapcPeriod;		/* ָ��ʱ����ÿ��ģ�ⲽ��ϵͳ��ˮ�����ṹ�� */
EXTERN	SCriteria* Criteria;			/* C_05����׼������ָ��(���ڼ���) */






#endif
