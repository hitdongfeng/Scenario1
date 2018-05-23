#ifndef _GA_H_
#define _GA_H_
#include "wdstypes.h"


/* ����GA��ز��� */
int Num_group = 100;		/* Ⱥ������ģ */
int Num_offs = 102;			/* �����������(Num_son = Num_group + 2) */
int Num_iteration = 1000;	/* �������� */
double P_mutation = 0.01;	/* ������� */
double P_crossover = 0.8;	/* ������� */

/* ����Solution�ṹ�� */
typedef struct {
	int C_01;		/* Criteria 1*/
	int C_02;		/* Criteria 2*/
	double C_03;	/* Criteria 3*/
	double C_04;	/* Criteria 4*/
	int C_05;		/* Criteria 5*/
	double C_06;	/* Criteria 6*/
	double objvalue;	/* Ŀ�꺯��ֵ */
	double P_Reproduction;	/* ������� */
	LinkedList* SerialSchedule;  /* ����ָ������ָ��(���е���ָ��) */
	LinkedList	NewVisDemages;	 /* �޸��������³��ֵĿɼ�����ܵ�����ָ�� */
	STaskassigmentlist Schedule[MAX_CREWS]; /* ���̶ӵ���ָ��(������ʼ���������) */
}Solution;



Solution* Groups;			/* �洢Ⱥ�� */  
Solution* Offspring;		/* �洢�ӽ���ĸ��� */


/* ������غ��� */
int Memory_Allocation();			/* ������ڴ� */
int InitialGroups();				/* ��ʼ����Ⱥ */


#endif // _GA_H_
