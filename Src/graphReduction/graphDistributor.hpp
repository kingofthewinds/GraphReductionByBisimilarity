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
						std::set<nodeType>& attributedNodesToClusterNode) : 
						cluster(clusterHandler), outij(out),inij(in),
						attributedNodes(attributedNodesToClusterNode)		
						{reduceGraph(path);}
		
	private :
		/**
			receives the address of a graph file on the disk and calls the readAndDistributeGraph member 
			function 
		*/
		void reduceGraph(std::string path);
		
		/*
			reads and distributes the graph nodes to cluster nodes 
		*/
		void readAndDistributeGraph(std::string path);
		
		/**
			receives IN and OUT messages from the root cluster node and save them to the graph structures 
			of the local cluster node 
		*/
		void receiveGraphSegment();
		
		/**
			instantiates (on the heap) and returns an InitOut Structure with data passed in as parameters 
		*/
		initOut* createInitOutStruct(nodeType source, edgeType edge, 
				blockType destinationBlock, 
				int clusterDestinationNode);
		/**
			instantiates (on the heap) an returns an InitIn Structure with data passed in as parameters 
		*/
		initIn* createInitInStruct(nodeType dest, int clusterSourceNode);
		
		/**
			converts the initOut structure to and Out structure and saves it to the 
			underlying node of the cluster. It then deallocates the initOut strcture 
			passed in as parameters 
		*/
		void addOut(initOut* out);
		
		/**
			Does the same as addOut but with an initIn structure 
		*/
		void addIn(initIn* in);
		
		//the cluster wrapper object 
		ClusterHandler cluster;
		
		//where the distributed data will be saved : 
		std::vector< std::vector<Out*> >& outij; 
		std::vector< std::vector<In*>  >& inij;
		std::set<nodeType>& attributedNodes;
};

#endif 