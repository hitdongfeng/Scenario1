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
		return (407);
	}
}

void Get_FailurePipe_Attribute()
/**----------------------------------------------------------------
**  ����:  ��
**  ���:  ��
**  ����:  ��ȡ����ܵ��������ֵ
**----------------------------------------------------------------*/
{
	int errcode = 0;
	int nodeindex,pipeindex1, pipeindex2;
	float emitter;

	for (int i = 0; i < Nbreaks; i++)
	{
		ERR_CODE(ENgetnodeindex(BreaksRepository[i].nodeID, &nodeindex));
		if (errcode){
			fprintf(ErrFile, ERR408, BreaksRepository[i].nodeID);
			return (408);
		}
		BreaksRepository[i].nodeindex = nodeindex;

		ERR_CODE(ENgetlinkindex(BreaksRepository[i].pipeID, &pipeindex1));
		if (errcode) {
			fprintf(ErrFile, ERR409, BreaksRepository[i].pipeID);
			return (409);
		}
		BreaksRepository[i].pipeindex = pipeindex1;

		ERR_CODE(ENgetnodevalue(nodeindex, EN_EMITTER, &emitter));
		if (errcode) {
			fprintf(ErrFile, ERR410);
		}
		BreaksRepository[i].emittervalue = emitter;

		for (int j = 0; j < BreaksRepository[i].num_isovalve; j++)
		{
			ERR_CODE(ENgetlinkindex(BreaksRepository[i].pipes[j].pipeID, &pipeindex2));
			if (errcode) {
				fprintf(ErrFile, ERR408, BreaksRepository[i].pipes[j].pipeID);
				return (408);
			}
			BreaksRepository[i].pipes[j].pipeindex = pipeindex2;
		}
	}
	
	for (int i = 0; i < Nleaks; i++)
	{
		ERR_CODE(ENgetnodeindex(LeaksRepository[i].nodeID, &nodeindex));
		if (errcode) {
			fprintf(ErrFile, ERR408, LeaksRepository[i].nodeID);
			return (408);
		}
		LeaksRepository[i].nodeindex = nodeindex;

		ERR_CODE(ENgetlinkindex(LeaksRepository[i].pipeID, &pipeindex1));
		if (errcode) {
			fprintf(ErrFile, ERR409,LeaksRepository[i].pipeID);
			return (409);
		}
		LeaksRepository[i].pipeindex = pipeindex1;

		ERR_CODE(ENgetnodevalue(nodeindex, EN_EMITTER, &emitter));
		if (errcode) {
			fprintf(ErrFile, ERR410);
		}
		LeaksRepository[i].emittervalue = emitter;
	}
}

int Add_Visdemage_tail(LinkedList *list, int type, int index,long time)
/*--------------------------------------------------------------
**  Input:   list: pointer to LinkedList array
**			 type: ����ܵ�����, 1:����; 2:©��
**			 index: �ܵ��ڲֿ������е�����(��0��ʼ)
**			 time:	Times of demage that is visible
**  Output:  none
**  Purpose: Add a visible demanges struct to the tail of the list
**--------------------------------------------------------------*/
{
	int errcode = 0; /* ��ʼ��������� */
	SCvisible *p;	/* ��ʱ���������ڴ洢�ɼ����ܻ�©��ܵ���Ϣ */
	p = (SCvisible*)calloc(1, sizeof(SCvisible));
	ERR_CODE(MEM_CHECK(p));	if (errcode) return 402;
	p->time = time;
	p->type = type;
	p->Repoindex = index;
	p->next = NULL;

	if (list->head == NULL)
	{
		list->head = p;
	}
	else
	{
		list->tail->next = p;
	}
	list->tail = p;

	return errcode;
}


int Visible_Damages_initial()
/**----------------------------------------------------------------
**  ����:  ��
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
		if (t == 1800) /* Begin the restoration */
		{
			/* �������б��� */
			for (int i = 0; i < Nbreaks; i++)
			{
				ERR_CODE(ENgetnodevalue(BreaksRepository[i].nodeindex, EN_DEMAND, &flow));
				if (BreaksRepository[i].pipediameter >= 150 || flow > 2.5)
					errcode = Add_Visdemage_tail(&IniVisDemages, _Break, i, 1800);
				if (errcode>100)	errsum++;
			}

			/* ��������©��ܵ� */
			for (int i = 0; i < Nleaks; i++)
			{
				ERR_CODE(ENgetnodevalue(LeaksRepository[i].nodeindex, EN_DEMAND, &flow));
				if (LeaksRepository[i].pipediameter >= 300 || flow > 2.5)
					errcode = Add_Visdemage_tail(&IniVisDemages, _Leak, i, 1800);
				if (errcode>100)	errsum++;
			}
			
		}
		ERR_CODE(ENnextH(&tstep));	if (errcode>100) errsum++;
	} while (tstep>0);
	ERR_CODE(ENcloseH());	if (errcode>100) errsum++;

	if (errsum > 0) errcode = 411;
	return errcode;
}






int main(void)
{
	int errcode = 0;

	errcode = readdata("data.txt", "err.txt");
	if (errcode){
		fprintf(ErrFile, ERR406);
		return (406);
	}
	Open_inp_file("BBM_Scenario1.inp", "BBM_Scenario1.rpt", "");
	Get_FailurePipe_Attribute();

	ERR_CODE(Visible_Damages_initial());
	if (errcode) {
		fprintf(ErrFile, ERR411);
		return (411);
	}



	getchar();
	fclose(ErrFile);
	return errcode;
}
