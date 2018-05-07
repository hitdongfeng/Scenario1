/*******************************************************************
Name: readdata.c
Purpose: ���ڶ�ȡdata.txt�ļ��е�����
Data: 5/3/2018
Author: Qingzhou Zhang
Email: wdswater@gmail.com
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "wdstext.h"
#include "wdstypes.h"
#define EXTERN 
#include "wdsvars.h"
#define MAXERRS  5   /* ������Ϣ�ۻ������� */

FILE *InFile;
char *Tok[MAX_TOKS]; /* �����ֶ����飬���ڴ洢�ֶ� */
int Ntokens;		 /* data.txt��ÿ���ֶ����� */
int	break_count;	 /* ���ܼ��� */
int	leak_count;		 /* ©��ܵ����� */

/* ����data.txt�ļ��������� */
char *Sect_Txt[] = { "[Initial_Solution]",
					"[BREAKS]",
					"[LEAKS]",
					NULL
				   };

/* ����data.txt�ļ�����ö�� */
enum Sect_Type {
	_Initial_Solution,
	_BREAKS,
	_LEAKS,
	_END
};


int  str_comp(char *s1, char *s2)
/*---------------------------------------------------------------
**  Input:   s1 = character string; s2 = character string                                
**  Output:  1 if s1 is same as s2, 0 otherwise                                                      
**  Purpose: case insensitive comparison of strings s1 & s2  
**---------------------------------------------------------------*/
{
	for (int i = 0; U_CHAR(s1[i]) == U_CHAR(s2[i]); i++)
		if (!s1[i + 1] && !s2[i + 1]) return(1);
	return(0);
}                                       

void Open_file(char *f1,char *f2)
/*----------------------------------------------------------------
**  Input: f1 = data.txt�ļ�ָ��, f2 = ���󱨸��ļ�ָ��
**  Output: none
**  Purpose: �����ݺʹ��󱨸��ļ�
**----------------------------------------------------------------*/
{
	/* ��ʼ���ļ�ָ��Ϊ NULL */
	InFile = NULL;
	ErrFile = NULL;

	if (str_comp(f1, f2))
	{
		printf("Cannot use duplicate file names!\n");
		assert(0); //��ֹ���򣬷��ش�����Ϣ
	}

	/* ֻ����ʽ�������ļ� */
	if ((InFile = fopen(f1, "rt")) == NULL || (ErrFile = fopen(f2, "wt")) == NULL)
	{
		printf("Can not open the data.txt or error report file!\n");
		assert(0); //��ֹ���򣬷��ش�����Ϣ
	}
}

void initializeList(LinkedList *list)
/*----------------------------------------------------------------
**  Input:   *list, pointer to list
**  Output:  none
**  Purpose: initializes LinkedList pointers to NULL
**----------------------------------------------------------------*/
{
	list->head = NULL;
	list->tail = NULL;
	list->current = NULL;
}

void Init_pointers()
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes global pointers to NULL
**----------------------------------------------------------------*/
{
	//Part_init_solution = NULL;	/* ��ʼ��ָ�� */
	BreaksRepository = NULL;	/* ���ֿܲ�ָ��(���ڴ洢���б���) */
	LeaksRepository = NULL;		/* ©��ܵ��ֿ�ָ��(���ڴ洢����©��ܵ�) */
	Schedule = NULL;			/* ���̶ӵ���ָ�� */
	initializeList(&linkedlist);/* ���߱���ָ��ṹ�� */
}

int  Str_match(char *str, char *substr)
/*--------------------------------------------------------------
**  Input:   *str    = string being searched
**           *substr = substring being searched for
**  Output:  returns 1 if substr found in str, 0 if not
**  Purpose: sees if substr matches any part of str
**--------------------------------------------------------------*/
{
	int i, j;

	if (!substr[0]) return(0);	/* Fail if substring is empty */

	/* Skip leading blanks of str. */
	for (i = 0; str[i]; i++)
		if (str[i] != ' ') break;

	/* Check if substr matches remainder of str. */
	for (i = i, j = 0; substr[j]; i++, j++)
		if (!str[i] || U_CHAR(str[i]) != U_CHAR(substr[j]))
			return(0);
	return(1);
}

int  Find_match(char *line, char *keyword[])
/*--------------------------------------------------------------
**  Input:   *line      = line from input file
**           *keyword[] = list of NULL terminated keywords
**  Output:  returns index of matching keyword or
**           -1 if no match found
**  Purpose: determines which keyword appears on input line
**--------------------------------------------------------------*/
{
	int i = 0;
	while (keyword[i] != NULL)
	{
		if (Str_match(line, keyword[i])) return(i);
		i++;
	}
	return(-1);
}

void  Get_count()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: determines number of breaks, leaks and inivarialbles
**--------------------------------------------------------------*/
{
	char  line[MAX_LINE + 1];	/* Line from data.txt file    */
	char  *tok;                 /* First token of line          */
	int   sect, newsect;        /* data.txt sections          */

	/* Initialize network component counts */
	Nbreaks = 0;		/* ���ܹܵ����� */
	Nleaks = 0;			/* ©ʧ�ܵ����� */
	Ninivariables = 0;	/* ��ʼ��������� */
	sect = -1;			/* data.txt������Ƭ������ */

	/* Make pass through data.txt counting number of each parameter */
	while (fgets(line, MAX_LINE, InFile) != NULL)
	{
		/* Skip blank lines & those beginning with a comment */
		tok = strtok(line, DATA_SEPSTR);
		if (tok == NULL) continue;
		if (*tok == ';') continue;

		/* Check if line begins with a new section heading */
		if (*tok == '[')
		{
			newsect = Find_match(tok, Sect_Txt);
			if (newsect >= 0)
			{
				sect = newsect;
				if (sect == _END) break;
				continue;
			}
			else continue;
		}
		/* Add to count of current component */
		switch (sect)
		{
			case _Initial_Solution:  Ninivariables++;    break;
			case _BREAKS:	Nbreaks++;	break;
			case _LEAKS:	Nleaks++;	break;
			default: break;
		}
	}
}

int  Alloc_Memory()
/*----------------------------------------------------------------
**  Input:   none
**  Output:  error code
**  Returns: error code
**  Purpose: allocates memory for breaks and leanks 
**----------------------------------------------------------------*/
{
	int errcode = 0, err_count = 0;
	
	if (Nbreaks > 0)
		BreaksRepository = (SBreaks*)calloc(Nbreaks, sizeof(SBreaks));

	if (Nleaks > 0)
		LeaksRepository = (SLeaks*)calloc(Nleaks, sizeof(SLeaks));

	Schedule = (SCrew*)calloc(MAX_CREWS, sizeof(SCrew));

	ERR_CODE(MEM_CHECK(BreaksRepository));	if (errcode) err_count++;
	ERR_CODE(MEM_CHECK(LeaksRepository));	if (errcode) err_count++;
	ERR_CODE(MEM_CHECK(Schedule));	if (errcode) err_count++;

	if (err_count)
	{
		fprintf(ErrFile, ERR402);
		return (402);
	}

	return errcode;
}

int  Get_tokens(char *s)
/*--------------------------------------------------------------
**  ����: *s = string to be tokenized
**  ���: returns number of tokens in s
**  ����: scans string for tokens, saving pointers to them
**       in module global variable Tok[]
**--------------------------------------------------------------*/
{
	int  len, m, n;
	char *c;

	/* Begin with no tokens */
	for (n = 0; n < MAX_TOKS; n++) Tok[n] = NULL;
	n = 0;

	/* Truncate s at start of comment */
	c = strchr(s, ';');
	if (c) *c = '\0';
	len = strlen(s);

	/* Scan s for tokens until nothing left */
	while (len > 0 && n < MAX_TOKS)
	{
		m = strcspn(s, DATA_SEPSTR);      /* Find token length */
		len -= m + 1;                     /* Update length of s */
		if (m == 0) s++;				  /* No token found */
		else
		{
			if (*s == '"')                /* Token begins with quote */
			{
				s++;                      /* Start token after quote */
				m = strcspn(s, "\"\n\r"); /* Find end quote (or EOL) */
			}
			s[m] = '\0';                  /* Null-terminate the token */
			Tok[n] = s;                   /* Save pointer to token */
			n++;                          /* Update token count */
			s += m + 1;                   /* Begin next token */
		}
	}
	return(n);
}

int  Get_int(char *s, int *y)
/*-----------------------------------------------------------
**  ����: *s = character string
**  ���: *y = int point number
**             returns 1 if conversion successful, 0 if not
**  ����: converts string to int point number
**-----------------------------------------------------------*/
{
	char *endptr;
	*y = (int)strtod(s, &endptr);
	if (*endptr > 0) return(0);
	return(1);
}

int  Get_float(char *s, float *y)
/*-----------------------------------------------------------
**  ����: *s = character string
**  ���: *y = float point number
**             returns 1 if conversion successful, 0 if not
**  ����: converts string to floating point number
**-----------------------------------------------------------*/
{
	char *endptr;
	*y = (float)strtod(s, &endptr);
	if (*endptr > 0) return(0);
	return(1);
}

void Add_tail(LinkedList *list, int type, int index)
/*--------------------------------------------------------------
**  Input:   list: pointer to LinkedList array
**			 type: �ܵ�����, 1:���ܸ���; 2:�����滻; 3:©���޸�; 4:����
**			 index: �ܵ���������,��0��ʼ
**  Output:  none
**  Purpose: Add a Decision_Variable struct to the tail of the list
**--------------------------------------------------------------*/
{
	PDecision_Variable p = (PDecision_Variable)calloc(1, sizeof(struct Decision_Variable));
	p->type = type;
	p->index = index;
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
}

//void Add_plan(SCrew* crew, long time)
//
//{
//	PDecision_Variable ptr;
//
//	for (int i = 0; i < MAX_CREWS; i++)
//	{
//		if (crew[i].cumulative_time == time)
//		{
//			ptr = linkedlist.head;
//			if (linkedlist.head->next == NULL)
//			{
//				linkedlist.head = NULL;
//				linkedlist.tail = NULL;
//			}
//			else
//				linkedlist.head = linkedlist.head->next;
//			
//			ptr->next = NULL;
//			
//			if (crew[i].Plan.head = NULL)
//				crew[i].Plan.head = ptr;
//			else
//				crew[i].Plan.tail->next= ptr;
//			crew[i].Plan.tail = ptr;
//		}
//	}
//
//}

int Initial_Solution()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  errcode code
**  Purpose: processes  initialsolution data
**  Format:
**  [Initial_Solution]
**  Type	index
**--------------------------------------------------------------*/
{
	int x,y;
	
	if (Ninivariables > 0)
	{

		if (!Get_int(Tok[0], &x))	return (403); /* ��ֵ���ʹ��󣬺��зǷ��ַ� */	
		if (!Get_int(Tok[1], &y))	return (403); /* ��ֵ���ʹ��󣬺��зǷ��ַ� */

		Add_tail(&linkedlist, x, y);
	}
	return 0;
}

int Breaks_Value()
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  error code
**  Purpose: processes  initialsolution data
**  Format:
**  [Initial_Solution]
**  PipeID BreakID Diameter(mm)	pipes_closed_for_isolation Isolation_time(min) replacement(h)
**--------------------------------------------------------------*/
{
	int errcode = 0;
	int time1, time2;
	float dia;
	SFailurePipe* ptr= (SFailurePipe*)calloc(Ntokens-5, sizeof(SFailurePipe));
	ERR_CODE(MEM_CHECK(ptr)); if (errcode) return(402);

	strncpy(BreaksRepository[break_count].pipeID, Tok[0], MAX_ID);
	strncpy(BreaksRepository[break_count].nodeID, Tok[1], MAX_ID);

	if (!Get_float(Tok[2], &dia)) return (403); /* ��ֵ���ʹ��󣬺��зǷ��ַ� */
	BreaksRepository[break_count].pipediameter = dia;

	for (int i = 3; i < Ntokens - 2; i++)
		strncpy(ptr[i-3].pipeID, Tok[i], MAX_ID);
	BreaksRepository[break_count].pipes = ptr;
	BreaksRepository[break_count].num_isovalve = Ntokens - 5;

	if (!Get_int(Tok[Ntokens - 2], &time1)) return (403); /* ��ֵ���ʹ��󣬺��зǷ��ַ� */
	if (!Get_int(Tok[Ntokens - 1], &time2)) return (403); /* ��ֵ���ʹ��󣬺��зǷ��ַ� */ 
	BreaksRepository[break_count].isolate_time = time1;
	BreaksRepository[break_count].replace_time = time2;

	break_count++;

	return (errcode);
}

int Leaks_Value()
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  error code
**  Purpose: processes  initialsolution data
**  Format:
**  [Initial_Solution]
**  PipeID LeakID Diameter(mm) reparation(h)
**--------------------------------------------------------------*/
{
	int time1;
	float dia;

	strncpy(LeaksRepository[leak_count].pipeID, Tok[0], MAX_ID);
	strncpy(LeaksRepository[leak_count].nodeID, Tok[1], MAX_ID);

	if (!Get_float(Tok[2], &dia))	return (403);
	LeaksRepository[leak_count].pipediameter = dia;

	if (!Get_int(Tok[Ntokens - 1], &time1))	return (403);
	LeaksRepository[leak_count].repair_time = time1;

	leak_count++;

	return 0;
}

int  newline(int sect, char *line)
/*--------------------------------------------------------------
**  Input:   sect  = current section of input file
**           *line = line read from input file
**  Output:  returns error code or 0 if no error found
**  Purpose: processes a new line of data from input file
**--------------------------------------------------------------*/
{
	switch (sect)
	{
	case _Initial_Solution:	return(Initial_Solution()); break;
	case _BREAKS:	return(Breaks_Value()); break;
	case _LEAKS:	return(Leaks_Value()); break;
	}
	return(403);
}

int  readdata(char *f1, char *f2)
/*----------------------------------------------------------------
**  Input:   f1 = pointer to name of data.txt file; 
**			 f2 = pointer to name of errcode report file;
**  Output:  error code
**  Purpose: opens data file & reads parameter data
**----------------------------------------------------------------*/
{
	int		errcode = 0,
			inperr, errsum = 0;	  /* ���������������� */
	char	line[MAX_LINE + 1],   /* Line from input data file       */
			wline[MAX_LINE + 1];  /* Working copy of input line      */
	int		sect, newsect;        /* Data sections                   */
	
	Ntokens = 0;			/* ÿ���ֶ����� */
	break_count = 0;	    /* ���ܼ��� */
	leak_count = 0;			/* ©��ܵ����� */

	Init_pointers();			/* Initialize global pointers to NULL. */
	Open_file(f1,f2);			/* Open input & report files */
	Get_count();				/* ��ȡ������Ͳ��������� */

	errcode = Alloc_Memory();	/* Ϊ������Ͳ����ṹ������ڴ� */
	if (errcode)
	{
		fprintf(ErrFile, ERR402);
		return (402);
	}

	rewind(InFile);				/* ��ָ��ָ���ļ���ͷ */

	while (fgets(line, MAX_LINE, InFile) != NULL)
	{

		/* Make copy of line and scan for tokens */
		strcpy(wline, line);
		Ntokens = Get_tokens(wline);

		/* Skip blank lines and comments */
		if (Ntokens == 0) continue;
		if (*Tok[0] == ';') continue;

		/* ����ַ����Ƿ񳬹���ÿ����󳤶�*/
		if (strlen(line) >= MAX_LINE)
		{
			fprintf(ErrFile, ERR404);
			fprintf(ErrFile, "%s\n", line);
			errsum++;
		}

		/* Check if at start of a new input section */
		if (*Tok[0] == '[')
		{
			newsect = Find_match(Tok[0], Sect_Txt);
			if (newsect >= 0)
			{
				sect = newsect;
				if (sect == _END) break;
				continue;
			}
			else
			{
				fprintf(ErrFile, ERR405,line);
				errsum++;
				break;
			}
		}
		/* Otherwise process next line of input in current section */
		else
		{
			inperr = newline(sect, line);
			if (inperr > 0)
			{
				fprintf(ErrFile, ERR403);
				errsum++;
			}
		}
		/* �������ļ�ĩβ��ﵽ������Ϣ�������ʱ������whileѭ�� */
		if (errsum == MAXERRS) break;
	}   /* End of while */

	if (errsum > 0)  errcode = 406; //�����ļ�����һ����ദ����

	return (errcode);
}

void Emptymemory()
/*--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: free memory
**--------------------------------------------------------------*/
{
	/* �ͷ�BreaksRepository�����ڴ� */
	for (int i = 0; i < Nbreaks; i++)
	{
		SafeFree(BreaksRepository[i].pipes);
		SafeFree(BreaksRepository[i]);
	}
	/* �ͷ�LeaksRepository�����ڴ� */
	for (int i = 0; i < Nleaks; i++)
		SafeFree(LeaksRepository[i]);

	/* �ͷ�Schedule�����ڴ� */
	for (int i = 0; i < MAX_CREWS; i++)
	{
		Schedule[i].Plan.current = Schedule[i].Plan.head;
		while (Schedule[i].Plan.current != NULL)
		{
			Schedule[i].Plan.head = Schedule[i].Plan.head->next;
			SafeFree(Schedule[i].Plan.current);
			Schedule[i].Plan.current = Schedule[i].Plan.head;
		}
	}
}

//void File_close()
///*----------------------------------------------------------------
//**  Input:   none
//**  Output:  none
//**  Returns: none
//**  Purpose: frees all memory & files used by BPDRR
//**----------------------------------------------------------------*/
//{
//	if (InFile != NULL) fclose(InFile);
//	if (ErrFile != NULL) fclose(ErrFile);
//}


//int main(void)
//{
//	int errcode;
//
//	errcode = readdata("data.txt", "err.txt");
//	fclose(ErrFile);
//
//	for (int i = 0; i < Nbreaks; i++)
//	{
//		printf("%s	%s	%f	", BreaksRepository[i].pipeID, BreaksRepository[i].nodeID, BreaksRepository[i].pipediameter);
//		for (int j = 0; j < BreaksRepository[i].num_isovalve; j++)
//			printf("%s ", BreaksRepository[i].pipes[j].pipeID);
//		printf("%d	%d,	%d	%d	%d\n",
//			BreaksRepository[i].isolate_time, BreaksRepository[i].replace_time, BreaksRepository[i].isolate_flag, BreaksRepository[i].replace_flag, BreaksRepository[i].reopen_flag);
//	}
//	printf("--------------------\n");
//	for (int i = 0; i < Nleaks; i++)
//	{
//		printf("%s	%s	%f	%d, %d	%d\n", LeaksRepository[i].pipeID, LeaksRepository[i].nodeID, LeaksRepository[i].pipediameter, LeaksRepository[i].repair_time, LeaksRepository[i].repair_flag, LeaksRepository[i].reopen_flag);
//	}
//
//	printf("--------------------\n");
//	linkedlist.current = linkedlist.head;
//	while (linkedlist.current != NULL)
//	{
//		printf("%d	%d\n", linkedlist.current->type, linkedlist.current->index);
//		linkedlist.current = linkedlist.current->next;
//	}
//
//	getchar();
//
//	return 0;
//
//}
