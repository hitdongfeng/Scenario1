/*******************************************************************
Name: GA.c
Purpose: GA�㷨������
Data: 5/3/2018
Author: Qingzhou Zhang
Email: wdswater@gmail.com
********************************************************************/

#include <stdio.h>
#include <time.h>
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

	Groups = (Solution**)calloc(Num_group, sizeof(Solution*));
	for (int i = 0; i < Num_group; i++)
	{
		Groups[i] = (Solution*)calloc(1, sizeof(Solution));
		ERR_CODE(MEM_CHECK(Groups[i]));	if (errcode) err_sum++;
	}

	Offspring = (Solution**)calloc(Num_offs, sizeof(Solution*));

	ERR_CODE(MEM_CHECK(Groups));	if (errcode) err_sum++;
	ERR_CODE(MEM_CHECK(Offspring));	if (errcode) err_sum++;

	if (err_sum)
	{
		fprintf(ErrFile, ERR402);
		return (402);
	}

	return errcode;
}

void Free_Solution(Solution* ptr)
/*--------------------------------------------------------------
**  Input:   ptr: Solution�ṹ��ָ��
**  Output:  None
**  Purpose: �ͷ�Solution�ṹ���ڴ�
**--------------------------------------------------------------*/
{
	/* �ͷ�SerialSchedule����ָ�� */
	if (ptr != NULL)
	{
		if (ptr->SerialSchedule != NULL)
		{
			ptr->SerialSchedule->current = ptr->SerialSchedule->head;
			while (ptr->SerialSchedule->current != NULL)
			{
				ptr->SerialSchedule->head = ptr->SerialSchedule->head->next;
				SafeFree(ptr->SerialSchedule->current);
				ptr->SerialSchedule->current = ptr->SerialSchedule->head;
			}
		}

		/* �ͷ�Schedule�����ڴ� */
		for (int i = 0; i < MAX_CREWS; i++)
		{
			if (ptr->Schedule[i].head != NULL)
			{
				ptr->Schedule[i].current = ptr->Schedule[i].head;
				while (ptr->Schedule[i].current != NULL)
				{
					ptr->Schedule[i].head = ptr->Schedule[i].head->next;
					SafeFree(ptr->Schedule[i].current);
					ptr->Schedule[i].current = ptr->Schedule[i].head;
				}
			}
		}

		SafeFree(ptr);
	}
}

void Free_GAmemory()
/*--------------------------------------------------------------
**  Input:   None
**  Output:  None
**  Purpose: �ͷ������ڴ�
**--------------------------------------------------------------*/
{
	/* �ͷ���ȺGroups�ڴ� */
	for (int i = 0; i < Num_group; i++)
		Free_Solution(Groups[i]);

	/* �ͷ��Ӵ�Offspring�ڴ� */
	for (int i = 0; i < Num_offs; i++)
		Free_Solution(Offspring[i]);

}

int InitialGroups()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  Error code
**  Purpose: ��ʼ����Ⱥ
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;
	Chrom_length = 0; /* ����Ⱦɫ�峤�� */
	for (int i = 0; i < Num_group; i++)
	{
		/*  ������ɿɼ�����/©�����˳�򣬹����̶Ӵ���ѡȡ */
		Groups[i]->SerialSchedule = Randperm();
		/* ��ӡSerialSchedule�ṹ����ֵ */
		Groups[i]->SerialSchedule->current = Groups[i]->SerialSchedule->head;
		while (Groups[i]->SerialSchedule->current != NULL)
		{
			printf("index: %d	type: %d\n", Groups[i]->SerialSchedule->current->index, Groups[i]->SerialSchedule->current->type);

			Groups[i]->SerialSchedule->current = Groups[i]->SerialSchedule->current->next;
		}

		/*  ��SerialSchedule�����е�����ָ�������ÿ�����̶� */
		ERR_CODE(Task_Assignment(Groups[i]->SerialSchedule, Groups[i]->Schedule));

		Groups[i]->C_01 = 0;Groups[i]->C_02 = 0;Groups[i]->C_03 = 0;
		Groups[i]->C_04 = 0;Groups[i]->C_05 = 0;Groups[i]->C_06 = 0;
		Groups[i]->P_Reproduction = 0;Groups[i]->objvalue = 0;

		/*��ӡ Schedule �ṹ����ֵ */
	for (int j = 0; j < MAX_CREWS; j++)
	{
		printf("\nSchedule[%d]:\n", j);
		Groups[i]->Schedule[j].current = Groups[i]->Schedule[j].head;
		while (Groups[i]->Schedule[j].current != NULL)
		{
			printf("index: %d	type: %d	starttime: %d	endtime: %d\n",
				Groups[i]->Schedule[j].current->pointer->index, Groups[i]->Schedule[j].current->pointer->type,
				Groups[i]->Schedule[j].current->pointer->starttime, Groups[i]->Schedule[j].current->pointer->endtime);
			Groups[i]->Schedule[j].current = Groups[i]->Schedule[j].current->next;
		}
		printf("\n");
	}

		if (errcode) err_count++;
	}

	/* ��ȡ����Ⱦɫ�峤�� */
	Groups[0]->SerialSchedule->current = Groups[0]->SerialSchedule->head;
	while (Groups[0]->SerialSchedule->current != 0)
	{
		Chrom_length++;
		Groups[0]->SerialSchedule->current = Groups[0]->SerialSchedule->current->next;
	}

	/* ����ÿ���������Ӧ��ֵ */
	for (int i = 0; i < Num_group; i++)
	{
		ERR_CODE(Calculate_Objective_Value(Groups[i]));
		if (errcode)	err_count++;
		printf("Complited: %d / %d\n", i + 1, Num_group);
	}

	Calc_Probablity();/* ����ÿ�������ѡ����� */
	BestSolution(); /* ���浱ǰ���Ž� */
		
	if (err_count)
		errcode = 419;
	return errcode;
}

int Calculate_Objective_Value(Solution* sol)
/*--------------------------------------------------------------
**  Input:   sol: ��Ⱥ����ָ��
**  Output:  Error code
**  Purpose: ����Ŀ�꺯��ֵ
**--------------------------------------------------------------*/
{
	int errcode = 0, err_sum = 0;	/* ������뼰���� */
	int s;							/* ����ʱ�� */
	int temvar,temindex;			/* ��ʱ���� */
	float y,temflow;				/* ��ʱ���� */
	double sumpdddemand, sumbasedemand; /* �ڵ�pdd��ˮ�����ڵ������ˮ�� */
	long t, tstep;	/* t: ��ǰʱ��; tstep: ˮ������ʱ�䲽�� */

	/* ����Schedule�ṹ������currentָ�� */
	for (int i = 0; i < MAX_CREWS; i++)
		sol->Schedule[i].current = sol->Schedule[i].head;
	/* ��ʼ������˨�ۼ����� */
	for (int i = 0; i < Nfirefight; i++)
		Firefighting[i].cumu_flow = 0;
	/* ��ʼ��C_05������*/
	for (int i = 0; i < Ndemands; i++)
	{
		Criteria[i].count = 0;
		Criteria[i].flag = -1;
	}
					
	/* run epanet analysis engine */
	Open_inp_file(inpfile, "report.rpt", "");
	ENsetstatusreport(0);		/* No Status reporting */
	ENsetreport("MESSAGES NO"); /* No Status reporting */
	ERR_CODE(ENopenH());	if (errcode > 100) err_sum++;	/* Opens the hydraulics analysis system. */
	ERR_CODE(ENinitH(0));	if (errcode > 100) err_sum++;	/* Don't save the hydraulics file */
	
	do 
	{
		ERR_CODE(ENrunH(&t)); if (errcode > 100) err_sum++;
		sumpdddemand = 0.0, sumbasedemand = 0.0;	/* ��ʼ���ڵ�pdd��ˮ�����ڵ������ˮ�� */

		if ((t >= SimulationStartTime && t <= SimulationEndTime) && (t % Time_Step == 0))
		{
			s = (t / 3600) % 24; //��ǰʱ������Ӧ��ʱ��
			/* ����C_01 */
			for (int i = 0; i < Nhospital; i++) /* Hospital */
			{
				ERR_CODE(ENgetlinkvalue(Hospitals[i].pipeindex, EN_FLOW, &temflow));
				if (errcode > 100) err_sum++;
				if ((temflow < FLow_Tolerance) || (temflow - ActuralBaseDemand[Hospitals[i].nodeindex - 1][s]) > 0.5)
					temflow = 0.0;

				y = temflow / ActuralBaseDemand[Hospitals[i].nodeindex - 1][s];
				if (y <= 0.5)
					sol->C_01 += Time_Step / 60;
					
			}

			if (t >= RestorStartTime)
			{
				for (int i = 0; i < Nfirefight; i++) /* Firefight nodes */
				{
					if (Firefighting[i].cumu_flow < MAX_Fire_Volume)
					{
						ERR_CODE(ENgetlinkvalue(Firefighting[i].index, EN_FLOW, &temflow));
						if (errcode > 100) err_sum++;
						if ((temflow < FLow_Tolerance) || (temflow - Firefighting[i].fire_flow)>0.5)
							temflow = 0.0;

						y = temflow / Firefighting[i].fire_flow;
						if (y <= 0.5)
							sol->C_01 += Time_Step / 60;
							
					}
				}
			}
			///* ����C_02��C_04��C_05 */
			for (int i = 0; i < Ndemands; i++)
			{
				ERR_CODE(ENgetlinkvalue(i + Start_pipeindex, EN_FLOW, &temflow));
				if (errcode > 100) err_sum++;
				if ((temflow < FLow_Tolerance) || (temflow - ActuralBaseDemand[i][s]) > 0.5)
					temflow = 0.0;

				if (temflow / ActuralBaseDemand[i][s] <= 0.5)
				{
					sol->C_04 += (double)Time_Step / (Ndemands*60.0);

					Criteria[i].count++;
					if ((Criteria[i].count >= Time_of_Consecutive * (3600 / Time_Step)) && Criteria[i].flag == -1)
					{
						sol->C_05++;
						Criteria[i].flag = 1;
					}
				}
				else
				{
					Criteria[i].count = 0;
				}

				sumpdddemand += (double)temflow;
				sumbasedemand += (double)ActuralBaseDemand[i][s];
			}
			if (sumpdddemand / sumbasedemand <= 0.95)
				sol->C_02 = (int)t/60;
			
			/* ����C_03 */
			if (sumpdddemand / sumbasedemand < 0.999)
				sol->C_03 += (1.0 - sumpdddemand / sumbasedemand)*(Time_Step/60);


			/* ����C_06 */
			for (int i = 0; i < Nbreaks; i++) /* �������б��� */
			{
				ERR_CODE(ENgetlinkvalue(BreaksRepository[i].flowindex, EN_FLOW, &temflow));
				if (errcode > 100)	err_sum++;
				if ((temflow < FLow_Tolerance))
					temflow = 0.0;
				sol->C_06 += (double)temflow * Time_Step;
			}

			for (int i = 0; i < Nleaks; i++) /* ��������©��ܵ� */
			{
				ERR_CODE(ENgetlinkvalue(LeaksRepository[i].flowindex, EN_FLOW, &temflow));
				if (errcode > 100)	err_sum++;
				if ((temflow < FLow_Tolerance))
					temflow = 0.0;
				sol->C_06 += (double)temflow * Time_Step;
			}
/*************************************************************************************************/
			/* ��������˨״̬ */
			if (t >= RestorStartTime)
			{
				for (int i = 0; i < Nfirefight; i++)
				{
					if (Firefighting[i].cumu_flow >= MAX_Fire_Volume)
					{
						ERR_CODE(ENsetlinkvalue(Firefighting[i].index, EN_SETTING, 0));
						if (errcode > 100) err_sum++;
					}
					else
					{
						ERR_CODE(ENgetlinkvalue(Firefighting[i].index, EN_FLOW, &temflow));
						if (errcode > 100) err_sum++;
						if ((temflow < FLow_Tolerance) || (temflow - Firefighting[i].fire_flow>0.5))
							temflow = 0.0;

						Firefighting[i].cumu_flow += temflow * Time_Step;
					}
				}
			}

			/* ���¹ܵ�״̬ */
			for (int i = 0; i < MAX_CREWS; i++)
			{
				if ((sol->Schedule[i].current != NULL) && (sol->Schedule[i].current->pointer->endtime == t))
				{
					temindex = sol->Schedule[i].current->pointer->index;
					temvar = sol->Schedule[i].current->pointer->type;
					if (temvar == _Isolate)
					{
						ERR_CODE(Breaks_Adjacent_operation(_Isolate,temindex, EN_STATUS,0,0));
						if(errcode) err_sum++;
					}
					else if (temvar == _Replace)
					{
						ERR_CODE(Breaks_Adjacent_operation(_Reopen, temindex, EN_STATUS, 1, 0));
						if (errcode) err_sum++;
					}
					else if (temvar == _Repair)
					{
						ERR_CODE(Leaks_operation(temindex, EN_STATUS, 1, 0));
						if (errcode) err_sum++;
					}

					sol->Schedule[i].current = sol->Schedule[i].current->next;
				}

			}
		}
		ERR_CODE(ENnextH(&tstep));	if (errcode > 100) err_sum++;
	} while (tstep > 0);

	sol->objvalue = sol->C_01 + sol->C_02 + sol->C_03 + sol->C_04 + sol->C_05 + sol->C_06/1000000;
	//sol->objvalue = sol->C_01;
	//sol->objvalue = sol->C_01 + sol->C_02 + sol->C_03 + sol->C_04 + sol->C_05;

	ERR_CODE(ENcloseH()); if (errcode) err_sum++;
	ERR_CODE(ENclose()); if (errcode) err_sum++; /* �ر�ˮ��ģ�� */
	if (err_sum)
		errcode = 419;
	return errcode;
}

void Calc_Probablity()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: ÿ���²�����Ⱥ��, ����ÿ������ĸ���
**--------------------------------------------------------------*/
{
	double total_objvalue = 0.0;
	double TempTotal_P = 0.0;

	for (int i = 0; i < Num_group; i++)
		total_objvalue += Groups[i]->objvalue;

	for (int i = 0; i < Num_group; i++)
	{
		Groups[i]->P_Reproduction = (1.0 / Groups[i]->objvalue)*total_objvalue;
		TempTotal_P += Groups[i]->P_Reproduction;
	}

	for (int i = 0; i < Num_group; i++)
		Groups[i]->P_Reproduction /= TempTotal_P;
}

int Select_Individual()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  ��������,��Ϊ�ҵ������ش������
**  Purpose: ���̶�����ӵ�ǰ��Ⱥɸѡ��һ���ӽ�����
**--------------------------------------------------------------*/
{
	double selection_P = genrand_real1(); /* �������һ��[0,1]����� */
	double distribution_P = 0.0;

	for (int i = 0; i < Num_group; i++)
	{
		distribution_P += Groups[i]->P_Reproduction;
		if (selection_P < distribution_P)
			return i;
	}

	return 422;
}

int* Find_Mother_Index(Solution* Father, Solution* Mother)
/*--------------------------------------------------------------
**  Input:   Father: ��������, Mother: ĸ������
**  Output:  ĸ���ڸ�����Ӧ��������������ָ��
**  Purpose: ����ĸ���ڸ�����Ӧ����������
**--------------------------------------------------------------*/
{
	int i = 0,j;
	int *Mother_index = (int*)calloc(Chrom_length, sizeof(int));

	Mother->SerialSchedule->current = Mother->SerialSchedule->head;
	while (Mother->SerialSchedule->current != NULL)
	{
		j = 0;
		Father->SerialSchedule->current = Father->SerialSchedule->head;
		while (Father->SerialSchedule->current != NULL)
		{
			if ((Father->SerialSchedule->current->index == Mother->SerialSchedule->current->index)
				&& (Father->SerialSchedule->current->type == Mother->SerialSchedule->current->type))
			{
				Mother_index[i] = j;
				break;
			}
			j++;
			Father->SerialSchedule->current = Father->SerialSchedule->current->next;
		}

		i++;
		Mother->SerialSchedule->current = Mother->SerialSchedule->current->next;
	}
	
	return Mother_index;
}

int Get_Conflict_Length(int* Detection_Cross, int* Model_Cross, int Length_Cross)
/*--------------------------------------------------------------
**  Input:   Detection_Cross:��ǰ�����ĸ���, ���ҳ�ͻ�Ķ���
**			 Model_Cross: ��һ��������
**			 Length_Cross:����ı�������
**  Output:  ������ͻ�ı�������
**  Purpose: �ҵ�Father_Cross��Mother_cross�в�����ͻ�ı�������
**--------------------------------------------------------------*/
{
	int Conflict_Length = 0;
	int flag_Conflict;
	for (int i = 0; i < Length_Cross; i++)
	{
		flag_Conflict = 1;  // �ж��Ƿ����ڳ�ͻ  
		for (int j = 0; j < Length_Cross; j++)
		{
			if (Detection_Cross[i] == Model_Cross[j])
			{
				// �����ڶ���ѭ��  
				j = Length_Cross;
				flag_Conflict = 0;  // �ó��в����ڳ�ͻ  
			}
		}
		if (flag_Conflict)
		{
			Conflict_Length++;
		}
	}
	return Conflict_Length;
}

int *Get_Conflict(int* Detection_Cross, int* Model_Cross, int Length_Cross, int Length_Conflict) 
/*--------------------------------------------------------------
**  Input:   Detection_Cross:��ǰ�����ĸ���, ���ҳ�ͻ�Ķ���
**			 Model_Cross: ��һ��������
**			 Length_Cross:����ı�������
**			 Length_Conflict: ��ͻ�ı�������
**  Output:  Error code
**  Purpose: �ҵ�Father_Cross��Mother_cross�в�����ͻ�ı���
**--------------------------------------------------------------*/
{
	int count = 0; /* ������ */
	int flag_Conflict; /* ��ͻ��־, 0:����ͻ, 1: ��ͻ */
	int *Conflict = (int *)calloc(Length_Conflict, sizeof(int));
	
	for (int i = 0; i < Length_Cross; i++)
	{
		flag_Conflict = 1;  // �ж��Ƿ����ڳ�ͻ  
		for (int j = 0; j < Length_Cross; j++)
		{
			if (Detection_Cross[i] == Model_Cross[j])
			{
				// �����ڶ���ѭ��  
				j = Length_Cross;
				flag_Conflict = 0;  // �ó��в����ڳ�ͻ  
			}
		}
		if (flag_Conflict)
		{
			Conflict[count] = Detection_Cross[i];
			count++;
		}
	}
	return Conflict;
}

Solution* Handle_Conflict(int* ConflictSolution, PDecision_Variable*pointer, int *Detection_Conflict, int *Model_Conflict, int Length_Conflict)
/*--------------------------------------------------------------
**  Input:   ConflictSolution:��Ҫ����ĳ�ͻ����
**			 pointer: �洢�����������̲���ָ��
**			 Detection_Conflict: �洢��ͻλ�õĸ�������
**			 Model_Conflict: �洢��ͻλ�õ�ĸ������
**			 Length_Conflict: ��ͻ�ı�������
**  Output:  ����õ��Ӵ�����ָ��
**  Purpose: �����ͻ�Ӵ�����
**--------------------------------------------------------------*/
{
	int errcode = 0;
	int flag; /* ��ͻ��ʶ */
	int index;
	int temp=0; /* ��ʱ���� */
	PDecision_Variable ptr; /* ��ʱ���� */
	Solution* Offspring;
	LinkedList* p;	/* ��ʱ�ṹ��ָ�� */
	/* ��ʼ����ز��� */
	Offspring = (Solution*)calloc(1, sizeof(Solution));
	p = (LinkedList*)calloc(1, sizeof(LinkedList));
	ERR_CODE(MEM_CHECK(Offspring));	if (errcode) {fprintf(ErrFile, ERR402);}
	ERR_CODE(MEM_CHECK(p));	if (errcode) {fprintf(ErrFile, ERR402);}
	Offspring->C_01 = 0; Offspring->C_02 = 0; Offspring->C_03 = 0;
	Offspring->C_04 = 0; Offspring->C_05 = 0; Offspring->C_06 = 0;
	Offspring->P_Reproduction = 0; Offspring->objvalue = 0;
	p->head = NULL; p->tail = NULL; p->current = NULL;

	for (int i = 0; i < Length_Conflict; i++)
	{
		flag = 0;
		index = 0;

		/* [0, IndexCross_i) Ѱ�ҳ�ͻ */
		for (index = 0; index < IndexCross_i; index++)
		{
			if (Model_Conflict[i] == ConflictSolution[index])
			{
				flag = 1;
				break;
			}
		}
		/* ��һ��û�ҵ�, ��ʣ��Ĳ���(���˽����Ļ������) */
		if (!flag)
		{
			/* [IndexCross_i + 1, Chrom_length) Ѱ�ҳ�ͻ */
			for (index = IndexCross_j + 1; index < Chrom_length; index++)
			{
				if (Model_Conflict[i] == ConflictSolution[index])
					break;
			}
		}
		ConflictSolution[index] = Detection_Conflict[i];
	}

	for (int i = 0; i < Chrom_length; i++)
	{
		if (pointer[ConflictSolution[i]]->type != _Repair)
		{
			for (int j = i+1; j < Chrom_length; j++)
			{
				if ((pointer[ConflictSolution[i]]->index == pointer[ConflictSolution[j]]->index)
					&& (pointer[ConflictSolution[j]]->type != _Repair))
				{
					if (pointer[ConflictSolution[i]]->type > pointer[ConflictSolution[j]]->type)
					{
						temp= ConflictSolution[i];
						ConflictSolution[i] = ConflictSolution[j];
						ConflictSolution[j] = temp;
					}
					break;
				}
			}
		}
	}

	/* ����ת��Ϊ�Ӵ����� */
	for (int i = 0; i < Chrom_length; i++)
	{
		ptr = pointer[ConflictSolution[i]];
		ERR_CODE(Add_tail(p,ptr->index,ptr->type,0,0));
		if (errcode) {fprintf(ErrFile, ERR423);}
	}
	/*  ��SerialSchedule�����е�����ָ�������ÿ�����̶� */
	Offspring->SerialSchedule = p;
	ERR_CODE(Task_Assignment(Offspring->SerialSchedule,Offspring->Schedule));

	return Offspring;
}

int GA_Cross(Solution* Father, Solution* Mother)
/*--------------------------------------------------------------
**  Input:   Father: ��������, Mother: ĸ������
**  Output:  Error code
**  Purpose: GA��������������������в���һ���¸���
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0; /* ������� */
	int temp;	/* ��ʱ���� */
	int Length_Cross;    /* ����ĸ��� */
	int *Father_index, *Mother_index;
	PDecision_Variable* Father_pointer;
	int *Father_cross, *Mother_cross; /* �������� */
	int *Conflict_Father, *Conflict_Mother;  /* �洢��ͻ��λ�� */   
	int Length_Conflict;	/* ��ͻ�ĸ��� */ 

	/* �����ڴ� */
	Father_index = (int*)calloc(Chrom_length, sizeof(int));
	Father_pointer = (PDecision_Variable*)calloc(Chrom_length, sizeof(PDecision_Variable));
	ERR_CODE(MEM_CHECK(Father_index));	if (errcode) err_count++;
	ERR_CODE(MEM_CHECK(Father_pointer));	if (errcode) err_count++;
	for (int i = 0; i < Chrom_length; i++)
		Father_index[i] = i;
	Mother_index = Find_Mother_Index(Father, Mother);

	temp = 0;
	Father->SerialSchedule->current = Father->SerialSchedule->head;
	while (Father->SerialSchedule->current != NULL)
	{
		Father_pointer[temp] = Father->SerialSchedule->current;
		temp++;
		Father->SerialSchedule->current = Father->SerialSchedule->current->next;
	}

/* �����������λ�ã���֤ IndexCross_i < IndexCross_j */
	IndexCross_i = (int)floor(genrand_real1()*(Chrom_length-0.0001));
	IndexCross_j = (int)floor(genrand_real1()*(Chrom_length - 0.0001));
	while ((IndexCross_i == IndexCross_j) || abs(IndexCross_i - IndexCross_j) == (Chrom_length - 1))
	{
		IndexCross_j = (int)floor(genrand_real1()*(Chrom_length - 0.0001));
	}
	if (IndexCross_i > IndexCross_j)
	{
		temp = IndexCross_i;
		IndexCross_i = IndexCross_j;
		IndexCross_j = temp;
	}

	/* �������� */
	Length_Cross = IndexCross_j - IndexCross_i+1; /* ����ı������� */
	Father_cross = (int*)calloc(Length_Cross, sizeof(int)); /* �����Ŵ������ */
	Mother_cross = (int*)calloc(Length_Cross, sizeof(int)); /* ĸ���Ŵ������ */
	ERR_CODE(MEM_CHECK(Father_cross));	if (errcode) err_count++;
	ERR_CODE(MEM_CHECK(Father_cross));	if (errcode) err_count++;
	temp = 0;
	for (int i = IndexCross_i; i <= IndexCross_j; i++)
	{
		Father_cross[temp] = Father_index[i];
		Mother_cross[temp] = Mother_index[i];
		temp++;
	}

	// ��ʼ���� - �ҵ�Father_Cross��Mother_cross�в�����ͻ�ı���
	Length_Conflict = Get_Conflict_Length(Father_cross, Mother_cross, Length_Cross);
	Conflict_Father = Get_Conflict(Father_cross, Mother_cross, Length_Cross, Length_Conflict);
	Conflict_Mother = Get_Conflict(Mother_cross, Father_cross, Length_Cross, Length_Conflict);

	/*  Father and Mother ��������� */
	for (int i = IndexCross_i; i <= IndexCross_j; i++)
	{
		temp = Father_index[i];
		Father_index[i] = Mother_index[i];
		Mother_index[i] = temp;
	}

	/* ���������ĸ�������ĳ�ͻ���� */
	Solution* Descendant_ONE = Handle_Conflict(Father_index, Father_pointer, Conflict_Father, Conflict_Mother, Length_Conflict);
	Solution* Descendant_TWO = Handle_Conflict(Mother_index, Father_pointer, Conflict_Mother, Conflict_Father, Length_Conflict);

	Offspring[Length_SonSoliton++] = Descendant_ONE;
	Offspring[Length_SonSoliton++] = Descendant_TWO;

	/* �ͷ��ڴ� */
	SafeFree(Father_index);
	SafeFree(Mother_index);
	SafeFree(Father_pointer);
	SafeFree(Father_cross);
	SafeFree(Mother_cross);
	SafeFree(Conflict_Father);
	SafeFree(Conflict_Mother);

	if (err_count)
		errcode = 424;
	return errcode;
}

int GA_Variation(int Index_Offspring)
/*--------------------------------------------------------------
**  Input:   Index_Offspring: �����������
**  Output:  Error code
**  Purpose: �Ժ�����б������
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0; /* ������� */
	int temp;	/* ��ʱ���� */
	int IndexVariation_i, IndexVariation_j;
	int *Index;
	PDecision_Variable* Index_pointer;
	LinkedList* p;	/* ��ʱ�ṹ��ָ�� */
	Solution* Offptr;
	/* ��ʼ����ز��� */
	Offptr = (Solution*)calloc(1, sizeof(Solution));
	p = (LinkedList*)calloc(1, sizeof(LinkedList));
	ERR_CODE(MEM_CHECK(Offptr));	if (errcode) {fprintf(ErrFile, ERR402);}
	ERR_CODE(MEM_CHECK(p));	if (errcode) {fprintf(ErrFile, ERR402);}
	Offptr->C_01 = 0; Offptr->C_02 = 0; Offptr->C_03 = 0;
	Offptr->C_04 = 0; Offptr->C_05 = 0; Offptr->C_06 = 0;
	Offptr->P_Reproduction = 0; Offptr->objvalue = 0;
	p->head = NULL; p->tail = NULL; p->current = NULL;

	/* �����ڴ� */
	Index = (int*)calloc(Chrom_length, sizeof(int));
	Index_pointer = (PDecision_Variable*)calloc(Chrom_length, sizeof(PDecision_Variable));
	ERR_CODE(MEM_CHECK(Index));	if (errcode) err_count++;
	ERR_CODE(MEM_CHECK(Index_pointer));	if (errcode) err_count++;
	for (int i = 0; i < Chrom_length; i++)
		Index[i] = i;

	temp = 0;
	Offspring[Index_Offspring]->SerialSchedule->current = Offspring[Index_Offspring]->SerialSchedule->head;
	while (Offspring[Index_Offspring]->SerialSchedule->current != NULL)
	{
		Index_pointer[temp] = Offspring[Index_Offspring]->SerialSchedule->current;
		temp++;
		Offspring[Index_Offspring]->SerialSchedule->current = Offspring[Index_Offspring]->SerialSchedule->current->next;
	}

	/* ������������������ʾ���������λ��, ������λ�ý��� */
	IndexVariation_i = (int)floor(genrand_real1()*(Chrom_length - 0.0001));
	IndexVariation_j = (int)floor(genrand_real1()*(Chrom_length - 0.0001));
	while ((IndexVariation_i == IndexVariation_j))
	{
		IndexVariation_j = (int)floor(genrand_real1()*(Chrom_length - 0.0001));
	}

	/* ����λ�� */
	temp = Index[IndexVariation_i];
	Index[IndexVariation_i] = Index[IndexVariation_j];
	Index[IndexVariation_j] = temp;

	for (int i = 0; i < Chrom_length; i++)
	{
		if (Index_pointer[Index[i]]->type != _Repair)
		{
			for (int j = i + 1; j < Chrom_length; j++)
			{
				if ((Index_pointer[Index[i]]->index == Index_pointer[Index[j]]->index)
					&& (Index_pointer[Index[j]]->type != _Repair))
				{
					if (Index_pointer[Index[i]]->type > Index_pointer[Index[j]]->type)
					{
						temp = Index[i];
						Index[i] = Index[j];
						Index[j] = temp;
					}
					break;
				}
			}
		}
	}
	/* ����ת��Ϊ�Ӵ����� */
	for (int i = 0; i < Chrom_length; i++)
	{
		ERR_CODE(Add_tail(p, Index_pointer[Index[i]]->index, Index_pointer[Index[i]]->type, 0, 0));
		if (errcode) {fprintf(ErrFile, ERR423);}
	}
	/*  ��SerialSchedule�����е�����ָ�������ÿ�����̶� */
	Offptr->SerialSchedule = p;
	ERR_CODE(Task_Assignment(Offptr->SerialSchedule, Offptr->Schedule));

	Free_Solution(Offspring[Index_Offspring]);
	Offspring[Index_Offspring] = Offptr;

	SafeFree(Index);
	SafeFree(Index_pointer);

	if (err_count)
		errcode = 425;

	return errcode;
}

void Clone_Group(Solution** group, Solution** son)
/*--------------------------------------------------------------
**  Input:  group:��Ⱥ����ָ��, son: �������ָ��
**  Output:  None
**  Purpose: ������������и��Ʋ���
**--------------------------------------------------------------*/
{
	Solution* temp;
	temp = *group;
	*group = *son;
	*son = temp;
}

void BestSolution()
/*--------------------------------------------------------------
**  Input:   None
**  Output:  None
**  Purpose: �������Ž�
**--------------------------------------------------------------*/
{
	int index = 0;
	double Optimalvalue;
	Optimalvalue = Groups[0]->objvalue;
	for (int i = 1; i < Num_group; i++)
	{
		if (Optimalvalue > Groups[i]->objvalue)
		{
			Optimalvalue = Groups[i]->objvalue;
			index = i;
		}
	}

	/* ��ӡ Schedule �ṹ����ֵ */
	for (int j = 0; j < MAX_CREWS; j++)
	{
		fprintf(TemSolution,"Schedule[%d]:\n", j);
		Groups[index]->Schedule[j].current = Groups[index]->Schedule[j].head;
		while (Groups[index]->Schedule[j].current != NULL)
		{
			fprintf(TemSolution,"index: %d	type: %d	starttime: %d	endtime: %d\n",
				Groups[index]->Schedule[j].current->pointer->index, Groups[index]->Schedule[j].current->pointer->type,
				Groups[index]->Schedule[j].current->pointer->starttime, Groups[index]->Schedule[j].current->pointer->endtime);
			Groups[index]->Schedule[j].current = Groups[index]->Schedule[j].current->next;
		}
		fprintf(TemSolution, "\n");
	}
	/* ��ӡBestSolution */
	fprintf(TemSolution, "C_01= %d, C_02= %d, C_03= %f, C_04= %f, C_05= %d, C_06= %f, Sum= %f\n",
		Groups[index]->C_01, Groups[index]->C_02, Groups[index]->C_03, Groups[index]->C_04, Groups[index]->C_05,
		Groups[index]->C_06, Groups[index]->objvalue);
	fprintf(TemSolution, "\n***********************************\n");

}

void GA_UpdateGroup()
/*--------------------------------------------------------------
**  Input:   None
**  Output:  None
**  Purpose: ������Ⱥ
**--------------------------------------------------------------*/
{
	Solution* tempSolution;
	/* �ȶ��Ӵ� - Offspring ������Ӧ��ֵ�������� - ����[��Ŀ��ֵ��С����] */  
	for (int i = 0; i < Num_offs; i++)
	{
		for (int j = Num_offs - 1; j > i; j--)
		{
			if (Offspring[i]->objvalue > Offspring[j]->objvalue)
			{
				tempSolution = Offspring[i];
				Offspring[i] = Offspring[j];
				Offspring[j] = tempSolution;
			}
		}
	}

	/* ������Ⱥ */
	for (int i = 0; i < Num_offs; i++)
	{
		for (int j = 0; j < Num_group; j++)
		{
			if (Offspring[i]->objvalue < Groups[j]->objvalue)
			{
				Clone_Group(&Groups[j], &Offspring[i]);
				break;
			}
		}
	}

	/* �Ӵ�ɾ�� */
	for(int i=0;i<Num_offs;i++)
		Free_Solution(Offspring[i]);

	BestSolution();
}

int GA_Evolution()
/*--------------------------------------------------------------
**  Input:   None
**  Output:  Error code;
**  Purpose: GA��ѭ������
**--------------------------------------------------------------*/
{
	int errcode = 0, err_sum = 0;	/* ������� */
	int iter = 0;	/* �������������� */
	int M;	/* ������� */
	int Father_index, Mother_index;	/* ������ĸ���������� */
	double Is_Crossover;	/* ������������ */
	double Is_mutation;		/* ������������ */

	while (iter < Num_iteration)
	{
		fprintf(TemSolution, "Iteration: %d\n", iter + 1); /* д���������ļ� */
		printf("********Iteration: %d / %d **********\n", iter + 1, Num_iteration); /* ���������̨ */
		
		/* 1.ѡ�� */
		Father_index = Select_Individual();
		Mother_index = Select_Individual();

		/* ��ֹFather��Mother����ͬһ������ -> �Խ�( ��ĸΪͬһ������ʱ, ĸ������ѡ��, ֱ����ĸΪ��ͬ�ĸ���Ϊֹ ) */
		while (Mother_index == Father_index)
		{
			Mother_index = Select_Individual();
		}

		/* 2.����, �洢��ȫ�ֱ��� Offspring[] ���� - ͨ��M���ӽ�, ����2M���¸���, 2M >= Num_group */
		M = Num_group - Num_group / 2;
		Length_SonSoliton = 0;	/* �Ŵ������ĸ������, ���������ۼ� */

		while (M)
		{
			Is_Crossover = genrand_real1();
			if (Is_Crossover <= P_crossover)
			{
				ERR_CODE(GA_Cross(Groups[Father_index], Groups[Mother_index]));
				if (errcode) err_sum++;
				M--;
			}
		}

		/* 3.����, ��� Offspring[] ���� */
		//total_objvalue = 0.0;

		for (int IndexVariation = 0; IndexVariation < Length_SonSoliton; IndexVariation++)
		{
			Is_mutation = genrand_real1();
			if (Is_mutation <= P_mutation)
			{
				ERR_CODE(GA_Variation(IndexVariation));
				if (errcode) err_sum++;
			}

			/*  �������촦���,���¼�����Ӧ��ֵ  */
			ERR_CODE(Calculate_Objective_Value(Offspring[IndexVariation]));
			if (errcode) err_sum++;

			//total_objvalue += Offspring[IndexVariation]->objvalue;
			printf("Complited: %d / %d\n", IndexVariation + 1, Length_SonSoliton);
		}
		
		/* 4.���¸���, �������: ����+�Ӵ� */
		GA_UpdateGroup();
		Calc_Probablity();
		iter++;
	}

	if (err_sum)
		errcode = 426;
	return errcode;
}



#define _GA
#ifdef _GA

int main(void)
{
	int errcode = 0;	/* ������� */ 
	inpfile = "BBM_Scenario1.inp";	/* ˮ��ģ��inp�ļ�ָ�� */
	time_t T_begin = clock();

	/* ��ȡdata.txt���� */
	ERR_CODE(readdata("data.txt", "err.txt"));
	if (errcode) fprintf(ErrFile, ERR406);

	/* ��ȡ���ڵ�ÿ��ʱ�䲽��(h)�Ļ�����ˮ�� */
	errcode = GetDemand("BBM_EPS.inp");
	if (errcode) fprintf(ErrFile, ERR412);

	/* ��inp�ļ� */
	Open_inp_file(inpfile, "report.rpt", "");

	/* ��ȡ����/©��ܵ�����ڵ�����������ϵ�����ܵ�����; ҽԺ������˨�ڵ㡢�ܵ����� */
	Get_FailPipe_keyfacility_Attribute();
	/* �ر�ˮ��ģ���ļ� */
	ERR_CODE(ENclose());	if (errcode > 100) fprintf(ErrFile, ERR420);;

	/* ���ɿɼ�����/©�������Ϣ�������ѡ�������� */
	ERR_CODE(Get_Select_Repository());
	if (errcode) fprintf(ErrFile, ERR416);

	/* Ϊ��ʼ������ڴ� */
	ERR_CODE(Memory_Allocation());
	if (errcode) fprintf(ErrFile, ERR417);

	/* �����Ž�洢�ļ� */
	if ((TemSolution = fopen("TemSolution.txt", "wt")) == NULL)
	{
		printf("Can not open the TemSolution.txt!\n");
		assert(0); //��ֹ���򣬷��ش�����Ϣ
	}
	
	/* ��Ⱥ��ʼ�� */
	ERR_CODE(InitialGroups());
	if (errcode) fprintf(ErrFile, ERR418);

	/* GA��ѭ������ */
	ERR_CODE(GA_Evolution());
	if (errcode) fprintf(ErrFile, ERR426);

	/*�ͷ��ڴ�*/
	//Emptymemory();
	//Free_GAmemory();
	fclose(InFile);
	fclose(ErrFile);
	fclose(TemSolution);
	time_t Tend = clock();

	getchar();
	return 0;
}

#endif