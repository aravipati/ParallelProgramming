/* ANANYA RAVIPATI
	
	 pprimes.c
	 To compile code -> gcc pprimes.c -pthreads -o pprimes
	 To run ./pprimes <num> <nthreads>
	 Reference used- https://computing.llnl.gov/tutorials/pthreads/#Thread
*/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Define globally accessible variables and a mutex */

int *globalvector;
long range;
int nthreads;
#define start 2;

/* track which number we are on */
int cur_number = 0;
pthread_mutex_t plock;

/* struct to hold return values from threads */
struct return_values
{
	int element;
	int nprimes;
	int *primes;
};


void *runthread(void *param); /* the thread */

int main(int argc, char *argv[]) 
{
	int total_primes =0, j, number_printed=0;
  time_t timestart, timeend;
  timestart = time(NULL);

	if(argc != 3)
	{
		printf("Usage is <pprimes num nthreads>\n");
		printf("where num is range of primes you would like to calculate \n ");
		printf("nthreads is number of threads you would like to run\n");
	}

	range = atoi(argv[1]);
  if(range <= 2)
  {
    printf("num should be greater than 2\n");
    exit(0);
  }
	pthread_t Thid[range];

	pthread_mutex_init(&plock, NULL); //lock initialization

	globalvector = malloc(range *sizeof(globalvector[0]));

	if(argv[2]==NULL)
		nthreads = 2;
	else
		nthreads = atoi(argv[2]);

	int x;
	int t = start;
	for(x = 0; x< range; x++)
	{
		globalvector[x]= t++;
	}
	struct return_values * thread_values [nthreads];
	long i;
	#pragma omp parallel for
	for(i=0;i<nthreads;i++)
	{
		//allocating memory to struct variables
		struct return_values * rv = malloc(sizeof(struct return_values));
		rv->primes = (int *)malloc(sizeof(int*)*range);
		rv->element=i;
		rv->nprimes=0;
		thread_values[i] = rv;
		pthread_create(&Thid[i], NULL, runthread, (void *)thread_values[i]); // threads creation
	}


	/* Wait on the other threads */

	for(i=0;i<nthreads;i++) {
		pthread_join(Thid[i], NULL);
	}

	/* print results */
	printf("Primes:\n");
	for(i=0;i<nthreads;i++)
	{
		total_primes+=thread_values[i]->nprimes;

		for(j=0;j<thread_values[i]->nprimes;j++)
		{
			printf("%8d \t", thread_values[i]->primes[j]);
			number_printed++;
			if(number_printed%10==0) printf("\n");
		}
	}
	printf("\n\nTotal Primes: %d\n", total_primes);
  timeend = time(NULL);
  printf("Program used %f seconds.\n", difftime(timeend, timestart)); // execution time

	return 0;

}   
//every thread runs this program
void *runthread(void *param) {

	int i, flag;
	struct return_values * params = (struct return_values *)param;

	int local_int;
	pthread_mutex_lock(&plock); //getting a lock
	if(cur_number < range)
	{
		local_int = globalvector[cur_number++];
		pthread_mutex_unlock(&plock); // releasing lock
	}
	else
	{
		pthread_mutex_unlock(&plock);
		pthread_exit(0);
	}


	while(local_int < range)
	{

		flag = 0;

		for(i=2;i<local_int;i++)
		{
			if( (local_int%i==0) && (i != local_int) ) flag=1;

		}

		if(flag==0)
		{
			params->primes[params->nprimes]=local_int;
			params->nprimes++;
			//printf("Thread %d Prime: %d\n", element, local_int);

		}

		pthread_mutex_lock(&plock);
		if(cur_number < range)
		{
			local_int = globalvector[cur_number++];
			pthread_mutex_unlock(&plock);
		}
		else
		{
			pthread_mutex_unlock(&plock);
			break;
		}

	}

			//printf("Total primes for thread %d: %d\n", params->element, nprimes);
			pthread_exit(0);
}
