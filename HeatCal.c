/* **************************************************************** *
 * Name: Sukrut Kelkar                                 
 * Portland State University
 * Description: Pi Calculation
 * Modifed from a homework exercise designed by Min Xu 
 * At the University of Wisconsin-Madison                           *
 * **************************************************************** */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/errno.h>

#define T 6000
#define cx 0.12
#define cy 0.1
#define   MAX_NUMP    32

int               NumProcs;

pthread_mutex_t   SyncLock; /* mutex */
pthread_cond_t    SyncCV;    /* condition variable */
int               SyncCount;
int e,f;

pthread_mutex_t   ThreadLock; /* mutex */
struct timespec   StartTime;
struct timespec   EndTime;  
long long int Count;
long long int Remainder;
long long int Max;

double HeatMatrix_OG [1002] [1002]; //Local count for each thread
double HeatMatrix_New [1002] [1002]; //Local count for each thread

//***********************Barrier Code********************************************
void Barrier()
{
  int ret;

  pthread_mutex_lock(&SyncLock); /* Get the thread lock */
  SyncCount++;
  if(SyncCount == NumProcs) {
    ret = pthread_cond_broadcast(&SyncCV);
    assert(ret == 0);
  } else {
    ret = pthread_cond_wait(&SyncCV, &SyncLock); 
    assert(ret == 0);
  }
  pthread_mutex_unlock(&SyncLock);
}
//*******************************************************************************

//**********************Pi Calculations******************************************
/* The function which is called once the thread is allocated */
void* heatEquation(void* tmp)
{
  long int threadId = (long int) tmp;
  int ret;
  int startTime, endTime;
  long long int c,d;
  long long int start = (long long int) threadId*Count+1;
  if (threadId <= Remainder)
    start += threadId;
  else
    start += Remainder;
  long long int end = (long long int) start+Count-1;
  if (threadId < Remainder)
    end ++;

/* ********************** Thread Synchronization*********************** */
  Barrier();

/* ********************** Execute Job ********************************* */
  
    //printf("Loop 3 initiated\n\n");
    for (c = start; c <= end ; c++)
    {
        for (d=1; d<= 1000; d++)
        {
            HeatMatrix_New [c][d] = HeatMatrix_OG [c][d] +
                                cx * ((HeatMatrix_OG [c+1][d])+ (HeatMatrix_OG [c-1][d])- (2*(HeatMatrix_OG [c][d]))) +
                                cy * ((HeatMatrix_OG [c][d+1])+ (HeatMatrix_OG [c][d-1])- (2*(HeatMatrix_OG [c][d])));
        }
    }
  
  pthread_mutex_lock(&ThreadLock); /* Get the thread lock */
  
    for (e = start; e <=end ; e++)
    {
        for (f=1; f<=1000; f++)
        {
            HeatMatrix_OG [e][f] = HeatMatrix_New [e][f];
        }
    }
  
  pthread_mutex_unlock(&ThreadLock); /* Release the lock */
}


main(int argc, char** argv)
{
  pthread_t*     threads;
  pthread_attr_t attr;
  int            ret;
  long int       threadId;
  int a,b,i;

  if(argc < 2) {
    fprintf(stderr, "Syntax: %s <numProcesors>\nExiting Program...\n", argv[0]);
    exit(1);
  }

  Max = 1000;//(unsigned long long int)atol(argv[1]);
  NumProcs = atoi(argv[1]);

  if (NumProcs < 1 || NumProcs > MAX_NUMP) {
    fprintf(stderr,"Number of processors has to be between 1 and %d\nExiting Program...\n",MAX_NUMP);
    exit(1);
  }

  if (Max < 1 || Max > 320000000) {
    fprintf(stderr,"Max has to be between 1 and 320000000\nExiting Program...\n");
    exit(1);
  }

  //************************************************************************************
  //printf("Loop 1 initiated\n\n");
    
    for (a=0; a<1002; a++)
    {
        for (b=0; b<1002; b++)
        {
            HeatMatrix_OG [a][b] = 0;
        }  
    }
    
    //printf("Loop 1 done\n\n");
    
    //printf("Loop 2 initiated\n\n");
    
    for (a=200; a<=800; a++)
    {
        for (b=200; b<=800; b++)
        {
            HeatMatrix_OG [a][b] = 500;
        }
    }
    
    //printf("Loop 2 done\n\n");
//*************************************************************************************
  
  Count = Max / NumProcs;
  Remainder = Max % NumProcs;

  ret = clock_gettime(CLOCK_REALTIME, &StartTime);
  assert(ret == 0);

    
  for (i=0; i<T; i++)
  {
  
     /* Initialize array of thread structures */
      threads = (pthread_t *) malloc(sizeof(pthread_t) * NumProcs);
      assert(threads != NULL);

      /* Initialize thread attribute */
      pthread_attr_init(&attr);
      pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

      /* Initialize mutexs */
      ret = pthread_mutex_init(&SyncLock, NULL);
      assert(ret == 0);
      ret = pthread_mutex_init(&ThreadLock, NULL);
      assert(ret == 0);
      
      /* Init condition variable */
      ret = pthread_cond_init(&SyncCV, NULL);
      assert(ret == 0);
      SyncCount = 0;

      for(threadId=0; threadId < NumProcs; threadId++) {
        /* ************************************************************
         * pthread_create takes 4 parameters
         *  p1: threads(output)
         *  p2: thread attribute
         *  p3: start routine, where new thread begins
         *  p4: arguments to the thread
         * ************************************************************ */
        ret = pthread_create(&threads[threadId], &attr, heatEquation, (void*) threadId);
        assert(ret == 0);

      }
    //***************************Printing Every 200th Step*********************************  
    if ((i%200)==0)
        {
            printf ("Time: %d | %lf | %lf | %lf | %lf | %lf | %lf |\n",i , HeatMatrix_OG[1][1], HeatMatrix_OG[150][150], HeatMatrix_OG[400][400], HeatMatrix_OG[500][500], HeatMatrix_OG[750][750], HeatMatrix_OG[900][900]);
        }
    //*************************************************************************************
        
      /* Wait for each of the threads to terminate */
      for(threadId=0; threadId < NumProcs; threadId++) {
        ret = pthread_join(threads[threadId], NULL);
        assert(ret == 0);
      }

      pthread_mutex_destroy(&ThreadLock);

      pthread_mutex_destroy(&SyncLock);
      pthread_cond_destroy(&SyncCV);
      pthread_attr_destroy(&attr);

  }  
  /*  EndTime = gethrtime();*/
  ret = clock_gettime(CLOCK_REALTIME, &EndTime);
  assert(ret == 0);

  unsigned long long int runtime = 1000000000 * (EndTime.tv_sec - StartTime.tv_sec) + EndTime.tv_nsec - StartTime.tv_nsec; 
  printf("Time = %lld nanoseconds\t(%d.%09lld sec)\n", runtime, runtime / 1000000000, runtime % 1000000000);

  
}
