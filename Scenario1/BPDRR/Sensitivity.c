/*******************************************************************
Name: Sensitivity.c
Purpose: ������ܵ����������ȷ���������ȷ������ܵ���Ҫ�̶�
Data: 5/3/2018
Author: Qingzhou Zhang
Email: wdswater@gmail.com
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "wdstext.h"
#include "wdstypes.h"
#include "wdsfuns.h"
#include "epanet2.h"
#define EXTERN extern
//#define EXTERN 
#include "wdsvars.h"

void Open_inp_file(char *f1, char *f2, char *f3)
/**----------------------------------------------------------------
**  ����:  f1: .inp�ļ�ָ��
		  f2: .rpt�ļ�ָ��
		  f3: .out�ļ�ָ��
**  ���:  ��
**  ����:  ��.inp�ļ�.
**----------------------------------------------------------------*/
{
	int errcode = 0;
	ERR_CODE(ENopen(f1, f2, f3));
	if (errcode)
	{
		fprintf(ErrFile, ERR407);
	}
}

int GetDemand(char* f1, long time)
/**----------------------------------------------------------------
**  ����:  f1:inp�ļ�ָ��; time: ģ��ʱ�̣�����ƣ�
**  ���:  Error code
**  ����:  ��ȡָ��ģ��ʱ����ˮ���ڵ�ʵ����ˮ��
**----------------------------------------------------------------*/
{
	int s;
	int errcode = 0, errsum = 0;
	long t, tstep;		/* t: ��ǰʱ��; tstep: ˮ������ʱ�䲽�� */
	float demand;		/* ��ʱ���������ڴ洢й���� */

	s = (time / 3600) % 24; //��ǰʱ������Ӧ��ʱ��
	/* run epanet analysis engine */
	Open_inp_file(f1, "BBM_EPS.rpt", "");
	ENsetstatusreport(0);		/* No Status reporting */
	ENsetreport("MESSAGES NO"); /* No Status reporting */
	ERR_CODE(ENopenH());	if (errcode > 100) errsum++;	/* Opens the hydraulics analysis system. */
	ERR_CODE(ENinitH(0));	if (errcode > 100) errsum++;	/* Don't save the hydraulics file */

	do 
	{
		ERR_CODE(ENrunH(&t)); if (errcode>100) errsum++;
		if (t == s*3600) /* Begin the restoration */
		{
			for (int i = 0; i < Ndemands; i++)
			{
				ERR_CODE(ENgetnodevalue(i+1, EN_DEMAND, &demand));
				if (errcode > 100) errsum++;
				ActuralBaseDemand[i] = demand;
			}
			break;
		}
		ERR_CODE(ENnextH(&tstep));	if (errcode>100) errsum++;
	} while (tstep>0);
	
	ERR_CODE(ENcloseH());	if (errcode > 100) errsum++;
	ERR_CODE(ENclose());	if (errcode > 100) errsum++;

	if (errsum > 0) errcode = 412;
	return errcode;
}

void Get_FailPipe_keyfacility_Attribute()
/**----------------------------------------------------------------
**  ����:  ��
**  ���:  ��
**  ����:  ��ȡ����ܵ��͹ؼ���������(ҽԺ������˨)�������ֵ
**----------------------------------------------------------------*/
{
	int errcode = 0;
	int nodeindex,pipeindex1, pipeindex2;
	float emitter;

	for (int i = 0; i < Nbreaks; i++)
	{
		ERR_CODE(ENgetnodeindex(BreaksRepository[i].nodeID, &nodeindex));
		if (errcode) fprintf(ErrFile, ERR408, BreaksRepository[i].nodeID);

		BreaksRepository[i].nodeindex = nodeindex;

		ERR_CODE(ENgetlinkindex(BreaksRepository[i].pipeID, &pipeindex1));
		if (errcode) fprintf(ErrFile, ERR409, BreaksRepository[i].pipeID);
			
		BreaksRepository[i].pipeindex = pipeindex1;

		ERR_CODE(ENgetnodevalue(nodeindex, EN_EMITTER, &emitter));
		if (errcode) fprintf(ErrFile, ERR410);
			
		BreaksRepository[i].emittervalue = emitter;

		for (int j = 0; j < BreaksRepository[i].num_isovalve; j++)
		{
			ERR_CODE(ENgetlinkindex(BreaksRepository[i].pipes[j].pipeID, &pipeindex2));
			if (errcode) fprintf(ErrFile, ERR408, BreaksRepository[i].pipes[j].pipeID);
				
			BreaksRepository[i].pipes[j].pipeindex = pipeindex2;
		}
	}
	
	for (int i = 0; i < Nleaks; i++)
	{
		ERR_CODE(ENgetnodeindex(LeaksRepository[i].nodeID, &nodeindex));
		if (errcode) fprintf(ErrFile, ERR408, LeaksRepository[i].nodeID);
			
		LeaksRepository[i].nodeindex = nodeindex;

		ERR_CODE(ENgetlinkindex(LeaksRepository[i].pipeID, &pipeindex1));
		if (errcode) fprintf(ErrFile, ERR409,LeaksRepository[i].pipeID);
			
		LeaksRepository[i].pipeindex = pipeindex1;

		ERR_CODE(ENgetnodevalue(nodeindex, EN_EMITTER, &emitter));
		if (errcode) fprintf(ErrFile, ERR410);
			
		LeaksRepository[i].emittervalue = emitter;
	}

	for (int i = 0; i < Nhospital; i++)
	{
		ERR_CODE(ENgetnodeindex(Hospitals[i].nodeID, &nodeindex)); 
		if (errcode) fprintf(ErrFile, ERR408, Hospitals[i].nodeID);

		ERR_CODE(ENgetlinkindex(Hospitals[i].pipeID, &pipeindex1)); 
		if (errcode) fprintf(ErrFile, ERR409, Hospitals[i].pipeID);

		Hospitals[i].nodeindex = nodeindex;
		Hospitals[i].pipeindex = pipeindex1;
	}

	for (int i = 0; i < Nfirefight; i++)
	{
		ERR_CODE(ENgetlinkindex(Firefighting[i].ID, &pipeindex1));
		if (errcode)
		{
			fprintf(ErrFile, ERR409, Firefighting[i].ID);
			break;
		}
		Firefighting[i].index = pipeindex1;
	}
}

int Visible_Damages_initial(long time)
/**----------------------------------------------------------------
**  ����:  time ģ��ʱ��
**  ���:  Error code
**  ����:  ��ȡģ�⿪ʼʱ�ɼ����ܻ�©��ܵ���Ϣ
**----------------------------------------------------------------*/
{
	int errcode = 0, errsum=0;
	long t, tstep;		/* t: ��ǰʱ��; tstep: ˮ������ʱ�䲽�� */ 
	float flow;	/* ��ʱ���������ڴ洢й���� */
	

	//run epanet analysis engine
	ENsetstatusreport(0);		/* No Status reporting */
	ENsetreport("MESSAGES NO"); /* No Status reporting */
	ERR_CODE(ENopenH());	if (errcode>100) errsum++;	/* Opens the hydraulics analysis system. */
	ERR_CODE(ENinitH(0));	if (errcode>100) errsum++;	/* Don't save the hydraulics file */

	do 
	{
		ERR_CODE(ENrunH(&t)); if (errcode>100) errsum++;
		if (t == time) /* Begin the restoration */
		{
			/* �������б��� */
			for (int i = 0; i < Nbreaks; i++)
			{
				ERR_CODE(ENgetnodevalue(BreaksRepository[i].nodeindex, EN_DEMAND, &flow));
				if (BreaksRepository[i].pipediameter >= 150 || flow > 2.5)
					errcode = Add_tail(&IniVisDemages, _Break, i);
				if (errcode>100)	errsum++;
			}

			/* ��������©��ܵ� */
			for (int i = 0; i < Nleaks; i++)
			{
				ERR_CODE(ENgetnodevalue(LeaksRepository[i].nodeindex, EN_DEMAND, &flow));
				if (LeaksRepository[i].pipediameter >= 300 || flow > 2.5)
					errcode = Add_tail(&IniVisDemages, _Leak, i);
				if (errcode>100)	errsum++;
			}
		}
		ERR_CODE(ENnextH(&tstep));	if (errcode>100) errsum++;
	} while (tstep>0);
	ERR_CODE(ENcloseH());	if (errcode>100) errsum++;

	if (errsum > 0) errcode = 411;
	return errcode;
}

Sercapacity* GetSerCapcity(long time)
/**----------------------------------------------------------------
**  ����:  time: ģ��ʱ�̣�����ƣ�
**  ���:  Error code
**  ����:  ����ָ��ģ��ʱ��ϵͳ��ˮ����
**----------------------------------------------------------------*/
{
	int errcode = 0, errsum = 0;
	int facility_count = 0;
	float x;
	double sumpdddemand = 0.0, sumbasedemand = 0.0;
	double y, meankeyfac = 0.0;
	Sercapacity* ptr = (Sercapacity*)calloc(1, sizeof(Sercapacity));

	for (int i = 0; i < Ndemands; i++)
	{
		ERR_CODE(ENgetlinkvalue(i + Start_pipeindex, EN_FLOW, &x));
		if (errcode > 100) errsum++;
		
		sumpdddemand += (double)x;
		sumbasedemand += (double)ActuralBaseDemand[i];
	}
	ptr->Functionality = sumpdddemand / sumbasedemand;

	for (int i = 0; i <Nhospital; i++)
	{
		ERR_CODE(ENgetlinkvalue(Hospitals[i].pipeindex, EN_FLOW, &x));
		if (errcode > 100) errsum++;

		y = (double)(x / ActuralBaseDemand[Hospitals[i].nodeindex-1]);
		if (y > 0.5)
			facility_count++;

		meankeyfac += y;
	}

	for (int i = 0; i < Nfirefight; i++)
	{
		ERR_CODE(ENgetlinkvalue(Firefighting[i].index, EN_FLOW, &x));
		if (errcode > 100) errsum++;

		y = (double)(x / Firefighting[i].fire_flow);
		if (y > 0.5)
			facility_count++;

		meankeyfac += y;
	}

	ptr->MeankeyFunc = meankeyfac / (double)(Nhospital + Nfirefight);
	ptr->Numkeyfac = facility_count;

	if (errsum > 0)	fprintf(ErrFile, ERR413);

	return ptr;
	
}

//int SensitivityAnalysis()
///**----------------------------------------------------------------
//**  ����:  ��
//**  ���:  Error code
//**  ����:  ������ɼ��ܵ����������ȷ���
//**----------------------------------------------------------------*/
//{
//
//
//
//}

//int main(void)
//{
//	int errcode = 0, errsum = 0;
//	long t, tstep;		/* t: ��ǰʱ��; tstep: ˮ������ʱ�䲽�� */
//	Sercapacity* sercapacity;
//
//	errcode = readdata("data.txt", "err.txt");
//	if (errcode) {
//		fprintf(ErrFile, ERR406);
//		return (406);
//	}
//
//	errcode = GetDemand("BBM_EPS.inp", 1800);
//	if (errcode) {
//		fprintf(ErrFile, ERR412);
//		return (412);
//	}
//
//	Open_inp_file("BBM_Scenario1.inp", "BBM_Scenario1.rpt", "");
//	Get_FailPipe_keyfacility_Attribute();
//
//	ERR_CODE(Visible_Damages_initial(1800));
//	if (errcode) {
//		fprintf(ErrFile, ERR411);
//		return (411);
//	}
//
//
//	/* run epanet analysis engine */
//	ENsetstatusreport(0);		/* No Status reporting */
//	ENsetreport("MESSAGES NO"); /* No Status reporting */
//	ERR_CODE(ENopenH());	if (errcode > 100) errsum++;	/* Opens the hydraulics analysis system. */
//	ERR_CODE(ENinitH(0));	if (errcode > 100) errsum++;	/* Don't save the hydraulics file */
//
//	do
//	{
//		ERR_CODE(ENrunH(&t)); if (errcode > 100) errsum++;
//		if (t == 1800) /* Begin the restoration */
//		{
//			sercapacity = GetSerCapcity(1800);
//			break;
//		}
//		ERR_CODE(ENnextH(&tstep));	if (errcode > 100) errsum++;
//	} while (tstep > 0);
//	ERR_CODE(ENcloseH());	if (errcode > 100) errsum++;
//	ERR_CODE(ENclose());	if (errcode > 100) errsum++;
//
//	IniVisDemages.current = IniVisDemages.head;
//	while (IniVisDemages.current != NULL)
//	{
//		printf("type:%d		index:%d\n", IniVisDemages.current->type, IniVisDemages.current->index);
//		IniVisDemages.current = IniVisDemages.current->next;
//	}
//
//	getchar();
//	fclose(ErrFile);
//
//	if (errsum > 0) errcode = 412;
//	return errcode;
//}
