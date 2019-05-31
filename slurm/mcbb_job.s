#!/bin/bash
#SBATCH --cpus-per-task 4
#SBATCH -o ./output-scaling-sync/slurm-%j.out # STDOUT
#SBATCH --time 240

module purge
module load openmpi/gnu/2.0.2
module load mosek/8.1.0.64
module load eigen/3.3.1

mpirun $SCRATCH/mcbb-mpi/mcbb -f $SCRATCH/mcbb-mpi/test_data/er__0_5__80__2.csv -m 200 -s
