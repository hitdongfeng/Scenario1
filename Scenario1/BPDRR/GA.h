#ifndef _GA_H_
#define _GA_H_
#include "wdstypes.h"


/* ����GA��ز��� */
int Num_group = 100;		/* Ⱥ������ģ */
int Num_offs = 102;		/* �����������(Num_son = Num_group + 2) */
int Num_iteration = 1000;	/* �������� */
double P_mutation = 0.01;	/* ������� */
double P_crossover = 0.8;	/* ������� */

/* ����Solution�ṹ�� */
typedef struct {
	double objvalue;	/* Ŀ�꺯��ֵ */
	LinkedList* SerialSchedule;  /* ����ָ������ָ��(���е���ָ��) */
	STaskassigmentlist Schedule[MAX_CREWS]; /* ���̶ӵ���ָ��(������ʼ���������) */
	double P_Reproduction;	/* ������� */
}Solution;

Solution* Groups;			/* �洢Ⱥ�� */  
Solution* Offspring;		/* �洢�ӽ���ĸ��� */


/* ������غ��� */
int Memory_Allocation();			/* ������ڴ� */
void InitialGroups();				/* ��ʼ����Ⱥ */


#endif // _GA_H_
