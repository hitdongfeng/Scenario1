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
#define EXTERN extern
//#define EXTERN 
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

	/* ��ʼ��Break,Leak�ṹ���е�countֵ */
	Break.current = Break.head;
	while (Break.current != NULL)
	{
		Break.current->count = 0;
		Break.current = Break.current->next;
	}

	Leak.current = Leak.head;
	while (Leak.current != NULL)
	{
		Leak.current->count = 0;
		Leak.current = Leak.current->next;
	}

	return p;
}

void Add_Taskassigmentlist(STaskassigmentlist* list, Scheduleindex *ptr)
/*--------------------------------------------------------------
**  Input:   list: pointer to STaskassigmentlist chain table
**			 ptr: ��Ҫ�����Scheduleindex�ṹ��ָ��
**  Output:  none
**  Purpose: Add a Scheduleindex struct to the tail of the list
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

int Find_Replace_Crow(PDecision_Variable ptr, STaskassigmentlist* schedule)
/*--------------------------------------------------------------
**  Input:   ptr: pointer to Decision_Variable
**  Output:  ִ��Isolate�����Ĺ��̶�����,���򣬷��ش���ֵ-1.
**  Purpose: ��Ա���replace����������ִ��Isolate�����Ĺ��̶�����
**--------------------------------------------------------------*/
{
	for (int i = 0; i < MAX_CREWS; i++)
	{
		schedule[i].current = schedule[i].head;
		while (schedule[i].current != NULL)
		{
			if (schedule[i].current->pointer->type == _Break)
			{
				if (schedule[i].current->pointer->index == ptr->index)
					return i;
			}
			schedule[i].current = schedule[i].current->next;
		}
	}
	return -1;
}

int Find_Finished_Crow(STaskassigmentlist* Schedule)
/*--------------------------------------------------------------
**  Input:   None
**  Output:  �����������Ĺ��̶�����,���򣬷��ش���ֵ-1.
**  Purpose: �����������Ĺ��̶��������Լ���������һ��ָ��
**--------------------------------------------------------------*/
{
	int index=-1;
	long mintime=(long)1E+8;
	for (int i = 0; i < MAX_CREWS; i++)
	{
		if (Schedule[i].head == NULL)
			return i;
		else
		{
			if (mintime > Schedule[i].tail->pointer->endtime)
			{
				mintime = Schedule[i].tail->pointer->endtime;
				index = i;
			}	
		}
	}
	return index;
}

int Task_Assignment(LinkedList *list, STaskassigmentlist* schedule)
/**----------------------------------------------------------------
**  ����:  ��
**  ���:  Error Code
**  ����:  ��SerialSchedule�����е�����ָ�������ÿ�����̶�
**----------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;
	int crowindex;	/* replace�������̶����� */
	
	Scheduleindex* ptr;	/* ��ʱָ�� */

	/* ���г�ʼ�⣬����ӳ�ʼ�� */
	if (NvarsCrew1 > 0 || NvarsCrew2 > 0 || NvarsCrew3 > 0)
	{
		for (int i = 0; i < MAX_CREWS; i++)
		{
			ExistSchedule[i].current = ExistSchedule[i].head;
			while (ExistSchedule[i].current != NULL)
			{
				ptr = (Scheduleindex*)calloc(1, sizeof(Scheduleindex));
				ERR_CODE(MEM_CHECK(ptr));	if (errcode) err_count++;

				ptr->pointer = ExistSchedule[i].current;
				ptr->next = NULL;
				Add_Taskassigmentlist(&schedule[i], ptr);
				ExistSchedule[i].current = ExistSchedule[i].current->next;
			}
		}
	}
	
	/* ��SerialSchedule�����е�����ָ�������ÿ�����̶� */
	list->current = list->head;
	while (list->current != NULL)
	{
		/* ��Ա���Isolate����,��������Ӧ�Ĺ��̶� */
		if (list->current->type == _Isolate)
		{
			crowindex = Find_Finished_Crow(schedule);
			if (crowindex < 0) err_count++;
			
			if (schedule[crowindex].head == NULL)
			{
				list->current->starttime = RestorStartTime;
				list->current->endtime = list->current->starttime + 60 * BreaksRepository[list->current->index].isolate_time;
			}
			else
			{
				list->current->starttime= schedule[crowindex].tail->pointer->endtime;
				list->current->endtime = list->current->starttime + 60 * BreaksRepository[list->current->index].isolate_time;
			}
				ptr = (Scheduleindex*)calloc(1, sizeof(Scheduleindex));
				ptr->pointer = list->current;
				ptr->next = NULL;
				Add_Taskassigmentlist(&schedule[crowindex], ptr);
		}
		/* ��Ա���replace����������ִ��Isolate�����Ĺ��̶����� */
		else if (list->current->type == _Replace)
		{
			crowindex = Find_Replace_Crow(list->current, schedule);
			if (crowindex < 0) err_count++;

			list->current->starttime = schedule[crowindex].tail->pointer->endtime;
			list->current->endtime = list->current->starttime + 3600 * BreaksRepository[list->current->index].replace_time;
			
			ptr = (Scheduleindex*)calloc(1, sizeof(Scheduleindex));
			ptr->pointer = list->current;
			ptr->next = NULL;
			Add_Taskassigmentlist(&schedule[crowindex], ptr);
		}

		/* ���©��repair��������������Ӧ�Ĺ��̶� */
		else if (list->current->type == _Repair)
		{
			crowindex = Find_Finished_Crow(schedule);
			if (crowindex < 0) err_count++;

			if (schedule[crowindex].head == NULL)
			{
				list->current->starttime = RestorStartTime;
				list->current->endtime = list->current->starttime + 3600 * LeaksRepository[list->current->index].repair_time;
			}
			else
			{
				list->current->starttime = schedule[crowindex].tail->pointer->endtime;
				list->current->endtime = list->current->starttime + 3600 * LeaksRepository[list->current->index].repair_time;
			}
			ptr = (Scheduleindex*)calloc(1, sizeof(Scheduleindex));
			ptr->pointer = list->current;
			ptr->next = NULL;
			Add_Taskassigmentlist(&schedule[crowindex], ptr);
		}

		list->current = list->current->next;
	}

	if (err_count)
		errcode = 417;
	return errcode;
}


void FreeMemory(LinkedList*	SerialSchedule,STaskassigmentlist* Schedule)
/*--------------------------------------------------------------
**  Input:   SerialSchedule: ����ָ������ָ��(���е���ָ��)
**			 Schedule: ���̶ӵ���ָ��(������ʼ���������)
**  Output:  none
**  Purpose: free memory
**--------------------------------------------------------------*/
{
	/* �ͷ�Schedule�����ڴ� */
	for (int i = 0; i < MAX_CREWS; i++)
	{
		Schedule[i].current = Schedule[i].head;
		while (Schedule[i].current != NULL)
		{
			Schedule[i].head = Schedule[i].head->next;
			SafeFree(Schedule[i].current);
			Schedule[i].current = Schedule[i].head;
		}
	}

	/* �ͷ�SerialSchedule����ָ�� */
	SerialSchedule->current = SerialSchedule->head;
	while (SerialSchedule->current != NULL)
	{
		SerialSchedule->head = SerialSchedule->head->next;
		SafeFree(SerialSchedule->current);
		SerialSchedule->current = SerialSchedule->head;
	}
}

//#define EVO
#ifdef EVO

int main(void)
{
	int errcode = 0;	//������� 

	STaskassigmentlist* Schedule; /* ���̶ӵ���ָ��(������ʼ���������) */
	Schedule = (STaskassigmentlist*)calloc(MAX_CREWS, sizeof(STaskassigmentlist));
	LinkedList*	SerialSchedule;     /* ����ָ������ָ��(���е���ָ��) */



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

	errcode = Task_Assignment(SerialSchedule, Schedule);

	/* ��ӡ Schedule �ṹ����ֵ */
	for (int i = 0; i < MAX_CREWS; i++)
	{
		printf("\nSchedule[%d]:\n", i);
		Schedule[i].current = Schedule[i].head;
		while (Schedule[i].current != NULL)
		{
			printf("index: %d	type: %d	starttime: %d	endtime: %d\n",
				Schedule[i].current->pointer->index, Schedule[i].current->pointer->type,
				Schedule[i].current->pointer->starttime, Schedule[i].current->pointer->endtime);
			Schedule[i].current = Schedule[i].current->next;
		}
		printf("\n");
	}
	
	
	FreeMemory(SerialSchedule,Schedule);
	getchar();
	return 0;
}

#endif