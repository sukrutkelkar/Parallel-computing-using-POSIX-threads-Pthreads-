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


#define   MAX_NUMP    32

int               NumProcs;

pthread_mutex_t   SyncLock; /* mutex */
pthread_cond_t    SyncCV;    /* condition variable */
int               SyncCount;

pthread_mutex_t   Threa dLock; /* mutex */
struct timespec   StartTime;
struct timespec   EndTime;  
long long int Count;
long long int Remainder;
long long int Max;
long long int circle_count;

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
void* Pi_Calc(void* tmp)
{
  long int threadId = (long int) tmp;
  int ret;
  int startTime, endTime;
  long long int i;
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
  long long int circle_count_local = 0; //Local count for each thread
  unsigned int seed;
  long double x;
  long double y;
  
  for (i=start; i <= end; i++)
  {
    x = (long double)(rand_r(&seed)); //rand_r() returns a pseudo-random integer in the range [0, RAND_MAX]
	y = (long double)(rand_r(&seed));
	x = x/RAND_MAX;
	y = y/RAND_MAX;
    
	if( (((x-0.5)*(x-0.5)) + ((y-0.5)*(y-0.5))) < 0.25) // point in circle logic
    
    circle_count_local++; 
  }  
  
  pthread_mutex_lock(&ThreadLock); /* Get the thread lock */
  circle_count += circle_count_local;
  pthread_mutex_unlock(&ThreadLock); /* Release the lock */
}


main(int argc, char** argv)
{
  pthread_t*     threads;
  pthread_attr_t attr;
  int            ret;
  long int       threadId;
  long double pi = 0;
  double Pi_result;

  if(argc < 3) {
    fprintf(stderr, "Syntax: %s <max> <numProcesors>\nExiting Program...\n", argv[0]);
    exit(1);
  }

  Max = (unsigned long long int)atol(argv[1]);
  NumProcs = atoi(argv[2]);

  if (NumProcs < 1 || NumProcs > MAX_NUMP) {
    fprintf(stderr,"Number of processors has to be between 1 and %d\nExiting Program...\n",MAX_NUMP);
    exit(1);
  }

  if (Max < 1 || Max > 320000000) {
    fprintf(stderr,"Max has to be between 1 and 320000000\nExiting Program...\n");
    exit(1);
  }

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

  Count = Max / NumProcs;
  Remainder = Max % NumProcs;
  circle_count = 0;

  ret = clock_gettime(CLOCK_REALTIME, &StartTime);
  assert(ret == 0);
                         
  for(threadId=0; threadId < NumProcs; threadId++) {
    /* ************************************************************
     * pthread_create takes 4 parameters
     *  p1: threads(output)
     *  p2: thread attribute
     *  p3: start routine, where new thread begins
     *  p4: arguments to the thread
     * ************************************************************ */
    ret = pthread_create(&threads[threadId], &attr, Pi_Calc, (void*) threadId);
    assert(ret == 0);

  }

  /* Wait for each of the threads to terminate */
  for(threadId=0; threadId < NumProcs; threadId++) {
    ret = pthread_join(threads[threadId], NULL);
    assert(ret == 0);
  }

  /*  EndTime = gethrtime();*/
  ret = clock_gettime(CLOCK_REALTIME, &EndTime);
  assert(ret == 0);

//***************************Calculating Pi********************************************  
  Pi_result = (double)(4.00*((double)circle_count/(double)Max));
  printf("The value of Pi is %.10f \n", Pi_result);
//*************************************************************************************
  
  unsigned long long int runtime = 1000000000 * (EndTime.tv_sec - StartTime.tv_sec) + EndTime.tv_nsec - StartTime.tv_nsec; 
  printf("Time = %lld nanoseconds\t(%d.%09lld sec)\n", runtime, runtime / 1000000000, runtime % 1000000000);

  pthread_mutex_destroy(&ThreadLock);

  pthread_mutex_destroy(&SyncLock);
  pthread_cond_destroy(&SyncCV);
  pthread_attr_destroy(&attr);
}
