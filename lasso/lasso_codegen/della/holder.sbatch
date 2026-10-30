#!/bin/bash
# A holder for the agents' builds, verifies and benchmarks (through the make
# wrapper): one 80 GB A100 and 16 cores for one hour, which della starts at once in
# the gputest partition (a longer job waits days). It only sleeps; holders.sh keeps
# one running (submitting the next near the end of the current one), and della/make
# sends work to the current one with srun --overlap.
#SBATCH --job-name=fom-holder
#SBATCH --account=bstellato
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --gres=gpu:1
#SBATCH --constraint=gpu80
#SBATCH --time=01:00:00
#SBATCH --output=slurm/logs/%x-%j.out

echo "allocation $SLURM_JOB_ID on $(hostname): $(nvidia-smi -L | head -1)"
sleep infinity
