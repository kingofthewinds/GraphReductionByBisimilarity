#!/bin/bash
/Users/kingwinds/Desktop/MPIProject/OpenMPI/bin/mpic++ -o out ./main.cpp ./cluster/cluster.cpp ./graph/graph.cpp ./graphReduction/clusterReduction.cpp
/Users/kingwinds/Desktop/MPIProject/OpenMPI/bin/mpirun -hostfile ./hosts -n "$2" ./out