#!/bin/sh

pwd="/gpfs/u/home/PCP8/PCP8hnhh/scratch/hw45"

#------------------------------------test 4-----------------------------------
#For this part, record the time of Parallel and store the universes generated.
ticks=128
Nnode=128
total=$(($Nnode * 64))

nthread=4
nrank=$(($total / $nthread))

threshold=0
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold
mv R${nrank}T${nthread}.txt 0.txt

threshold="0.25"
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold
mv R${nrank}T${nthread}.txt 1.txt

threshold="0.50"
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold
mv R${nrank}T${nthread}.txt 2.txt

threshold="0.75"
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold
mv R${nrank}T${nthread}.txt 3.txt
#----------------------------------end of test--------------------------------
