#ifndef GRAPHDISTRIBUTOR_PROTECT
#define GRAPHDISTRIBUTOR_PROTECT

#include "../cluster/cluster.hpp"
#include <vector>
#include "../graph/graph.hpp"
#include <set>
#include <algorithm>
#include <string>

class GraphDistributor
{
	public : 
		GraphDistributor(ClusterHandler& clusterHandler,std::string path) : cluster(clusterHandler){reduceGraph(path);}
		
	private :
		void reduceGraph(std::string path);
		void readAndDistributeGraph(std::string path);
		void receiveGraphSegment();
		initOut* createInitOutStruct(nodeType source, edgeType edge, blockType destinationBlock, int clusterDestinationNode);
		initIn* createInitInStruct(nodeType dest, int clusterSourceNode);
		
		ClusterHandler cluster;
		Out** outij;
		In** inij;
};

#endif 