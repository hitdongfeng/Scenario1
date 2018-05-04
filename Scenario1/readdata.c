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
#define EXTERN extern
#include "wdsvars.h"

char *Tok[MAX_TOKS]; /* �����ֶ����飬���ڴ洢�ֶ� */
int Ntokens;	/* data.txt��ÿ���ֶ����� */
FILE *InFile;	/* data.txt�ļ�ָ�� */

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

void Open_file(char *f1)
/*----------------------------------------------------------------
**  ����: f1 = �ļ�ָ��
**  ���: ��
**  ����: �������
**  ����: �������ļ�
**----------------------------------------------------------------*/
{
	/* ��ʼ���ļ�ָ��Ϊ NULL */
	InFile = NULL;

	/* �������ļ� */
	if ((InFile = fopen(f1, "rt")) == NULL)
	{
		printf("Can not open the data.txt file!\n");
		assert(0); //��ֹ���򣬷��ش�����Ϣ
	}
}

void Init_pointers()
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: initializes global pointers to NULL
**----------------------------------------------------------------*/
{
	Part_init_solution = NULL;	/* ��ʼ��ָ�� */
	BreaksRepository = NULL;	/* ���ֿܲ�ָ��(���ڴ洢���б���) */
	LeaksRepository = NULL;		/* ©��ܵ��ֿ�ָ��(���ڴ洢����©��ܵ�) */
	Schedule = NULL;			/* ���̶ӵ���ָ�� */
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
**  Output:  returns error code
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

void  Alloc_Memory()
/*----------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Returns: error code
**  Purpose: allocates memory for network data structures
**----------------------------------------------------------------*/
{
	if (Nbreaks > 0)
		BreaksRepository = (SBreaks*)calloc(Nbreaks, sizeof(SBreaks));
	if (Nleaks > 0)
		LeaksRepository = (SLeaks*)calloc(Nleaks, sizeof(SLeaks));
	Schedule = (SCrew*)calloc(MAX_CREWS, sizeof(SCrew));
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

int  Get_float(char *s, double *y)
/*-----------------------------------------------------------
**  ����: *s = character string
**  ���: *y = floating point number
**             returns 1 if conversion successful, 0 if not
**  ����: converts string to floating point number
**-----------------------------------------------------------*/
{
	char *endptr;
	*y = (double)strtod(s, &endptr);
	if (*endptr > 0) return(0);
	return(1);
}

void initialsolution()
/*
**--------------------------------------------------------------
**  Input:   none
**  Output:  none
**  Purpose: processes  initialsolution data
**  Format:
**  [Initial_Solution]
**  Type	index
**--------------------------------------------------------------*/
{
	double x;
	double *p;
	if (Ninivariables > 0)
	{
		p = (double*)calloc(Ntokens, sizeof(double));
		for (int i = 0; i < Ntokens; i++)
		{
			getfloat(Tok[i], &x);
			p[i] = x;
		}
		Initial_data[Ninitialsolutions] = p;
		Ninitialsolutions++;
	}
}