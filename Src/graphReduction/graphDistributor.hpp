#ifndef GRAPHDISTRIBUTOR_PROTECT
#define GRAPHDISTRIBUTOR_PROTECT

#include "../cluster/cluster.hpp"
#include <vector>
#include "../graph/graph.hpp"
#include <set>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

class GraphDistributor
{
	public : 
		/**
			receives information about the location of the graph on the root node's 
			memory and starts distributing the graph on to the cluster nodes by saving the 
			received messages to the appropptiate data structutres passed in as parameters 
			
			@param clusterhandler the cluster wrapper object 
			@param out/in/attributedNodesToClusterNode the data structures that will be filled
				in during the distribution process 
		*/
		GraphDistributor(ClusterHandler& clusterHandler,std::string path, 
						std::vector< std::vector<Out*> >& out, 
						std::vector< std::vector<In*>  >& in,
						std::map<nodeType,nodeInfo>& attributedNodesToCluster) : 
						cluster(clusterHandler), outij(out),inij(in),
						attributedNodes(attributedNodesToCluster)		
						{readAndDistributeGraph(path);}
		
	private :
		/*
			reads and distributes the graph nodes to cluster nodes 
		*/
		void readAndDistributeGraph(std::string path);

		
		//the cluster wrapper object 
		ClusterHandler cluster;
		
		//where the distributed data will be saved : 
		std::vector< std::vector<Out*> >& outij; 
		std::vector< std::vector<In*>  >& inij;
		std::map<nodeType,nodeInfo>& attributedNodes;
};

#endif 