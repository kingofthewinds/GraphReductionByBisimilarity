#ifndef GRAPHDISTRIBUTOR_PROTECT
#define GRAPHDISTRIBUTOR_PROTECT

#include "../cluster/cluster.hpp"
#include <vector>
#include "../graph/graph.hpp"
#include <set>
#include <algorithm>
#include <string>
#include <set>

class GraphDistributor
{
	public : 
		GraphDistributor(ClusterHandler& clusterHandler,std::string path, 
						std::vector< std::vector<Out*> >& out, 
						std::vector< std::vector<In*>  >& in,
						std::set<nodeType>& attributedNodesToClusterNode) : 
						cluster(clusterHandler), outij(out),inij(in),
						attributedNodes(attributedNodesToClusterNode)		
						{reduceGraph(path);}
		
	private :
		void reduceGraph(std::string path);
		void readAndDistributeGraph(std::string path);
		void receiveGraphSegment();
		initOut* createInitOutStruct(nodeType source, edgeType edge, blockType destinationBlock, int clusterDestinationNode);
		initIn* createInitInStruct(nodeType dest, int clusterSourceNode);
		void addOut(initOut* out);
		void addIn(initIn* in);
		
		ClusterHandler cluster;
		std::vector< std::vector<Out*> >& outij; 
		std::vector< std::vector<In*>  >& inij;
		std::set<nodeType>& attributedNodes;
};

#endif 