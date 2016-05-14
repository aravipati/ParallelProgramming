//ANANYA RAVIPATI

//Command used to compile - mpicc mpi.c -o mpi
//Command used to run - mpirun -np <# of nodes> ./mpi <inputfile name>

/*To run the program on one of the queues available on dmc cluster of alabama supercomputer, follow the steps below
1> Create a makefile with the content below given.

dmc : mpi-dmc

asv1-dmc : asv1.c
	mpiCC -o mpi-dmc mpi.c

clean :
	rm -f *.o mpi-*

2> Create a script file (mpi.sh) with the content below given.

module load openmpi

mpiexec mpi-dmc <inputfile name> (eg: data100.txt)

3> To compile, run the following commands.
> make dmc
> chmod +x mpi.sh
> run_script_mpi mpi.sh
> Select the queue and answer the prompts.
*/

/*References used: 
1> arraysum.c file provided on canvas 
2> http://web.stanford.edu/class/cs238/handouts/scatter.c
3> http://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf
*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;
int Numprocs, MyRank;
int mpi_err; 
#define Root = 0
void init_it(int  *argc, char ***argv);

/*MPI Initialization*/
void init_it(int  *argc, char ***argv)
{
	mpi_err = MPI_Init(argc, argv);
	mpi_err = MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
	mpi_err = MPI_Comm_size(MPI_COMM_WORLD, &Numprocs);

}

int main(int argc, char** argv)
{

/*Variables Initialization*/
	int        index; 
	double     *InputBuffer, *RecvBuffer,  sum=0.0, psum = 0.0;
	double     ptime = 0.0, Totaltime= 0.0,startwtime = 0.0, endwtime = 0.0;
	int        S_DataSize;
	int        DataSize, y;
	int 	   *sendcount, *displs;
	FILE       *fp;

	init_it(&argc,&argv);
	if (argc != 2)
	{
		fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n"); //checking if the input array file is given 
		exit(1);
	}


	//Scatter_SBuff[ ai ] = 0;
	//displs[ ai ] = 0;

	if(MyRank == 0)
	{
		startwtime = MPI_Wtime();
		printf("Number of nodes running %d\n",Numprocs);
		// Read input - calling readArray function
		readArray(argv[1], &InputBuffer, &DataSize);
		printf("Size of array %d\n", DataSize);
	}

	/*passing the value of datasize to all processor nodes*/
	if(MyRank!=0)
	{
		MPI_Recv(&DataSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, NULL);
	}
	else
	{
		int i;

		for(i=1;i<Numprocs;i++)
		{
			MPI_Send(&DataSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
		}
	}

	int i;
/* To Scatter data among processor nodes, I used MPI_Scatterv function. Followig is the syntax of MPI_Scatterv!

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
     			  MPI_Datatype sendtype, void *recvbuf, int recvcount,
	              MPI_Datatype recvtype,
	              int root, MPI_Comm comm)*/

	//allocating memory for *sendcount, *displs for using in MPI_Scatterv// 
	sendcount = (int *)malloc(sizeof(int)*Numprocs);
	displs = (int *)malloc(sizeof(int)*Numprocs);

	//Assigning values to displs and sendcount
	
	for(i=0;i<Numprocs;i++)
	{
		displs[i] = (DataSize / Numprocs) * i;

		if(i==Numprocs-1)
		{
			sendcount[i] = (DataSize / Numprocs) + (DataSize % Numprocs);
		}
		else
		{
			sendcount[i] = (DataSize / Numprocs);
		}
	}
	

	//printf("MyRank: %d Sendcount[%d]: %d\n", MyRank, MyRank, sendcount[MyRank]);

	// Receive count value of MPI_Scatterv can only take integer values, 
	//so assigning the value of sendcount to an integer.
	int count = sendcount[MyRank];
	RecvBuffer= (double *)malloc(count* sizeof(double));

	//printf("MyRank: %d S_DataSize: %d Displacement: %d Count: %d\n",
	//		MyRank, sendcount[MyRank], displs[MyRank], count);

	//MPI_Barrier(MPI_COMM_WORLD);

	mpi_err = MPI_Scatterv(InputBuffer, sendcount , displs, MPI_DOUBLE, RecvBuffer, count , MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Calling sumArray function

	if(MyRank < Numprocs)
	{
	
	psum = sumArray(RecvBuffer, count);
	//printf("Process %d computed sum %f\n", MyRank, psum);
	
	}
	//MPI_Barrier(MPI_COMM_WORLD);

	//To sum all the partial sums from running processor nodes, MPI_Reduce function is used in this program.

	mpi_err = MPI_Reduce(&psum,&sum,1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if(MyRank == 0)
	{
		endwtime = MPI_Wtime();
        Totaltime = endwtime - startwtime;
		printf("Total sum %f\n",sum);
		printf("Total time %f\n", Totaltime);
	}

	MPI_Finalize();
	return 0;
}


void readArray(char * fileName, double ** a, int * n)
{
	int count, DataSize;
	double * InputBuffer;


	FILE * fin;

	fin = fopen(fileName, "r");
	if (fin == NULL) {
		fprintf(stderr, "\n*** Unable to open input file '%s'\n\n",
				fileName);
		exit(1);
	}

	fscanf(fin, "%d\n", &DataSize);
	InputBuffer = (double *)malloc(DataSize * sizeof(double));
	if (InputBuffer == NULL)
	{
		fprintf(stderr, "\n*** Unable to allocate %d-length array",DataSize);
		exit(1);
	}

	for (count = 0; count < DataSize; count++)
	{
		fscanf(fin, "%lf", &InputBuffer[count]);
	}
	fclose(fin);

	*n = DataSize;
	*a = InputBuffer;

}

double sumArray(double *RecvBuffer, int DataSize)
	{
		int index;
		double psum = 0.0;
		for(index = 0; index < DataSize; index++)
		{
			psum = psum + RecvBuffer[index];
			
		}

		return psum;
	}

