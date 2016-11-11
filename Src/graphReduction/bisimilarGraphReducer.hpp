#ifndef BISIMILAR_PROTECT
#define BISIMILAR_PROTECT

#include <string>
#include "graphDistributor.hpp"
#include "../cluster/cluster.hpp"

class BisimilarGraphReducer
{
	public : 
		BisimilarGraphReducer( ClusterHandler& clusterHandler , std::string path );
	private :
		ClusterHandler cluster;
	
};


#endif