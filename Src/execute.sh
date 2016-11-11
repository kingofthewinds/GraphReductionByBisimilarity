#!/bin/bash
/Users/kingwinds/Desktop/MPIProject/OpenMPI/bin/mpic++ -std=c++14 -o out ./main.cpp ./cluster/cluster.cpp ./graphReduction/clusterReduction.cpp
/Users/kingwinds/Desktop/MPIProject/OpenMPI/bin/mpirun -hostfile ./hosts -n 3 ./out
