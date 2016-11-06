#ifndef BISIMILAR_PROTECT
#define BISIMILAR_PROTECT

#include "../cluster/cluster.hpp"
#include <vector>
#include "../types.hpp"
#include "../graph/graph.hpp"
#include <set>
#include <algorithm>

class BisimilarReduction
{
	public : 
		BisimilarReduction(ClusterHandler& clusterHandler) : cluster(clusterHandler){reduceGraph();}
		
	private :
		ClusterHandler cluster;
		void reduceGraph();
		void readAndDistributeGraph();
		void receiveGraphSegment();
};

#endif 