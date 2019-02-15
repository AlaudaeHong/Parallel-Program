CLA: CLA.c
	mpicc  CLA.c -o CLA -Wall -Werror

CLA1: CLA_1.c
	gcc CLA_1.c -o CLA1 -Wall -Werror

CLAB: CLA.c
	mpicc CLA.c -o CLAB -Wall -Werror -D Barrier

all: CLA CLA1 CLAB
