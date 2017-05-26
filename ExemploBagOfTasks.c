/* matrix summation using pthreads

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_mutex_t checkMaxMin; /* mutex lock for checking maximum and minimum*/ 
pthread_mutex_t bagLock; /* mutex lock for bag of tasks*/ 
pthread_mutex_t sumTotal; /* mutex lock for adding to total*/ 
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/* IGNORE */
/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */
int maxElement, minElement;
int maxRow = 0, maxCol = 0, minRow = 0, minCol = 0;
int total = 0; /* Total sum */
int nextRow = 0;  /* Counter for our bag of task */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* initialize mutex for maximum and minimum and total sum*/
  pthread_mutex_init(&checkMaxMin, NULL);
  pthread_mutex_init(&sumTotal, NULL);
  pthread_mutex_init(&bagLock, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* Seed rng */
  srand(getpid()); 

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;//rand()%99;
      }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
      printf("[ ");
      for (j = 0; j < size; j++) {
        printf(" %d", matrix[i][j]);
      }
      printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  maxElement = matrix[0][0];
  minElement = matrix[0][0];
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  end_time = read_timer();
  /* print results */
  printf("The total is %d\n", total);
  printf("The maximum element is %d at row %d and col %d\n", maxElement, maxRow, maxCol);
  printf("The minimum element is %d at row %d and col %d\n", minElement, minRow, minCol);
  printf("The execution time is %g sec\n", end_time - start_time);
}

/* Each worker will grab a task from the bag and compute the row sum and add it to the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int localTotal,row, j;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif


  while(true){
    /* Get task from bag */
    pthread_mutex_lock(&bagLock);
    row = nextRow;
    nextRow++;
    pthread_mutex_unlock(&bagLock);
    /* If we are already finished with the bag, lets break out of the while loop */
    if(row >= size){
    break;
    }
    for (j = 0; j < size; j++){
      /* Add sum to the local total for the work assigned to the thread */
      localTotal += matrix[row][j];
      /* Critical area, here we check for the maximum and minimum element */
      pthread_mutex_lock(&checkMaxMin);
      if(maxElement < matrix[row][j]){
              maxElement = matrix[row][j];
              maxRow = row;
              maxCol = j;
      }
      if(minElement > matrix[row][j]){
              minElement = matrix[row][j];
              minRow = row;
              minCol = j;
      }
      pthread_mutex_unlock(&checkMaxMin);
    }
  }
  pthread_mutex_lock(&sumTotal);
  total += localTotal;
  pthread_mutex_unlock(&sumTotal);
}