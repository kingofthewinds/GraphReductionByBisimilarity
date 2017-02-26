#!/bin/bash
/Users/kingwinds/Desktop/MPIProject/OpenMPI_MT/bin/mpic++ -std=c++14 -o out ./main.cpp ./cluster/cluster.cpp ./graphReduction/graphDistributor.cpp ./graphReduction/bisimilarGraphReducer.cpp ./graphReduction/profiler.cpp
/Users/kingwinds/Desktop/MPIProject/OpenMPI_MT/bin/mpirun -n 1 ./out