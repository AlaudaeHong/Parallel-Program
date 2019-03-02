#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#include "MPI_p2p.h"

#define array_size 1073741824

long long* rank_array = NULL;
long long rank_array_size = 0;
long long result = 0;

int my_mpi_size = -1;
int my_mpi_rank = -1;
MPI_Request mpi_request;
MPI_Status mpi_status;

void Init(void);
void DeInit(void);


int main(int argc, char** argv){
	
	//Init MPI things
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

//	printf("aaaaa\n");

	Init();

//	printf("preparing\n");

	if(my_mpi_rank == 0) 
		MPI_P2P_Reduce(rank_array, &result, rank_array_size, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	else
		MPI_P2P_Reduce(rank_array, NULL, rank_array_size, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

	printf("%lld\n", result);

	DeInit();

	return EXIT_SUCCESS;
}

void Init(void){

	rank_array_size = array_size / my_mpi_size;
	rank_array = calloc(rank_array_size, sizeof(long long));

//	printf("array_size: %lld\n", rank_array_size);

//	long long sum = 0;

	for(int i = 0; i < rank_array_size; i++){
		rank_array[i] = my_mpi_rank * rank_array_size + i;
//		sum += rank_array[i];
	}

//	printf("%lld\n", sum);
}

void DeInit(void){
	free(rank_array);
}
