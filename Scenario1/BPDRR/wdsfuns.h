
//���ļ�������BPDRR������ʹ�õĺ���

#ifndef WDSfUNS_H
#define WDSfUNS_H


int  readdata(char*, char*);		  /* Opens data.txt file & reads parameter data */
int GetDemand(char*);	/* ��ȡ����ʱ�䲽��(Сʱ)��ˮ���ڵ�ʵ����ˮ�� */
void Emptymemory();					  /* Free global variable dynamic memory */
int  Add_tail(LinkedList*, int, int, long, long); /* Add a Decision_Variable struct to the tail of the list */
void Add_SerCapcity_list(Sercaplist*, Sercapacity*); /* Add a Sercapacity struct to the tail of the list */
void Get_FailPipe_keyfacility_Attribute(); /* ��ȡ����ܵ��͹ؼ���������(ҽԺ������˨)�������ֵ */
void Open_inp_file(char*, char*, char*);	/* ��.inp�ļ� */
LinkedList* Randperm();	/* ������ɿɼ�����/©�����˳�򣬹����̶Ӵ���ѡȡ */
int Get_Select_Repository(); /* ���ɿɼ�����/©�������Ϣ�������ѡ�������� */
int Task_Assignment(LinkedList*, STaskassigmentlist*); /* ��SerialSchedule�����е�����ָ�������ÿ�����̶� */
int Breaks_Adjacent_operation(int, int,int, float, float); /* �رջ������ܸ����ܵ����Ա��ܽ��и����ԭ */
int Leaks_operation(int, int, float, float); /* �޸�©��ܵ�����©��ܵ����и�ԭ */


#endif
