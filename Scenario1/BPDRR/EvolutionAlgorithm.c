/*******************************************************************
Name: EvolutionAlgorithm.c
Purpose: �����㷨�漰����غ���
Data: 5/3/2018
Author: Qingzhou Zhang
Email: wdswater@gmail.com
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "wdstext.h"
#include "wdstypes.h"
#include "wdsfuns.h"
#include "epanet2.h"
#include "mt19937ar.h"
//#define EXTERN extern
#define EXTERN 
#include "wdsvars.h"


Sdamagebranchlist Break, Leak;	/* ���ӱ���/©��ܵ�����ָ�� */
int Num_breaks = 0, Num_leaks = 0;	/* ���ӱ���/©��ܵ����� */
int Break_count, Leak_count;		/* ���ӱ���/©��ܵ��������� */


void Add_damagebranch_list(Sdamagebranchlist* list, Sdamagebranch *ptr)
/*--------------------------------------------------------------
**  Input:   list: pointer to Sdamagebranchlist chain table
**			 ptr: ��Ҫ�����Sdamagebranch�ṹ��ָ��
**  Output:  none
**  Purpose: Add a Sdamagebranch struct to the tail of the list
**--------------------------------------------------------------*/
{
	if (list->head == NULL)
	{
		list->head = ptr;
	}
	else
	{
		list->tail->next = ptr;
	}
	list->tail = ptr;
}

int Get_Select_Repository()
/*--------------------------------------------------------------
**  Input:   ��
**  Output:  Error Code
**  Purpose: ���ɿɼ�����/©�������Ϣ�������ѡ��������
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;	/* ������� */
	Sdamagebranch *ptr;	/* ��ʱ�ṹ��ָ�� */

	/* ��ʼ��Break, Leak, p�ṹ�� */
	Break.head = NULL; Break.tail = NULL; Break.current = NULL;
	Leak.head = NULL; Leak.tail = NULL; Leak.current = NULL;

	/* ��ȡ�ɼ����ܺ�©��ܵ����� */
	decisionlist.current = decisionlist.head;
	while (decisionlist.current != NULL)
	{
		if (decisionlist.current->type == _Break)
		{
			ptr = (Sdamagebranch*)calloc(1, sizeof(Sdamagebranch));
			ERR_CODE(MEM_CHECK(ptr));	if (errcode) err_count++;

			ptr->index = decisionlist.current->index;
			ptr->count = 0;
			ptr->next = NULL;
			Add_damagebranch_list(&Break, ptr);
			Num_breaks++;
		}
		else if (decisionlist.current->type = _Leak)
		{
			ptr = (Sdamagebranch*)calloc(1, sizeof(Sdamagebranch));
			ERR_CODE(MEM_CHECK(ptr));	if (errcode) err_count++;

			ptr->index = decisionlist.current->index;
			ptr->count = 0;
			ptr->next = NULL;
			Add_damagebranch_list(&Leak, ptr);
			Num_leaks++;
		}

		else {
			printf("Type of visible demages error: not Break or Leak!\n");
			assert(0); //��ֹ���򣬷��ش�����Ϣ
		}
		decisionlist.current = decisionlist.current->next;
	}
	if (err_count) errcode = 416;

	return errcode;
}

Sdamagebranch* find_visibledamage_index(Sdamagebranchlist* list, int index,int type, int* OperationType)
/*--------------------------------------------------------------
**  Input:   list: pointer to Sdamagebranchlist chain table
**			 index: ����
**			 number: ����/©��ά�޲�����������,���ܣ�2; ©��: 1.
**  Output:  ����ָ���������ϲ����ṹ��ָ��,���Ҳ���ָ���������򷵻�NULL
**  Purpose: �����������ֵ����ȡָ����������
**--------------------------------------------------------------*/
{
	int count = -1;
	list->current = list->head;
	while (list->current != NULL)
	{
		if (type == _Break)
		{
			if (list->current->count < NUM_BreakOperation)
			{
				++count;
				if (count == index)
				{
					if (list->current->count == 0)
						*OperationType = _Isolate;
					else if (list->current->count == 1)
					{
						*OperationType = _Replace;
						Break_count--;
					}
					
					list->current->count++;
					return (list->current);
				}
				
			}
		}
		else 
		{
			if (list->current->count < NUM_LeakOperation)
			{
				++count;
				if (count == index)
				{
					if (list->current->count == 0)
					{
						*OperationType = _Repair;
						Leak_count--;
					}

					list->current->count++;
					return (list->current);
				}
				
			}
		}

		list->current = list->current->next;
	}

	return NULL;
}

LinkedList* Randperm()
/**----------------------------------------------------------------
**  ����:  ��
**  ���:  ����LinkedListָ��
**  ����:  ������ɿɼ�����/©�����˳�򣬹����̶Ӵ���ѡȡ
**----------------------------------------------------------------*/
{
	int errcode = 0;					/* ������� */
	int temptype;						/* ��ʱ���� */
	int Random_breakindex, Random_leakindex; /* ����/©��������� */
	double randomvalue;					/* ���������[0,1] */
	Sdamagebranch *ptr;					/* ��ʱ�ṹ��ָ�� */
	LinkedList* p;						/* ��ʱ�ṹ��ָ�� */
	
	/* ��ʼ����ز��� */
	p = (LinkedList*)calloc(1, sizeof(LinkedList));
	p->head = NULL; p->tail = NULL; p->current = NULL;
	Break_count = Num_breaks;
	Leak_count = Num_leaks;

	while (Break_count > 0 || Leak_count > 0)
	{
		if (Break_count > 0 && Leak_count > 0)
		{
			randomvalue = genrand_real1();
			if (randomvalue < Break_Weight_Leak)
			{
				randomvalue = genrand_real1();
				Random_breakindex = (int)floor(randomvalue*(Break_count - 0.0001));
				ptr = find_visibledamage_index(&Break, Random_breakindex, _Break,&temptype);
				errcode = Add_tail(p, ptr->index, temptype, 0, 0);
				if (errcode) fprintf(ErrFile, ERR415);
			}
			else
			{
				randomvalue = genrand_real1();
				Random_leakindex = (int)floor(randomvalue*(Leak_count - 0.0001));
				ptr = find_visibledamage_index(&Leak, Random_leakindex, _Leak, &temptype);
				errcode = Add_tail(p, ptr->index, temptype, 0, 0);
				if (errcode) fprintf(ErrFile, ERR415);
			}
		}
		else if (Break_count > 0)
		{
			randomvalue = genrand_real1();
			Random_breakindex = (int)floor(randomvalue*(Break_count - 0.0001));
			ptr = find_visibledamage_index(&Break, Random_breakindex, _Break, &temptype);
			errcode = Add_tail(p, ptr->index, temptype, 0, 0);
			if (errcode) fprintf(ErrFile, ERR415);
		}
		else
		{
			randomvalue = genrand_real1();
			Random_leakindex = (int)floor(randomvalue*(Leak_count - 0.0001));
			ptr = find_visibledamage_index(&Leak, Random_leakindex, _Leak, &temptype);
			errcode = Add_tail(p, ptr->index, temptype, 0, 0);
			if (errcode) fprintf(ErrFile, ERR415);
		}
	}

	return p;
}



int main(void)
{
	int errcode = 0;	//������� 
	


	/* ��ȡdata.txt���� */
	errcode = readdata("data.txt", "err.txt");
	if (errcode) { fprintf(ErrFile, ERR406); return (406); }

	/* ��inp�ļ� */
	Open_inp_file("BBM_Scenario1.inp", "BBM_Scenario1.rpt", "");

	/* ��ȡ����/©��ܵ�����ڵ�����������ϵ�����ܵ�����; ҽԺ������˨�ڵ㡢�ܵ����� */
	Get_FailPipe_keyfacility_Attribute();

	/* ���ɿɼ�����/©�������Ϣ�������ѡ�������� */
	errcode = Get_Select_Repository();
	if (errcode) { fprintf(ErrFile, ERR416); return (416); }

	SerialSchedule = Randperm();

	/* ��ӡSerialSchedule�ṹ����ֵ */
	SerialSchedule->current = SerialSchedule->head;
	while (SerialSchedule->current != NULL)
	{
		printf("index: %d	type: %d\n", SerialSchedule->current->index, SerialSchedule->current->type);

		SerialSchedule->current = SerialSchedule->current->next;
	}


	
	getchar();
	return 0;
}