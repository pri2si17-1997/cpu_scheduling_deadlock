#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>
#include <semaphore.h>
#include <stdbool.h>
#include <errno.h>

int choice_user;

long int at[5];
long int bt[5];
sem_t agent_ready;
sem_t smoker_semaphors[3];
char* smoker_types[3] = { "matches & tobacco", "matches & paper", "tobacco & paper" };
bool items_on_table[3] = { false, false, false };
sem_t pusher_semaphores[3];
sem_t pusher_lock;

void* smoker(void* arg)
{
	int smoker_id = *(int*) arg;
	int type_id   = smoker_id % 3;
	int i = 0;
	// Smoke 3 times
	for (i = 0; i < 3; ++i)
	{
		printf("\033[0;37mSmoker %d \033[0;31m>>\033[0m Waiting for %s\n",
			smoker_id, smoker_types[type_id]);

		// Wait for the proper combination of items to be on the table
		sem_wait(&smoker_semaphors[type_id]);

		// Make the cigarette before releasing the agent
		printf("\033[0;37mSmoker %d \033[0;32m<<\033[0m Now making the a cigarette\n", smoker_id);
		usleep(rand() % 50000);
		sem_post(&agent_ready);

		// We're smoking now
		printf("\033[0;37mSmoker %d \033[0;37m--\033[0m Now smoking\n", smoker_id);
		usleep(rand() % 50000);
	}

	return NULL;
}

void* pusher(void* arg)
{
	int pusher_id = *(int*) arg;
	int i = 0;
	for (i = 0; i < 12; ++i)
	{
		// Wait for this pusher to be needed
		sem_wait(&pusher_semaphores[pusher_id]);
		sem_wait(&pusher_lock);

		// Check if the other item we need is on the table
		if (items_on_table[(pusher_id + 1) % 3])
		{
			items_on_table[(pusher_id + 1) % 3] = false;
			sem_post(&smoker_semaphors[(pusher_id + 2) % 3]);
		}
		else if (items_on_table[(pusher_id + 2) % 3])
		{
			items_on_table[(pusher_id + 2) % 3] = false;
			sem_post(&smoker_semaphors[(pusher_id + 1) % 3]);
		}
		else
		{
			// The other item's aren't on the table yet
			items_on_table[pusher_id] = true;
		}

		sem_post(&pusher_lock);
	}

	return NULL;
}


void* agent(void* arg)
{
	int agent_id = *(int*) arg;
	int i;

	for (i = 0; i < 6; ++i)
	{
		usleep(rand() % 200000);

		// Wait for a lock on the agent
		sem_wait(&agent_ready);

		// Release the items this agent gives out
		sem_post(&pusher_semaphores[agent_id]);
		sem_post(&pusher_semaphores[(agent_id + 1) % 3]);

		// Say what type of items we just put on the table
		printf("\033[0;35m==> \033[0;33mAgent %d giving out %s\033[0;0m\n",
			agent_id, smoker_types[(agent_id + 2) % 3]);
	}

	return NULL;
}


void deadlock_problem()
{
	int i;
	srand(time(NULL));
	sem_init(&agent_ready, 0, 1);
	sem_init(&pusher_lock, 0, 1);
	for (i = 0; i < 3; ++i)
	{
		sem_init(&smoker_semaphors[i], 0, 0);
		sem_init(&pusher_semaphores[i], 0, 0);
	}
	int smoker_ids[6];
	pthread_t smoker_threads[6];
	for (i = 0; i < 6; ++i)
	{
		smoker_ids[i] = i;

		if (pthread_create(&smoker_threads[i], NULL, smoker, &smoker_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}
	int pusher_ids[6];

	pthread_t pusher_threads[6];

	for (i = 0; i < 3; ++i)
	{
		pusher_ids[i] = i;

		if (pthread_create(&pusher_threads[i], NULL, pusher, &pusher_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}
	int agent_ids[6];

	pthread_t agent_threads[6];

	for (i = 0; i < 3; ++i)
	{
		agent_ids[i] =i;

		if (pthread_create(&agent_threads[i], NULL, agent, &agent_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}
	
	for (i = 0; i < 6; ++i)
	{
		pthread_join(smoker_threads[i], NULL);
	}

	return 0;
}

static long int pclock(char *msg, clockid_t cid)
{
	struct timespec ts;
        printf("%s", msg);
        if (clock_gettime(cid, &ts) == -1)
               printf("Got Error!!");
        //printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec);
        return ts.tv_nsec;
}

void *function_thread(void *arg)
{
	int id=(int)arg;
	clockid_t cid;
	printf("Hello thread [%d]\n",id+1);
	printf("CPU time taken by thread : \n");
        bt[id]=pclock("Process total CPU time: ", CLOCK_THREAD_CPUTIME_ID);
        int s = pthread_getcpuclockid(pthread_self(), &cid);
        //bt[i]=s;
        printf("%03ld\n ",bt[id]);

        if (s != 0)
               //handle_error_en(s, "pthread_getcpuclockid");
        pclock("Main thread CPU time:   ", cid);
	pthread_exit(NULL);
}


void FCFS(int n)
{
    int wt[n],tat[n],avwt=0,avtat=0,i,j,time;

    time=at[0];    //time elapsed is arrival time for first process

    //calculating turnaround time and waiting time
    for(i=0;i<n;i++)
    {
        if(time<at[i])
            time+=at[i];
        time+=bt[i];
        tat[i]=time-at[i];
        wt[i]=tat[i]-bt[i];
    }

    printf("\nProcess\t\t\tArrival Time\t\tBurst Time\t\t  Turnaround Time\t\tWaiting Time");

    //calculating turnaround time
    for(i=0;i<n;i++)
    {
        avwt+=wt[i];
        avtat+=tat[i];
        printf("\nP[%d]  \t\t\t\t%4d\t\t\t\t%4d\t\t\t     %4d\t\t\t\t  %4d",i+1,at[i],bt[i],tat[i],wt[i]);
    }

    printf("\n\nAverage Turnaround Time:  %f ns",(float)avtat/n);
    printf("\nAverage Waiting Time:  %f ns",(float)avwt/n);

}

void SJF(int n)
{
    int p[n],wt[n],tat[n],i,j,total=0,pos,temp;
    float avg_wt,avg_tat;

 	for(i=0;i<n;i++){
 		p[i]=i+1;
 	}
    //sorting burst time in ascending order using selection sort
    for(i=0;i<n;i++)
    {
        pos=i;
        for(j=i+1;j<n;j++)
        {
            if(bt[j]<bt[pos])
                pos=j;
        }

        temp=bt[i];
        bt[i]=bt[pos];
        bt[pos]=temp;

        temp=p[i];
        p[i]=p[pos];
        p[pos]=temp;
    }

    wt[0]=0;            //waiting time for first process will be zero

    //calculate waiting time
    for(i=1;i<n;i++)
    {
        wt[i]=0;
        for(j=0;j<i;j++)
            wt[i]+=bt[j];

        total+=wt[i];
    }

    avg_wt=(float)total/n;      //average waiting time
    total=0;

    printf("\nProcess\t\t\tArrival Time\t\tBurst Time\t\t  Turnaround Time\t\tWaiting Time");
    for(i=0;i<n;i++)
    {
        tat[i]=bt[i]+wt[i];     //calculate turnaround time
        total+=tat[i];
        printf("\nP[%d] \t\t\t\t%4d\t\t\t\t%4d\t\t\t     %4d\t\t\t\t  %4d",p[i],at[i],bt[i],tat[i],wt[i]);
    }

    avg_tat=(float)total/n;     //average turnaround time
    printf("\nAverage Turnaround Time=  %f ns",avg_tat);
    printf("\nAverage Waiting Time=  %f ns",avg_wt);

}


int q[100];  //queue
int front=-1,rear=-1;

void enqueue(int i)
{
	if(rear==100){
		printf("overflow");
		return;
	}
	rear++;
	q[rear]=i;
	if(front==-1)
	front=0;
}

int dequeue()
{
	if(front==-1)
	{
		printf("underflow");
		return -1;
	}
	int temp=q[front];
	if(front==rear)
	front=rear=-1;
	else
	front++;
	return temp;
}

int isInQueue(int i)
{
	int k;
	for(k=front;k<=rear;k++)
	{
		if(q[k]==i)
		return 1;
	}
	return 0;
}


void RR(int n)
{
    int p[n],wt[n],tt[n],rt[n],completed[n],i,j,time=0,sum_bt=0,tq;
    float avgtt=0,avgwt=0;

     for(i=0;i<n;i++)
     {
     	 p[i]=i+1;
		 rt[i]=bt[i];
		 completed[i]=0;
		 sum_bt+=bt[i];
    }

    tq=2000;

    enqueue(0);          // enqueue the first process
    printf("Process execution order: ");
    for(time=at[0];time<sum_bt;)       // run until the total burst time reached
    {
		i=dequeue();
		if(rt[i]<=tq)
		{                 /* for processes having remaining time with less than or  equal  to time quantum  */
			time+=rt[i];
			rt[i]=0;
			completed[i]=1;
			printf(" %d ",p[i]);
					tt[i]=time-at[i];
					wt[i]=tt[i]-bt[i];
					for(j=0;j<n;j++)                /*enqueue the processes which have come                                         while scheduling */
					{
						if(at[j]<=time && completed[j]!=1&& isInQueue(j)!=1)
						{
							enqueue(j);
						}
				    }
		}
		else               // more than time quantum
		{
			time+=tq;
			rt[i]-=tq;
			printf(" %d ",p[i]);
			for(j=0;j<n;j++)    /*first enqueue the processes which have come while                                             scheduling */
			{
				if(at[j]<=time && completed[j]!=1&&i!=j&& isInQueue(j)!=1)
				{
					enqueue(j);
				}
			}
			enqueue(i);   // then enqueue the uncompleted process
		}
    }

    printf("\nProcess\t\t\tArrival Time\t\tBurst Time\t\t  Turnaround Time\t\tWaiting Time");
    for(i=0;i<n;i++)
	{
		avgtt+=tt[i];
		avgwt+=wt[i];
		printf("\nP[%d]  \t\t\t\t%4d\t\t\t\t%4d\t\t\t     %4d\t\t\t\t  %4d",p[i],at[i],bt[i],tt[i],wt[i]);
    }

    printf("\n\nAverage Turnaround Time:  %f ns",avgtt/n);
    printf("\nAverage waiting time:  %f ns\n",avgwt/n);

}


void cpu_schedule_algorithms()
{
	int err, s, i;
	printf("In cpu_schedule_algorithms : \n");
	pthread_t thread[5];
	at[0]=0;
	at[1]=1230;
	at[2]=3520;
	at[3]=6780;
	at[4]=9000;
	clockid_t cid;
	for(i=0;i<5;i++)
	{
	        err = pthread_create(&thread[i], NULL, &function_thread, (void*)i);
	        if(err != 0)
	            printf("Error while creating thread\n");
	        else
	            printf("Successfully created thread [%d].\n",i+1);
	}
	for(i=0;i<5;i++)
	{
	    	pthread_join(thread[i],NULL);
    	}

    	printf("\n\nFCFS CPU Scheduling:- \n");
    	FCFS(5);
    	printf("\n\nSJF CPU Scheduling:- \n");
   	SJF(5);
    	printf("\n\nROUND ROBIN CPU Scheduling:- \n");
    	RR(5);
	printf("\nFinished!\n");
	//pthread_exit(NULL);
}

int main()
{
	bool flag = false;
	while(1)
	{
		printf("Enter Your Choice : \n 1. Analysis Of CPU Scheduling Algoritmhs.\n 2. Deadlock Problem(Smoker-Cigarette)\n 3. Exit\n");
		scanf("%d", &choice_user);
		if(choice_user == 1)
		{
			printf("You have opted for CPU Scheduling Algorithms.\n");
 			cpu_schedule_algorithms();
		}

		else if(choice_user == 2)
		{
			printf("You have opted for Deadlock Problem.\n");
			deadlock_problem();	
		}

		else
		{
			flag = true;
		}

		if(flag)
			break;
	}
	return 0;
}
