#include <iostream>
#include "cluster/cluster.hpp"
#include "graph/graph.hpp"
#include "graphReduction/bisimilarGraphReducer.hpp"
#include <vector>


using namespace std;

int main(int argc, char *argv[]) {
	ClusterHandler cluster(new Cluster);
	
	BisimilarGraphReducer bgr(cluster,"Undefined");
	//cout << cluster->getNumberOfNodes() << " : " << cluster->getrankOfCurrentNode() << endl;
}