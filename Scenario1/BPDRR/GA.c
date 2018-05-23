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

int InitialGroups()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  Error code
**  Purpose: ��ʼ����Ⱥ
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;
	for (int i = 0; i < Num_group; i++)
	{
		/*  ������ɿɼ�����/©�����˳�򣬹����̶Ӵ���ѡȡ */
		Groups[i].SerialSchedule = Randperm();

		/*  ��SerialSchedule�����е�����ָ�������ÿ�����̶� */
		ERR_CODE(Task_Assignment(Groups[i].SerialSchedule, Groups[i].Schedule));

		Groups[i].C_01 = 0;
		Groups[i].C_02 = 0;
		Groups[i].C_03 = 0;
		Groups[i].C_04 = 0;
		Groups[i].C_05 = 0;
		Groups[i].C_06 = 0;
		Groups[i].P_Reproduction = 0;
		Groups[i].objvalue = 0;

		/* ��ӡ Schedule �ṹ����ֵ */
		for (int j = 0; j < MAX_CREWS; j++)
		{
			printf("\nSchedule[%d]:\n", j);
			Groups[i].Schedule[j].current = Groups[i].Schedule[j].head;
			while (Groups[i].Schedule[j].current != NULL)
			{
				printf("index: %d	type: %d	starttime: %d	endtime: %d\n",
					Groups[i].Schedule[j].current->pointer->index, Groups[i].Schedule[j].current->pointer->type,
					Groups[i].Schedule[j].current->pointer->starttime, Groups[i].Schedule[j].current->pointer->endtime);
				Groups[i].Schedule[j].current = Groups[i].Schedule[j].current->next;
			}
			printf("\n");
		}

		printf("\n***********************************\n");

		if (errcode) err_count++;
	}

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
	int temvar,temindex,flag=-1;	/* ��ʱ���� */
	float y,temflow;				/* ��ʱ���� */
	double sumpdddemand = 0.0, sumbasedemand = 0.0; /* �ڵ�pdd��ˮ�����ڵ������ˮ�� */
	long t, tstep;	/* t: ��ǰʱ��; tstep: ˮ������ʱ�䲽�� */

	/* ����Schedule�ṹ������currentָ�� */
	for (int i = 0; i < MAX_CREWS; i++)
		sol->Schedule[i].current = sol->Schedule[i].head;
	/* ��ʼ������˨�ۼ����� */
	for (int i = 0; i < Nfirefight; i++)
		Firefighting[i].cumu_flow = 0;
	/* ��ʼ��C_05������*/
	for (int i = 0; i < Ndemands; i++)
		Criteria[i] = 0;

					
	/* run epanet analysis engine */
	Open_inp_file(inpfile, "report.rpt", "");
	ENsetstatusreport(0);		/* No Status reporting */
	ENsetreport("MESSAGES NO"); /* No Status reporting */
	ERR_CODE(ENopenH());	if (errcode > 100) err_sum++;	/* Opens the hydraulics analysis system. */
	ERR_CODE(ENinitH(0));	if (errcode > 100) err_sum++;	/* Don't save the hydraulics file */
	
	do 
	{
		ERR_CODE(ENrunH(&t)); if (errcode > 100) err_sum++;

		if ((t >= SimulationStartTime && t <= SimulationEndTime) && (t % Time_Step == 0))
		{
			s = (t / 3600) % 24; //��ǰʱ������Ӧ��ʱ��
			/* ����C_01 */
			for (int i = 0; i < Nhospital; i++) /* Hospital */
			{
				ERR_CODE(ENgetlinkvalue(Hospitals[i].pipeindex, EN_FLOW, &temflow));
				if (errcode > 100) err_sum++;

				y = temflow / ActuralBaseDemand[Hospitals[i].nodeindex - 1][s];
				if (y <= 0.5)
					sol->C_01 += Time_Step / 60;
			}

			if (t >= RestorStartTime)
			{
				for (int i = 0; i < Nfirefight; i++) /* Firefight nodes */
				{
					/*ERR_CODE(ENgetlinkvalue(Firefighting[i].index, EN_FLOW, &temflow));
					printf("[%d]: fireflow: %f, cumu_flow: %f\n", i, temflow, Firefighting[i].cumu_flow);
					*/
					if (Firefighting[i].cumu_flow < MAX_Fire_Volume)
					{
						ERR_CODE(ENgetlinkvalue(Firefighting[i].index, EN_FLOW, &temflow));
						if (errcode > 100) err_sum++;

						y = temflow / Firefighting[i].fire_flow;
						if (y <= 0.5)
							sol->C_01 += Time_Step / 60;
					}
				}
			}
			/* ����C_02��C_04��C_05 */
			for (int i = 0; i < Ndemands; i++)
			{
				ERR_CODE(ENgetlinkvalue(i + Start_pipeindex, EN_FLOW, &temflow));
				if (errcode > 100) err_sum++;

				if (temflow / ActuralBaseDemand[i][s] <= 0.5)
				{
					sol->C_04 += (double)Time_Step / (Ndemands*60.0);

					Criteria[i]++;
					if ((Criteria[i] >= Time_of_Consecutive*(3600 / Time_Step)) && flag==-1)
					{
						sol->C_05++;
						flag = 1;
					}	
				}
				else
				{
					Criteria[i] = 0;
				}

				sumpdddemand += (double)temflow;
				sumbasedemand += (double)ActuralBaseDemand[i][s];
			}
			if (sumpdddemand / sumbasedemand <= 0.95)
				sol->C_02 = (int)t/60;
			
			/* ����C_03 */
			if (sumpdddemand / sumbasedemand < 0.999)
				sol->C_03 += (1.0 - sumpdddemand / sumbasedemand);


			/* ����C_06 */
			for (int i = 0; i < Nbreaks; i++) /* �������б��� */
			{
				int index1;
				float temflow1;
				ERR_CODE(ENgetlinkindex("D437",&index1));
				ERR_CODE(ENgetlinkvalue(index1, EN_FLOW, &temflow1));
				ERR_CODE(ENgetlinkvalue(BreaksRepository[i].pipeindex, EN_FLOW, &temflow));
				ERR_CODE(ENgetnodevalue(BreaksRepository[i].nodeindex, EN_DEMAND, &temflow));
				ERR_CODE(ENgetnodevalue(BreaksRepository[i].nodeindex, EN_PRESSURE, &temflow));
				ERR_CODE(ENgetnodevalue(BreaksRepository[i].nodeindex, EN_EMITTER, &temflow));
				if (errcode > 100)	err_sum++;
				sol->C_06 += (double)temflow;
			}

			for (int i = 0; i < Nleaks; i++) /* ��������©��ܵ� */
			{
				int index1;
				float temflow1;
				ERR_CODE(ENgetlinkindex("D3342", &index1));
				ERR_CODE(ENgetlinkvalue(index1, EN_FLOW, &temflow1));
				ERR_CODE(ENgetnodevalue(LeaksRepository[i].nodeindex, EN_DEMAND, &temflow));
				if (errcode > 100)	err_sum++;
				sol->C_06 += (double)temflow;
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

						Firefighting[i].cumu_flow += temflow * Time_Step * 60;
					}
				}
			}

			/* ���¹ܵ�״̬ */
			for (int i = 0; i < MAX_CREWS; i++)
			{
				if ((sol->Schedule[i].current != NULL) && (sol->Schedule[i].current->pointer->endtime == t))
				{
					printf("time: %d\n", t);
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
						ERR_CODE(Leaks_operation(temindex, EN_STATUS, 1,0));
					}

					sol->Schedule[i].current = sol->Schedule[i].current->next;
				}

			}
		}
		ERR_CODE(ENnextH(&tstep));	if (errcode > 100) err_sum++;
	} while (tstep > 0);

	sol->objvalue = 13.67771712 * (sol->C_01 + sol->C_02 + sol->C_03 + sol->C_04 + sol->C_05 + sol->C_06);

	ERR_CODE(ENclose()); /* �ر�ˮ��ģ�� */
	if (err_sum)
		errcode = 419;
	return errcode;
}
#define _GA
#ifdef _GA

int main(void)
{
	int errcode = 0;	/* ������� */ 
	inpfile = "BBM_Scenario1.inp";	/* ˮ��ģ��inp�ļ�ָ�� */

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
	
	/* ��Ⱥ��ʼ�� */
	ERR_CODE(InitialGroups());
	if (errcode) fprintf(ErrFile, ERR418);

	for (int i = 0; i < Num_group; i++)
	{
		ERR_CODE(Calculate_Objective_Value(&Groups[i]));
		printf("Complited: %d / %d\n", i + 1, Num_group);
	}

	getchar();
	return 0;
}

#endif