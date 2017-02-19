#include <iostream>
#include "cluster/cluster.hpp"
#include "graph/graph.hpp"
#include "graphReduction/bisimilarGraphReducer.hpp"
#include "graphReduction/profiler.hpp"
#include <vector>
#include <iostream>
#include <fstream>


using namespace std;

int main(int argc, char *argv[]) {
	
	Profiler profiler;
	profiler.startProfiling();
	
	//create a  cluster object and wrap a clusterHandler around it : the cluster object should be unique and  
	//the clusterhandler object takes care of that 
	ClusterHandler cluster(new Cluster(profiler));
	
	profiler.setRankAndNumberOfNodes(cluster->getrankOfCurrentNode(), cluster->getNumberOfNodes());
	
	//create a BisimilarGraphReducer and pass to it the clusterhandler object and a string of the address 
	//of the file that contains the graph that needs to be reduced. The algorithm of graph reduction will then 
	//automatically be applied by the constructor of BisimilarGraphRedcuser
	BisimilarGraphReducer bgr(cluster,"tpch.edge.hashed",profiler);
	
	profiler.endProfiling();
	
	string file = "Results/res-"+ std::to_string(cluster->getrankOfCurrentNode());
	ofstream resfile(file.c_str());
	profiler.printResults(resfile);
	cluster->closeCluster();
}