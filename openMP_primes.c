/* ANANYA RAVIPATI
     
     openMP_primes.c
     To compile code -> gcc -fopenmp openMP_primes.c -o ompprimes
     To run ./ompprimes <num> <nthreads>
     Reference used- class material & https://computing.llnl.gov/tutorials/openMP/ 
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int x, y;
    int total = 0; //monitor how many prime numbers are discovered
    int nthreads;
    int N;
    double c1,c2;

	//start time
    c1 = time(NULL);

    if(argc != 3)
  {
    printf("Usage is <pprimes num nthreads>\n");
    printf("where num is range of primes you would like to calculate \n ");
    printf("nthreads is number of threads you would like to run\n");
  }

  N = atoi(argv[1]);
  if(N <= 2)
 
  {
    printf("num should be greater than 2\n");
    exit(0);
  }
if(argv[2]==NULL)
    nthreads = 2;
else
    nthreads = atoi(argv[2]);

    #pragma omp parallel for shared(N) private(x,y) reduction(+:total) num_threads(nthreads)
    
		//prime number check
        for ( x = 2; x < N+1; x++ ){
            for ( y = 2; y <= x/2; y++ )
            {
                if ( ! ( x % y ) ) break;
            }
            if ( y > x / 2 )
            {
                total++;
                printf("%d\t", x); //print out prime number
            }
        
        }
    printf("\n");
    printf("Total primes:%d\n",total );

	//end time, calculate running time from this
    c2 = time(NULL);
    //duration = (double)(c2-c1)/CLOCKS_PER_SEC;
    printf("Time to run: %f\n", difftime(c2, c1));

    return 0;
}
            