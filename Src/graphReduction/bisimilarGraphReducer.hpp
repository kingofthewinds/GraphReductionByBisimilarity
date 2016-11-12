#ifndef BISIMILAR_PROTECT
#define BISIMILAR_PROTECT

#include <string>
#include <iostream>
#include "graphDistributor.hpp"
#include "../cluster/cluster.hpp"


class BisimilarGraphReducer
{
	public : 
		BisimilarGraphReducer(ClusterHandler& clusterHandler, std::string path );
	private :
		ClusterHandler cluster;
		std::vector< std::vector<Out*> > out; 
		std::vector< std::vector<In*>  > in;
		std::set<nodeType> s;
};


#endif