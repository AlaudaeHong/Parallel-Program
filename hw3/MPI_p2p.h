#ifndef MPI_p2p
#define MPI_p2p

#include <mpi.h>
#include <stdio.h>

int MPI_P2P_Reduce(
		const void *sendbuf, 
		void *recvbuf, 
		int count, 
		MPI_Datatype datatype, 
		MPI_Op op, 
		int root, 
		MPI_Comm comm );

#endif
