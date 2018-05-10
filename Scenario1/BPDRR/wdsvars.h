
//���ļ�������BPDRR�����е�ȫ�ֱ������ļ����ݱ��������ͽ�����֯��ÿ����������һ���򵥵�˵����

#ifndef WDSVARS_H
#define WDSVARS_H
#include "wdstypes.h"

EXTERN int	Nhospital,		/* ҽԺ��ʩ���� */
			Nfirefight,		/* ����˨���� */
			Nbreaks,		/* ���ܹܵ����� */
			Nleaks,			/* ©ʧ�ܵ����� */
			Ninivariables;	/* ��ʼ��������� */

EXTERN FILE *ErrFile;		/* ���󱨸��ļ�ָ�� */



//EXTERN	PDecision_Variable	Part_init_solution; /* ��ʼ��ָ�� */
EXTERN	SHospital* Hospitals;			/* ҽԺ��ʩ�ṹ��ָ�� */
EXTERN	SFirefight* Firefighting;		/* ����˨�ṹ��ָ�� */
EXTERN	SBreaks*	BreaksRepository;	/* ���ֿܲ�ָ��(���ڴ洢���б���) */
EXTERN	SLeaks*		LeaksRepository;	/* ©��ܵ��ֿ�ָ��(���ڴ洢����©��ܵ�) */
EXTERN	SCrew*		Schedule;			/* ���̶ӵ���ָ�� */
EXTERN	LinkedList	linkedlist;			/* ����ܵ��ܵ������޸�ָ��ṹ��(���ڴ洢����ָ��) */
EXTERN	VisiableList	IniVisDemages;	/* ģ�⿪ʼʱ��(6:30)�ɼ�����ܵ�����ָ�� */
EXTERN	VisiableList	NewVisDemages;	/* �޸��������³��ֵĿɼ�����ܵ�����ָ�� */
EXTERN	float*	ActuralBaseDemand;			/* �ڵ�ʵ����ˮ������ָ�� */





#endif
