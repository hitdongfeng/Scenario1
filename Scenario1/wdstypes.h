#ifndef WDSTYPES_H
#define WDSTYPES_H
#include<stdlib.h>

/*-----------------------------
          ȫ�ֳ����Ķ���
-----------------------------*/
#define   MAX_LINE   500          /* data.txt�ļ�ÿ������ַ��� */
#define   MAX_TOKS   50           /* data.txt�ļ�ÿ������ֶ��� */
#define   MAX_CREWS	 3			   /* ���̶����� */

#define	  MAX_ID	 31            /* ID����ַ��� */
#define   DATA_SEPSTR    " \t\n\r" /* data.txt�ļ��ֶηָ�� */
#define   U_CHAR(x) (((x) >= 'a' && (x) <= 'z') ? ((x)&~32) : (x)) /* ��ĸת��д */

#define   MEM_CHECK(x)  (((x) == NULL) ? 402 : 0 )   /* �ڴ���䲻�ɹ��᷵�ؿ�ֵNULL����ʾ�������402(û�г�ֵ��ڴ�),���򷵻�0(�޴���) */
#define   ERR_CODE(x)  (errcode = ((errcode>100) ? (errcode) : (x))) /* �����������ͼ��,�������������100,����Ϊ�����ش��� */

static void SafeFree(void **pp)          /* ��ȫ�ͷ��ڴ溯����safeFree��������ʵ���ͷ��ڴ��free���� */
{                                        /* ������free����ִ�к��ڴ汻�ͷţ�����Ȼ�п��ܰ���ԭֵ��ָ����ָ��ԭ��ַ���ᵼ����;ָ�� */
	if (pp != NULL && *pp != NULL)
	{
		free(*pp);
		*pp = NULL;
	}
}
#define SafeFree(p) SafeFree((void**)&(p))

/*-----------------------------
		ȫ��ö�ٱ�������
-----------------------------*/

/* ������Ϲܵ�״̬ */
enum Pipe_Status 
{
	_Isolate=1,	//���ܸ���
	_Replace,	//�����滻
	_Repair,	//©��ܵ��޸�
	_Reopen,	//���ſ���
};


/*-----------------------------
		ȫ�����ݽṹ�嶨��
-----------------------------*/

/* ������Ϲܵ��ṹ�� */
struct FailurePipe
{
	char pipeID[MAX_ID + 1];	//���Ϲܵ�ID
	int pipeindex;				//���Ϲܵ�����(��1��ʼ)
};
typedef struct FailurePipe SFailurePipe;

/* ���屬�ܽṹ�� */
struct Breaks
{
	int isolate_flag;		//���ܸ����ʶ, 0:δ����; 1:����
	int replace_flag;		//�����滻��ʶ, 0:δ�滻; 1:�滻
	int reopen_flag;		//���ſ�����ʶ, 0:δ����; 1:����
	int isolate_time;		//���뱬������Ҫ��ʱ��(minutes)
	int replace_time;		//�����޸�ʱ��(hours)
	int num_isovalve;		//���뱬������Ҫ�رյĹܵ�����
	char nodeID[MAX_ID + 1];//ģ�ⱬ������ӵ�����ڵ�ID(������ϵ����Ϊ0���Թرձ�������)
	char pipeID[MAX_ID + 1];//���ܹܵ�ID(���ܵ�״̬����Ϊopen���Իָ���ˮ)
	float pipediameter;		//���ܹܵ�ֱ��(mm)
	SFailurePipe *pipes;	//���뱬������Ҫ�رյĹܵ�ID
};
typedef struct Breaks SBreaks;

/* ����©��ܵ��ṹ�� */
struct Leaks
{
	int repair_flag;		//©���޸���ʶ, 0:δ�޸�; 1:�޸�
	int reopen_flag;		//���ſ�����ʶ, 0:δ����; 1:����
	int repair_time;		//©���޸�ʱ��(hours)
	char nodeID[MAX_ID + 1];//ģ��©������ӵ�����ڵ�ID(������ϵ����Ϊ0���Թر�©������)
	char pipeID[MAX_ID + 1];//©��ܵ�ID(���ܵ�״̬����Ϊopen���Իָ���ˮ)
	float pipediameter;		//©��ܵ�ֱ��(mm)
};
typedef struct Leaks SLeaks;

/* ������߱����ṹ�� */
struct Decision_Variable
{
	int index;	//�ܵ���������,��0��ʼ
	int type;	//�ܵ�����, 1:���ܸ���; 2:�����滻; 3:©���޸�; 4:���� 
	struct Decision_Variable *next;	//ָ����һ���ڽ�����ṹ��
};
typedef struct Decision_Variable* PDecision_Variable;

/* ����LinkedList����ָ��ṹ�� */
typedef struct _linkedlist
{
	PDecision_Variable head;	/* ָ��ͷ�ڵ�ָ�� */
	PDecision_Variable tail;	/* ָ��β�ڵ�ָ�� */
	PDecision_Variable current;	/* ��ǰָ�룬���ڸ����������� */
}LinkedList;

/* ���幤�̶ӽṹ�� */
struct Crew
{
	long cumulative_time;
	LinkedList Plan;
};
typedef struct Crew SCrew;

#endif
