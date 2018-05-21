/*******************************************************************
Name: GA.c
Purpose: GA�㷨������
Data: 5/3/2018
Author: Qingzhou Zhang
Email: wdswater@gmail.com
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "wdstext.h"
#include "GA.h"
#include "wdsfuns.h"
#include "epanet2.h"
#include "mt19937ar.h"
//#define EXTERN extern
#define EXTERN 
#include "wdsvars.h"

int Memory_Allocation()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  Error code
**  Purpose: ������ڴ�
**--------------------------------------------------------------*/
{
	int errcode = 0, err_sum = 0;

	Groups = (Solution*)calloc(Num_group, sizeof(Solution));
	Offspring = (Solution*)calloc(Num_offs, sizeof(Solution));

	ERR_CODE(MEM_CHECK(Groups));	if (errcode) err_sum++;
	ERR_CODE(MEM_CHECK(Offspring));	if (errcode) err_sum++;

	if (err_sum)
	{
		fprintf(ErrFile, ERR402);
		return (402);
	}

	return errcode;
}

void InitialGroups()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: ��ʼ����Ⱥ
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;
	for (int i = 0; i < Num_group; i++)
	{
		Groups[i].SerialSchedule = Randperm();
		ERR_CODE(Task_Assignment(Groups[i].SerialSchedule, Groups[i].Schedule));

		///* ��ӡSerialSchedule�ṹ����ֵ */
		//Groups[i].SerialSchedule->current = Groups[i].SerialSchedule->head;
		//while (Groups[i].SerialSchedule->current != NULL)
		//{
		//	printf("index: %d	type: %d\n", Groups[i].SerialSchedule->current->index, Groups[i].SerialSchedule->current->type);

		//	Groups[i].SerialSchedule->current = Groups[i].SerialSchedule->current->next;
		//}
		//printf("\n");

		

		///* ��ӡ Schedule �ṹ����ֵ */
		//for (int j = 0; j < MAX_CREWS; j++)
		//{
		//	printf("\nSchedule[%d]:\n", j);
		//	Groups[i].Schedule[j].current = Groups[i].Schedule[j].head;
		//	while (Groups[i].Schedule[j].current != NULL)
		//	{
		//		printf("index: %d	type: %d	starttime: %d	endtime: %d\n",
		//			Groups[i].Schedule[j].current->pointer->index, Groups[i].Schedule[j].current->pointer->type,
		//			Groups[i].Schedule[j].current->pointer->starttime, Groups[i].Schedule[j].current->pointer->endtime);
		//		Groups[i].Schedule[j].current = Groups[i].Schedule[j].current->next;
		//	}
		//	printf("\n");
		//}

		//printf("\n***********************************\n");

		if (errcode) err_count++;

		Groups[i].P_Reproduction = 0;
		Groups[i].objvalue = 0;
	}

}


int main(void)
{
	int errcode = 0;	//������� 

	/* ��ȡdata.txt���� */
	ERR_CODE(readdata("data.txt", "err.txt"));
	if (errcode) {
		fprintf(ErrFile, ERR406); return (406);
	}

	/* ��inp�ļ� */
	Open_inp_file("BBM_Scenario1.inp", "BBM_Scenario1.rpt", "");

	/* ��ȡ����/©��ܵ�����ڵ�����������ϵ�����ܵ�����; ҽԺ������˨�ڵ㡢�ܵ����� */
	Get_FailPipe_keyfacility_Attribute();

	/* ���ɿɼ�����/©�������Ϣ�������ѡ�������� */
	ERR_CODE(Get_Select_Repository());
	if (errcode) {
		fprintf(ErrFile, ERR416); return (416);
	}

	/* Ϊ��ʼ������ڴ� */
	ERR_CODE(Memory_Allocation());
	if (errcode) {
		fprintf(ErrFile, ERR416); return (416);
	}
	
	/* ��Ⱥ��ʼ�� */
	InitialGroups();




	getchar();
	return 0;
}