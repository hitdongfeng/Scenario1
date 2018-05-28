#ifndef _GA_H_
#define _GA_H_
#include <stdio.h>
#include "wdstypes.h"


/* ����GA��ز��� */
int Num_group = 100;		/* Ⱥ������ģ */
int Num_offs = 100;			/* �����������(Num_son = Num_group + 2) */
int Num_iteration = 10000;	/* �������� */
double P_mutation = 0.1;	/* ������� */
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
	//LinkedList*	NewVisDemages;	 /* �޸��������³��ֵĿɼ�����ܵ�����ָ�� */
	STaskassigmentlist Schedule[MAX_CREWS]; /* ���̶ӵ���ָ��(������ʼ���������) */
}Solution;

Solution** Groups;			/* �洢Ⱥ�� */  
Solution** Offspring;		/* �洢�ӽ���ĸ��� */
int IndexCross_i;			/* ��ʼ����� */
int IndexCross_j;			/* ��ֹ����� */
int Chrom_length;			/* ����Ⱦɫ�峤�� */
int Length_SonSoliton;		/* �Ŵ������ĺ��ӵĸ��� */  
FILE *TemSolution;			/* �洢ÿһ�����Ž� */

/* ������غ��� */
int Memory_Allocation();			/* ������ڴ� */
void Free_Solution(Solution*);		/*  �ͷ�Solution�ṹ���ڴ� */
void Free_GAmemory();				/* �ͷ������ڴ� */
int InitialGroups();				/* ��ʼ����Ⱥ */
int Calculate_Objective_Value(Solution*);/* ����Ŀ�꺯��ֵ */
void Calc_Probablity();				/* ÿ���²�����Ⱥ��, ����ÿ������ĸ��� */
int Select_Individual();			/* ���̶�����ӵ�ǰ��Ⱥɸѡ��һ���ӽ����� */
int* Find_Mother_Index(Solution*, Solution*);/* ����ĸ���ڸ�����Ӧ���������� */
int Get_Conflict_Length(int*, int*, int);/* �ҵ�Father_Cross��Mother_cross�в�����ͻ�ı������� */
int *Get_Conflict(int*, int*, int, int);/* �ҵ�Father_Cross��Mother_cross�в�����ͻ�ı��� */
Solution* Handle_Conflict(int*, PDecision_Variable*, int*, int*, int);/*  �����ͻ�Ӵ����� */
int GA_Cross(Solution*, Solution*);	/* GA��������������������в���һ���¸��� */
int GA_Variation(int);				/* �Ժ�����б������ */
void Clone_Group(Solution**, Solution**);/* ������������и��Ʋ��� */
void BestSolution();				/* �������Ž� */
void GA_UpdateGroup();				/* ������Ⱥ */
int GA_Evolution();					/* GA��ѭ������ */


#endif // _GA_H_
