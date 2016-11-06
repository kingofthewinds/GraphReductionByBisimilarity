#include <iostream>
#include "cluster/cluster.hpp"
#include "types.hpp"
#include "graph/graph.hpp"
#include "graphReduction/clusterReduction.hpp"
#include <vector>


using namespace std;

int main(int argc, char *argv[]) {
	ClusterHandler cluster(new Cluster);
	BisimilarReduction br(cluster);
	
	
	
	//cout << cluster->getNumberOfNodes() << " : " << cluster->getrankOfCurrentNode() << endl;
}