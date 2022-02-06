#!/bin/bash
# Selecting MPI processes and openMP threads
#PBS -l select=4:ncpus=8:mpiprocs=4:ompthreads=16:mem=1gb
# Set max execution time
#PBS -l walltime=0:03:00
# Execution queue
#PBS -q short_cpuQ
# Redirecting stdio and err output
#PBS -o ./stdio.txt
#PBS -e ./stderr.txt
module load mpich-3.2
# Compiling
mpicc -g -Wall -fopenmp -o ./huffman-final/main ./huffman-final/frequencies_utils.c ./huffman-final/main.c ./huffman-final/tree_utils.c -lm
# Change to the PBS working directory where qsub was started from.
cd ${PBS_O_WORKDIR}

# Run
mpirun.actual ./main 16

