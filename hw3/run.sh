#!/bin/sh
#/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3


srun --ntasks 64	--overcommit -o 64rank.log		/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 128	--overcommit -o 128rank.log		/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 256	--overcommit -o 256rank.log		/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 512	--overcommit -o 512rank.log		/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 1024	--overcommit -o 1024rank.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 2048	--overcommit -o 2048rank.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 4096	--overcommit -o 4096rank.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
srun --ntasks 8192	--overcommit -o 8192rank.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw3/main.xl
