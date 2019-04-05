#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#include "MPI_p2p.h"

#define array_size 1073741824

//#define BGQ 1 // when running BG/Q, comment out when running on mastiff
#ifdef BGQ
#include<hwi/include/bqc/A2_inlines.h>
#else
#define GetTimeBase MPI_Wtime
#endif

long long* rank_array = NULL;
long long rank_array_size = 0;
long long result = 0;
long long sum_self = 0;

int my_mpi_size = -1;
int my_mpi_rank = -1;
MPI_Request mpi_request;
MPI_Status mpi_status;

void Init(void);
void DeInit(void);
void Suming(void);

int main(int argc, char** argv){
	
	//Init MPI things
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

	Init();

	double time_in_secs = 0;
	double processor_frequency = 1600000000.0;
	unsigned long long start_cycles = 0;
	unsigned long long end_cycles = 0;

	if(my_mpi_rank == 0) {
		start_cycles = GetTimeBase();

		MPI_P2P_Reduce(rank_array, &result, rank_array_size, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	
		end_cycles = GetTimeBase();

		time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;

	
		printf("%lld\n", result);
		result = 0;

		MPI_Barrier(MPI_COMM_WORLD);

		start_cycles = GetTimeBase();

		Suming();

		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Reduce(&sum_self, &result, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	
		printf("%lld\n", result);
		
		end_cycles = GetTimeBase();

		time_in_secs = ((double)(end_cycles - start_cycles)) / processor_frequency;

	
	}
	else{
		MPI_P2P_Reduce(rank_array, NULL, rank_array_size, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);

		Suming();

		MPI_Barrier(MPI_COMM_WORLD);
		
		MPI_Reduce(&sum_self, &result, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	
	DeInit();

	return EXIT_SUCCESS;
}

void Init(void){

	rank_array_size = array_size / my_mpi_size;
	rank_array = calloc(rank_array_size, sizeof(long long));

	for(int i = 0; i < rank_array_size; i++){
		rank_array[i] = my_mpi_rank * rank_array_size + i;
	}
}

void Suming(void){
	for(int i = 0; i < rank_array_size; i++){
		sum_self += rank_array[i];
	}
}

void DeInit(void){
	free(rank_array);
}
