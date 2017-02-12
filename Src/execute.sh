#!/bin/bash
/Users/kingwinds/Desktop/MPIProject/OpenMPI_MT/bin/mpic++ -std=c++14 -o out ./main.cpp ./cluster/cluster.cpp ./graphReduction/GraphDistributor.cpp ./graphReduction/bisimilarGraphReducer.cpp
/Users/kingwinds/Desktop/MPIProject/OpenMPI_MT/bin/mpirun -hostfile ./hosts -n 4 ./out