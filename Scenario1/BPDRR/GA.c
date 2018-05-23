/*******************************************************************
Name: GA.c
Purpose: GA算法主函数
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
**  Purpose: 分配堆内存
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
**  Purpose: 初始化种群
**--------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;
	for (int i = 0; i < Num_group; i++)
	{
		/*  随机生成可见爆管/漏损操作顺序，供工程队从中选取 */
		Groups[i].SerialSchedule = Randperm();

		/*  将SerialSchedule链表中的所有指令分配至每个工程队 */
		ERR_CODE(Task_Assignment(Groups[i].SerialSchedule, Groups[i].Schedule));

		Groups[i].C_01 = 0;
		Groups[i].C_02 = 0;
		Groups[i].C_03 = 0;
		Groups[i].C_04 = 0;
		Groups[i].C_05 = 0;
		Groups[i].C_06 = 0;
		Groups[i].P_Reproduction = 0;
		Groups[i].objvalue = 0;

		///* 打印 Schedule 结构体数值 */
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
	}

	if (err_count)
		errcode = 419;

	return errcode;
}

int Calculate_Objective_Value(Solution* sol)
/*--------------------------------------------------------------
**  Input:   sol: 种群数组指针
**  Output:  Error code
**  Purpose: 计算目标函数值
**--------------------------------------------------------------*/
{
	int errcode = 0, err_sum = 0;	/* 错误编码及计数 */
	int s;							/* 整点时刻 */
	int temvar,temindex;			/* 临时变量 */
	float y,temflow;				/* 临时变量 */
	double sumpdddemand, sumbasedemand; /* 节点pdd需水量、节点基本需水量 */
	long t, tstep;	/* t: 当前时刻; tstep: 水力计算时间步长 */

	/* 设置Schedule结构体链表current指针 */
	for (int i = 0; i < MAX_CREWS; i++)
		sol->Schedule[i].current = sol->Schedule[i].head;
	/* 初始化消火栓累计流量 */
	for (int i = 0; i < Nfirefight; i++)
		Firefighting[i].cumu_flow = 0;
	/* 初始化C_05计数器*/
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
		sumpdddemand = 0.0, sumbasedemand = 0.0;	/* 初始化节点pdd需水量、节点基本需水量 */

		if ((t >= SimulationStartTime && t <= SimulationEndTime) && (t % Time_Step == 0))
		{
			s = (t / 3600) % 24; //当前时刻所对应的时段
			/* 计算C_01 */
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
			/* 计算C_02、C_04、C_05 */
			for (int i = 0; i < Ndemands; i++)
			{
				ERR_CODE(ENgetlinkvalue(i + Start_pipeindex, EN_FLOW, &temflow));
				if (errcode > 100) err_sum++;
				if ((temflow < FLow_Tolerance) || (temflow - ActuralBaseDemand[i][s])>0.5)
					temflow = 0.0;

				if (temflow / ActuralBaseDemand[i][s] <= 0.5)
				{
					sol->C_04 += (double)Time_Step / (Ndemands*60.0);

					Criteria[i].count++;
					if ((Criteria[i].count >= Time_of_Consecutive*(3600 / Time_Step)) && Criteria[i].flag==-1)
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
			
			/* 计算C_03 */
			if (sumpdddemand / sumbasedemand < 0.999)
				sol->C_03 += (1.0 - sumpdddemand / sumbasedemand)*(Time_Step/60);


			/* 计算C_06 */
			for (int i = 0; i < Nbreaks; i++) /* 遍历所有爆管 */
			{
				ERR_CODE(ENgetlinkvalue(BreaksRepository[i].flowindex, EN_FLOW, &temflow));
				if (errcode > 100)	err_sum++;
				if ((temflow < FLow_Tolerance))
					temflow = 0.0;
				sol->C_06 += (double)temflow * Time_Step;
			}

			for (int i = 0; i < Nleaks; i++) /* 遍历所有漏损管道 */
			{
				ERR_CODE(ENgetlinkvalue(LeaksRepository[i].flowindex, EN_FLOW, &temflow));
				if (errcode > 100)	err_sum++;
				if ((temflow < FLow_Tolerance))
					temflow = 0.0;
				sol->C_06 += (double)temflow * Time_Step;
			}
/*************************************************************************************************/
			/* 更新消火栓状态 */
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

			/* 更新管道状态 */
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
					}

					sol->Schedule[i].current = sol->Schedule[i].current->next;
				}

			}
		}
		ERR_CODE(ENnextH(&tstep));	if (errcode > 100) err_sum++;
	} while (tstep > 0);

	sol->objvalue = sol->C_01 + sol->C_02 + sol->C_03 + sol->C_04 + sol->C_05 + sol->C_06/1000000;

	ERR_CODE(ENclose()); /* 关闭水力模型 */
	if (err_sum)
		errcode = 419;
	return errcode;
}

void Calc_Probablity()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: 每次新产生的群体, 计算每个个体的概率
**--------------------------------------------------------------*/
{
	double total_objvalue = 0.0;
	double TempTotal_P = 0.0;

	for (int i = 0; i < Num_group; i++)
		total_objvalue += Groups[i].objvalue;

	for (int i = 0; i < Num_group; i++)
	{
		Groups[i].P_Reproduction = (1.0 / Groups[i].objvalue)*total_objvalue;
		TempTotal_P += Groups[i].P_Reproduction;
	}

	for (int i = 0; i < Num_group; i++)
		Groups[i].P_Reproduction /= TempTotal_P;
}

int Select_Individual()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  个体索引,若为找到，返回错误代码
**  Purpose: 轮盘赌随机从当前总群筛选出一个杂交对象
**--------------------------------------------------------------*/
{
	double selection_P = genrand_real1(); /* 随机生成一个[0,1]随机数 */
	double distribution_P = 0.0;

	for (int i = 0; i < Num_group; i++)
	{
		distribution_P += Groups[i].P_Reproduction;
		if (selection_P < distribution_P)
			return i;
	}

	return 422;
}

void GA_Cross(Solution* Father, Solution* Mother)
{

}



#define _GA
#ifdef _GA

int main(void)
{
	int errcode = 0;	/* 错误编码 */ 
	inpfile = "BBM_Scenario1.inp";	/* 水力模型inp文件指针 */

	/* 读取data.txt数据 */
	ERR_CODE(readdata("data.txt", "err.txt"));
	if (errcode) fprintf(ErrFile, ERR406);

	/* 获取各节点每个时间步长(h)的基本需水量 */
	errcode = GetDemand("BBM_EPS.inp");
	if (errcode) fprintf(ErrFile, ERR412);

	/* 打开inp文件 */
	Open_inp_file(inpfile, "report.rpt", "");

	/* 获取爆管/漏损管道喷射节点索引、喷射系数、管道索引; 医院及消火栓节点、管道索引 */
	Get_FailPipe_keyfacility_Attribute();
	/* 关闭水力模型文件 */
	ERR_CODE(ENclose());	if (errcode > 100) fprintf(ErrFile, ERR420);;


	/* 生成可见爆管/漏损相关信息，供随机选择函数调用 */
	ERR_CODE(Get_Select_Repository());
	if (errcode) fprintf(ErrFile, ERR416);

	/* 为初始解分配内存 */
	ERR_CODE(Memory_Allocation());
	if (errcode) fprintf(ErrFile, ERR417);
	
	/* 种群初始化 */
	ERR_CODE(InitialGroups());
	if (errcode) fprintf(ErrFile, ERR418);

	for (int i = 0; i < Num_group; i++)
	{
		ERR_CODE(Calculate_Objective_Value(&Groups[i]));
		printf("C_01= %d, C_02= %d, C_03= %f, C_04= %f, C_05= %d, C_06= %f, Sum= %f\n",
			Groups[i].C_01, Groups[i].C_02, Groups[i].C_03, Groups[i].C_04, Groups[i].C_05,
			Groups[i].C_06, Groups[i].objvalue);
		//printf("Complited: %d / %d\n", i + 1, Num_group);
	}

	getchar();
	return 0;
}

#endif