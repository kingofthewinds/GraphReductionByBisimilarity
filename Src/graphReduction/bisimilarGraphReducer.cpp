#include "bisimilarGraphReducer.hpp"

BisimilarGraphReducer::BisimilarGraphReducer(ClusterHandler& clusterHandler,std::string path):cluster(clusterHandler)
{
	GraphDistributor gd(cluster,path);
}