#include "scheduling_simulator.h"
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<ucontext.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>

struct node {
    int PID;
    char TaskName[50];
    char TaskState[50];
    int QueueingTime;
    char Priority;
    char TimeQuantum;
    ucontext_t child;

    struct node* next;
};

struct node* H_HEAD=NULL;
struct node* H_TAIL=NULL;
struct node* L_HEAD=NULL;
struct node* L_TAIL=NULL;

struct node* s_ptr=NULL;
struct node* t_ptr=NULL;

int H_QNUM=0;
int L_QNUM=0;
int i=0; //for loop
ucontext_t simulate,child,shell;
int F=0;
int add_pid=1;
struct itimerval t;
int state_con=0;

void my_Alarm_Handler();
void Push(int H_or_L,int PID,char TaskName[],int TaskName_len,char TaskState[],int TaskState_len,int QueueingTime,char Priority,char TimeQuantum,ucontext_t child1);
struct node* Pop(int H_or_L);
void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);
void Ctrl_Z_Handler(int signal_number);
void str_clear(char a[],int len);
void simulating();
void shell_mode();
void t1();
void t2();
void t3();
void t4();
void t5();
void t6();

int main()
{
    shell_mode();
    return 0;
}

void my_Alarm_Handler()
{
	//printf("Alarm!\n");
	t.it_interval.tv_sec = 0;
        t.it_interval.tv_usec = 0;
        // Set timer initial value
        t.it_value.tv_sec = 0;
        t.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &t, NULL);//3210 543210 543210 543210..

	swapcontext(&(s_ptr->child),&simulate);
}

void Push(int H_or_L,int PID,char TaskName[],int TaskName_len,char TaskState[],int TaskState_len,int QueueingTime,char Priority,char TimeQuantum,ucontext_t child1)
{
	if(H_or_L==1) { //implement High priority queue

		if(H_HEAD == NULL) {
			H_HEAD = (struct node*)malloc(sizeof(struct node));

			H_HEAD->PID=PID;
			for(i=0; i<TaskName_len; i++) {
				H_HEAD->TaskName[i]=TaskName[i];
			}
			for(i=0; i<TaskState_len; i++) {
				H_HEAD->TaskState[i]=TaskState[i];
			}
			H_HEAD->TaskName[TaskName_len]='\0';
			H_HEAD->TaskState[TaskState_len]='\0';
			H_HEAD->QueueingTime=QueueingTime;
			H_HEAD->Priority=Priority;
			H_HEAD->TimeQuantum=TimeQuantum;
			H_HEAD->child=child1;

			H_HEAD->next = NULL;
			H_TAIL = H_HEAD;
		} else {
			struct node* ptr = (struct node*)malloc(sizeof(struct node));
			ptr->PID=PID;
			for(i=0; i<TaskName_len; i++) {
				ptr->TaskName[i]=TaskName[i];
			}
			for(i=0; i<TaskState_len; i++) {
				ptr->TaskState[i]=TaskState[i];
			}
			ptr->TaskName[TaskName_len]='\0';
			ptr->TaskState[TaskState_len]='\0';
			ptr->QueueingTime=QueueingTime;
			ptr->Priority=Priority;
			ptr->TimeQuantum=TimeQuantum;
			ptr->child=child1;

			ptr->next = NULL;
			H_TAIL->next = ptr;
			H_TAIL = ptr;
		}
		H_QNUM++;
	} else if(H_or_L==0) {
		if(L_HEAD == NULL) {
			L_HEAD = (struct node*)malloc(sizeof(struct node));

			L_HEAD->PID=PID;
			for(i=0; i<TaskName_len; i++) {
				L_HEAD->TaskName[i]=TaskName[i];
			}
			for(i=0; i<TaskState_len; i++) {
				L_HEAD->TaskState[i]=TaskState[i];
			}
			L_HEAD->TaskName[TaskName_len]='\0';
			L_HEAD->TaskState[TaskState_len]='\0';
			L_HEAD->QueueingTime=QueueingTime;
			L_HEAD->Priority=Priority;
			L_HEAD->TimeQuantum=TimeQuantum;
			L_HEAD->child=child1;

			L_HEAD->next = NULL;
			L_TAIL = L_HEAD;
		} else {
			struct node* ptr = (struct node*)malloc(sizeof(struct node));
			ptr->PID=PID;
			for(i=0; i<TaskName_len; i++) {
				ptr->TaskName[i]=TaskName[i];
			}
			for(i=0; i<TaskState_len; i++) {
				ptr->TaskState[i]=TaskState[i];
			}
			ptr->TaskName[TaskName_len]='\0';
			ptr->TaskState[TaskState_len]='\0';
			ptr->QueueingTime=QueueingTime;
			ptr->Priority=Priority;
			ptr->TimeQuantum=TimeQuantum;
			ptr->child=child1;

			ptr->next = NULL;
			L_TAIL->next = ptr;
			L_TAIL = ptr;
		}
		L_QNUM++;

	}
}

struct node* Pop(int H_or_L)
{
	if(H_or_L==1) {
		struct node* ptr = H_HEAD;
		//for(i=0;i<50;i++)
		//{
		//      item[i]=ptr->data[i];
		//}
		H_HEAD = ptr->next;
		//free(ptr);
		H_QNUM--;
		return ptr;
	} else if(H_or_L==0) {
		struct node* ptr = L_HEAD;
		//for(i=0;i<50;i++)
		//{
		//      item[i]=ptr->data[i];
		//}
		L_HEAD = ptr->next;
		//free(ptr);
		L_QNUM--;
		return ptr;
	}
}
void hw_suspend(int msec_10)
{
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_WAITING");

	//usleep(10*msec_10*1000);
	clock_t s,f;
	s=clock();
	while(1)
	{
		f=clock();
		if(((double)(f-s))/(CLOCKS_PER_SEC) > 10*msec_10/1000)
		{
			break;
		}
		if(strcmp(s_ptr->TaskState,"TASK_RUNNING")==0)
		{
			break;
		}
	}
	str_clear(s_ptr->TaskState,50);
        strcat(s_ptr->TaskState,"TASK_READY");
	return;
}

void hw_wakeup_pid(int pid)
{
	struct node* n;
	if(H_QNUM!=0)
	{
		n=H_HEAD;
		for(;n!=NULL;n=n->next)
		{
			if(n->PID==pid)
			{
				str_clear(n->TaskState,50);
				strcat(n->TaskState,"TASK_READY");
				//free(n);
				return;
			}
		}
	}

	if(L_QNUM!=0)
	{
		n=L_HEAD;
		for(;n!=NULL;n=n->next)
		{
			if(n->PID==pid)
			{
				str_clear(n->TaskState,50);
				strcat(n->TaskState,"TASK_READY");
				//free(n);
				return;
			}
		}
	}
}

int hw_wakeup_taskname(char *task_name)
{
	int cnt=0;
	struct node* n;
	if(H_QNUM!=0)
	{
		n=H_HEAD;
		for(;n!=NULL;n=n->next)
		{
			if(strcmp(task_name,n->TaskName)==0&&strcmp("TASK_WAITING",n->TaskState)==0)
			{
				cnt++;
				str_clear(n->TaskState,50);
				strcat(n->TaskState,"TASK_READY");
			}
		}
	}
	if(L_QNUM!=0)
	{
		n=L_HEAD;
		for(;n!=NULL;n=n->next)
		{
			if(strcmp(task_name,n->TaskName)==0&&strcmp("TASK_WAITING",n->TaskState)==0)
			{
				cnt++;
				str_clear(n->TaskState,50);
				strcat(n->TaskState,"TASK_READY");
			}
		}
	}
	//free(n);
	return cnt;
}

int hw_task_create(char *task_name)
{
	if(strcmp(task_name,"task1")==0||strcmp(task_name,"task2")==0||strcmp(task_name,"task3")==0||strcmp(task_name,"task4")==0||strcmp(task_name,"task5")==0||strcmp(task_name,"task6")==0)
	{
		getcontext(&child); //獲取現在的context
		child.uc_stack.ss_sp=malloc(SIGSTKSZ); //指定stack
		child.uc_stack.ss_size=SIGSTKSZ; //指定stack大小
		child.uc_stack.ss_flags=0;
		child.uc_link=&simulate; //執行完後回到simulating
		if(strcmp(task_name,"task1")==0) {
			makecontext(&child,(void (*)(void))t1,0);
		} else if(strcmp(task_name,"task2")==0) {
			makecontext(&child,(void (*)(void))t2,0);
		} else if(strcmp(task_name,"task3")==0) {
			makecontext(&child,(void (*)(void))t3,0);
		} else if(strcmp(task_name,"task4")==0) {
			makecontext(&child,(void (*)(void))t4,0);
		} else if(strcmp(task_name,"task5")==0) {
			makecontext(&child,(void (*)(void))t5,0);
		} else {
			makecontext(&child,(void (*)(void))t6,0);
		}

		Push(0,add_pid,task_name,20,"TASK_READY",20,0,'L','S',child);
		return add_pid++;
	}
	else
	{
		return -1; // the pid of created task name
	}
}

void Ctrl_Z_Handler(int signal_number)
{
	t.it_interval.tv_sec = 0;
	t.it_interval.tv_usec = 0;
	// Set timer initial value
	t.it_value.tv_sec = 0;
	t.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &t, NULL);//3210 543210 543210 543210..

	if(s_ptr==NULL)
	{
		swapcontext(&simulate,&shell);
	}
	else
	{
		swapcontext(&(s_ptr->child),&shell);
	}
}

void str_clear(char a[],int len)
{
	for(i=0; i<len; i++) {
		a[i]='\0';
	}
}

void simulating()
{
	signal(SIGTSTP,Ctrl_Z_Handler);
	printf("In Simulation mode\n");

	if(H_QNUM==0&&L_QNUM==0) 
	{
		shell_mode();
	} 
	else 
	{
		if(H_QNUM!=0) 
		{
S:
			//printf("H_QUEUE is not empty\n");
			s_ptr=H_HEAD;
			for(; s_ptr!=NULL; s_ptr=s_ptr->next) 
			{
				state_con=0;
				if(strcmp(s_ptr->TaskState,"TASK_READY")==0||strcmp(s_ptr->TaskState,"TASK_WAITING")==0)
				{
					if(strcmp(s_ptr->TaskState,"TASK_WAITING")==0)
					{
						state_con=1;	
					}
					// Set interval
					t.it_interval.tv_sec = 0;
					if(s_ptr->TimeQuantum=='L') {
						t.it_interval.tv_usec = 20000;
					} else {
						t.it_interval.tv_usec = 10000;
					}
					// Set timer initial value
					t.it_value.tv_sec = 0;
					if(s_ptr->TimeQuantum=='L') {
						t.it_value.tv_usec = 20000;
					} else {
						t.it_value.tv_usec = 10000;
					}

					setitimer(ITIMER_REAL, &t, NULL);//3210 543210 543210 543210..

					if(signal(SIGALRM, my_Alarm_Handler)<0) {
						//0的時候印出來
						printf("Failed\n");
					}

					if(state_con==0)
					{
						str_clear(s_ptr->TaskState,50);
						strcat(s_ptr->TaskState,"TASK_RUNNING");
					}
					
					swapcontext(&simulate,&(s_ptr->child));

					if(strcmp(s_ptr->TaskState,"TASK_RUNNING")==0)
                                        {
					
                                               	str_clear(s_ptr->TaskState,50);
						if(state_con==0)
						{
                                                	strcat(s_ptr->TaskState,"TASK_READY");
						}
						else
						{
							strcat(s_ptr->TaskState,"TASK_WAITING");
						}
                                        }

					if(strcmp(s_ptr->TaskState,"TASK_TERMINATED")==0)
					{
						if(s_ptr->TimeQuantum=='L') 
						{
							s_ptr->QueueingTime=s_ptr->QueueingTime+20;
						}
						else
						{
							s_ptr->QueueingTime=s_ptr->QueueingTime+10;
						}

					}

					if(H_HEAD!=NULL)
					{
						t_ptr=H_HEAD;
						for(;t_ptr!=NULL;t_ptr=t_ptr->next)	
						{
							if(strcmp(t_ptr->TaskState,"TASK_TERMINATED")!=0&&state_con==0)
							{
								if(t_ptr->TimeQuantum=='L') 
								{
									t_ptr->QueueingTime=t_ptr->QueueingTime+20;
								}
								else 
								{
									t_ptr->QueueingTime=t_ptr->QueueingTime+10;
								}
							}
						}
					}
					if(L_HEAD!=NULL)
					{
						t_ptr=L_HEAD;
						for(;t_ptr!=NULL;t_ptr=t_ptr->next)
						{
							if(strcmp(t_ptr->TaskState,"TASK_TERMINATED")!=0&&state_con==0)
							{
								if(t_ptr->TimeQuantum=='L') 
								{       
									t_ptr->QueueingTime=t_ptr->QueueingTime+20;
								} 
								else 
								{       
									t_ptr->QueueingTime=t_ptr->QueueingTime+10;
								}
							}
						}
					}
				}
			}
		}
	}

	s_ptr=H_HEAD;
	for(; s_ptr!=NULL; s_ptr=s_ptr->next) {
		if(strcmp(s_ptr->TaskState,"TASK_READY")==0) {
			goto S;
		}
	}

	if(L_QNUM!=0)
	{
		//printf("L_QUEUE is not empty\n");
		s_ptr=L_HEAD;
		for(; s_ptr!=NULL; s_ptr=s_ptr->next) 
		{
			state_con=0;
			if(strcmp(s_ptr->TaskState,"TASK_READY")==0||strcmp(s_ptr->TaskState,"TASK_WAITING")==0) 
			{
				if(strcmp(s_ptr->TaskState,"TASK_WAITING")==0)
				{
					state_con=1;
				}
				// Set interval
				t.it_interval.tv_sec = 0;
				if(s_ptr->TimeQuantum=='L') {
					t.it_interval.tv_usec = 20000;
				} else {
					t.it_interval.tv_usec = 10000;
				}
				// Set timer initial value
				t.it_value.tv_sec = 0;
				if(s_ptr->TimeQuantum=='L') {
					t.it_value.tv_usec = 20000;
				} else {
					t.it_value.tv_usec = 10000;
				}

				setitimer(ITIMER_REAL, &t, NULL);//3210 543210 543210 543210..

				if(signal(SIGALRM, my_Alarm_Handler)<0) {
					//0的時候印出來
					printf("Failed\n");
				}

				if(state_con==0)
				{
					str_clear(s_ptr->TaskState,50);
					strcat(s_ptr->TaskState,"TASK_RUNNING");
				}

				//printf("context switch to %s\n",s_ptr->TaskName);
				swapcontext(&simulate,&(s_ptr->child));

				if(strcmp(s_ptr->TaskState,"TASK_RUNNING")==0)
				{

					str_clear(s_ptr->TaskState,50);
					if(state_con==0)
					{
						strcat(s_ptr->TaskState,"TASK_READY");
					}
					else
					{
						strcat(s_ptr->TaskState,"TASK_WAITING");
					}
				}

				if(strcmp(s_ptr->TaskState,"TASK_TERMINATED")==0)
				{
					if(s_ptr->TimeQuantum=='L')
					{
						s_ptr->QueueingTime=s_ptr->QueueingTime+20;
					}
					else
					{
						s_ptr->QueueingTime=s_ptr->QueueingTime+10;
					}

				}


				if(H_HEAD!=NULL)
				{
					t_ptr=H_HEAD;
					for(;t_ptr!=NULL;t_ptr=t_ptr->next)
					{
						if(strcmp(t_ptr->TaskState,"TASK_TERMINATED")!=0&&state_con==0)
						{
							if(t_ptr->TimeQuantum=='L')
							{
								t_ptr->QueueingTime=t_ptr->QueueingTime+20;
							}
							else
							{
								t_ptr->QueueingTime=t_ptr->QueueingTime+10;
							}

						}
					}
				}
				if(L_HEAD!=NULL)
				{
					t_ptr=L_HEAD;
					for(;t_ptr!=NULL;t_ptr=t_ptr->next)
					{
						if(strcmp(t_ptr->TaskState,"TASK_TERMINATED")!=0&&state_con==0)
						{
							if(t_ptr->TimeQuantum=='L')
							{
								t_ptr->QueueingTime=t_ptr->QueueingTime+20;
							}
							else
							{
								t_ptr->QueueingTime=t_ptr->QueueingTime+10;
							}

						}
					}
				}
			}
		}
	}
	s_ptr=H_HEAD;
	for(; s_ptr!=NULL; s_ptr=s_ptr->next) {
		if(strcmp(s_ptr->TaskState,"TASK_READY")==0) {
			goto S;
		}
	}

	s_ptr=L_HEAD;
	for(; s_ptr!=NULL; s_ptr=s_ptr->next) {
		if(strcmp(s_ptr->TaskState,"TASK_READY")==0) {
			goto S;
		}
	}

	t.it_interval.tv_sec = 0;
	t.it_interval.tv_usec = 0;	
	// Set timer initial value
	t.it_value.tv_sec = 0;
	t.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &t, NULL);//3210 543210 543210 543210..

	shell_mode();

}
void shell_mode()
{
	printf("In Shell mode\n");
	char comm[100]= {'\0'};
	char add[10]= {'\0'};
	char add_TaskName[20]= {'\0'};
	char add_t[10]= {'\0'};
	char add_time[10]= {'\0'};
	char add_p[10]= {'\0'};
	char add_priority[10]= {'\0'};
	int pid=0;

	getcontext(&simulate);
        simulate.uc_stack.ss_sp=malloc(SIGSTKSZ);
        simulate.uc_stack.ss_size=SIGSTKSZ;
        simulate.uc_link=&simulate;
        simulate.uc_stack.ss_flags=0;
        makecontext(&simulate,(void (*)(void))simulating,0);

	getcontext(&shell);
        shell.uc_stack.ss_sp=malloc(SIGSTKSZ);
        shell.uc_stack.ss_size=SIGSTKSZ;
        shell.uc_link=&shell;
        shell.uc_stack.ss_flags=0;
        makecontext(&shell,(void (*)(void))shell_mode,0);

	while(1) {
		str_clear(comm,100);

		scanf(" %[^\n]", &comm);
		if(strcmp(comm,"start")==0) {
			//start simulating
			if(s_ptr!=NULL)
			{
				// Set interval
				t.it_interval.tv_sec = 0;

				if(s_ptr->TimeQuantum=='L') 
				{
					t.it_interval.tv_usec = 20000;
				} 
				else 
				{
					t.it_interval.tv_usec = 10000;
				}
				// Set timer initial value
				t.it_value.tv_sec = 0;
				if(s_ptr->TimeQuantum=='L') 
				{
					t.it_value.tv_usec = 20000;
				} 
				else 
				{
					t.it_value.tv_usec = 10000;
				}
				setitimer(ITIMER_REAL, &t, NULL);//3210 543210 543210 543210..
				swapcontext(&shell,&(s_ptr->child));
			}
			else
			{
				swapcontext(&shell,&simulate);
			}
		} else if(strcmp(comm,"ps")==0) {
			//print all task state and property
			if(H_QNUM!=0) {
				struct node* ptr=H_HEAD;
				for(; ptr!=NULL; ptr=ptr->next) {
					printf("%-4d %-7s %-18s %-8d %-3c %c\n",ptr->PID,ptr->TaskName,ptr->TaskState,ptr->QueueingTime,ptr->Priority,ptr->TimeQuantum);
				}
				free(ptr);
			}
			if(L_QNUM!=0) {
				struct node* ptr=L_HEAD;
				for(; ptr!=NULL; ptr=ptr->next) {
					printf("%-4d %-7s %-18s %-8d %-3c %c\n",ptr->PID,ptr->TaskName,ptr->TaskState,ptr->QueueingTime,ptr->Priority,ptr->TimeQuantum);
				}
				free(ptr);
			}
		} else if(comm[0]=='a'&&comm[1]=='d'&&comm[2]=='d') {
			//add
			str_clear(add,10);
			str_clear(add_TaskName,20);
			str_clear(add_t,10);
			str_clear(add_time,10);
			str_clear(add_p,10);
			str_clear(add_priority,10);

			sscanf(comm,"%s %s %s %s %s %s",add,add_TaskName,add_t,add_time,add_p,add_priority);
			if(strcmp(add_t,"\0")==0&&strcmp(add_time,"\0")==0&&strcmp(add_p,"\0")==0&&strcmp(add_priority,"\0")==0) {
				//add Task4
				strcat(add_time,"S");
				strcat(add_priority,"L");
			} else if(strcmp(add_t,"-t")==0&&strcmp(add_p,"\0")==0&&strcmp(add_priority,"\0")==0) {
				//add Task3 -t S
				strcat(add_priority,"L");
			} else if(strcmp(add_t,"-p")==0&&strcmp(add_p,"\0")==0&&strcmp(add_priority,"\0")==0) {
				//add Task1 -p H
				strcat(add_priority,add_time);
				str_clear(add_time,10);
				strcat(add_time,"S");
			}

			getcontext(&child); //獲取現在的context
			child.uc_stack.ss_sp=malloc(SIGSTKSZ); //指定stack
			child.uc_stack.ss_size=SIGSTKSZ; //指定stack大小
			child.uc_stack.ss_flags=0;
			child.uc_link=&simulate; //執行完後回到simulating
			if(strcmp(add_TaskName,"task1")==0) {
				makecontext(&child,(void (*)(void))t1,0);
			} else if(strcmp(add_TaskName,"task2")==0) {
				makecontext(&child,(void (*)(void))t2,0);
			} else if(strcmp(add_TaskName,"task3")==0) {
				makecontext(&child,(void (*)(void))t3,0);
			} else if(strcmp(add_TaskName,"task4")==0) {
				makecontext(&child,(void (*)(void))t4,0);
			} else if(strcmp(add_TaskName,"task5")==0) {
				makecontext(&child,(void (*)(void))t5,0);
			} else {
				makecontext(&child,(void (*)(void))t6,0);
			}

			if(add_priority[0]=='H') {
				Push(1,add_pid++,add_TaskName,20,"TASK_READY",20,0,add_priority[0],add_time[0],child);
			} else if(add_priority[0]=='L') {
				Push(0,add_pid++,add_TaskName,20,"TASK_READY",20,0,add_priority[0],add_time[0],child);
			}
		} else if(comm[0]=='r'&&comm[1]=='e'&&comm[2]=='m'&&comm[3]=='o'&&comm[4]=='v'&&comm[5]=='e') {
			//remove
			int con=0;
			str_clear(add,10);
			pid=0;
			sscanf(comm,"%s %d",add,&pid);
			if(H_QNUM!=0) {
				struct node* ptr2 = (struct node*)malloc(sizeof(struct node));
				ptr2=H_HEAD;
				struct node *pre2;
				if(H_HEAD->PID==pid) {
					H_HEAD=H_HEAD->next;
					free(ptr2);
					con=1;
					H_QNUM--;
				}
				while((ptr2!=NULL)&&(ptr2->PID!=pid)) {
					pre2=ptr2;
					ptr2=ptr2->next;
				}
				if(ptr2!=NULL) {
					pre2->next=ptr2->next;
					free(ptr2);
					con=1;
					H_QNUM--;
				}
			}
			if(L_QNUM!=0&&con==0) {
				struct node* ptr1 = (struct node*)malloc(sizeof(struct node));
				ptr1=L_HEAD;
				struct node *pre;
				if(L_HEAD->PID==pid) {
					L_HEAD=L_HEAD->next;
					free(ptr1);
					L_QNUM--;
				}
				while((ptr1!=NULL)&&(ptr1->PID!=pid)) {
					pre=ptr1;
					ptr1=ptr1->next;
				}
				if(ptr1!=NULL) {
					pre->next=ptr1->next;
					free(ptr1);
					L_QNUM--;
				}
			}
		} else {
			printf("Not a legal commamd\n");
		}
	}
}
void t1()
{
	task1();
	F=1;
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_TERMINATED");
	swapcontext(&child,&simulate);
}
void t2()
{
	task2();
	F=1;
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_TERMINATED");
	swapcontext(&child,&simulate);
}
void t3()
{
	task3();
	F=1;
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_TERMINATED");
	swapcontext(&child,&simulate);
}
void t4()
{
	task4();
	F=1;
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_TERMINATED");
	swapcontext(&child,&simulate);
}
void t5()
{
	task5();
	F=1;
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_TERMINATED");
	swapcontext(&child,&simulate);
}
void t6()
{
	task6();
	F=1;
	str_clear(s_ptr->TaskState,50);
	strcat(s_ptr->TaskState,"TASK_TERMINATED");
	swapcontext(&child,&simulate);
}
