#include "MPI_p2p.h"

//Current ONLY support MPI_LONG_LONG, MPI_SUM, and root rank of 0
//Not support array addition
//NOTICE: COUNT HERE IS DIFFERENT FROM WHAT IS IN MPI_REDUCE
int MPI_P2P_Reduce(const void *sendbuf, void *recvbuf, int count, 
		MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){

	MPI_Request mpi_request;
	MPI_Status mpi_status;

	if(op != MPI_SUM) {
		printf("Funtion current only support MPI_SUM\n");
		return 0;
	}

	long long sum_temp1 = 0;				//Used to store self sum
	long long sum_temp2 = 0;				//Used to reveive sum of pair

	int mpi_size = 0;
	int mpi_rank = 0;

	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	int stride = 0;
	
	if(recvbuf != NULL) *(long long*)recvbuf = 0;

	//Each rank computes sum over local data array
	for(int i = 0; i < count; i++){
		long long * ptr = (long long *)sendbuf;
		sum_temp1 += *(ptr + i);
	}
	stride = 1;

	//Keep collecting until rank 0 has the final sum result
	while(stride != mpi_size){
		
		char if_target = 0;

		if( (mpi_rank % (2 * stride)) ==  0 ){

			int source_rank = mpi_rank + stride;

			if_target = 1;

			MPI_Irecv(&sum_temp2, 1, datatype, source_rank, stride, comm, &mpi_request);
		}

		MPI_Barrier(comm);

		if( (mpi_rank % (2 * stride)) == stride ){
			
			int target_rank = mpi_rank - stride;

			MPI_Isend(&sum_temp1, 1, datatype, target_rank, stride, comm, &mpi_request);
		}

		if( if_target ){ 
			MPI_Wait(&mpi_request, &mpi_status); 
			sum_temp1 += sum_temp2;
		}

		stride *= 2;
	}

	if(recvbuf != NULL) *( (long long*)recvbuf ) = sum_temp1;
	
	return 1;
	
}
