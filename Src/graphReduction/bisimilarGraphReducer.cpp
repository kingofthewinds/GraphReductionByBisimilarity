#include "bisimilarGraphReducer.hpp"

using namespace std;

BisimilarGraphReducer::BisimilarGraphReducer(ClusterHandler& clusterHandler,std::string path):
	cluster(clusterHandler),
	out(clusterHandler->getNumberOfNodes()), 
	in(clusterHandler->getNumberOfNodes()),
	s()
{
	GraphDistributor gd(cluster,path,out,in,s);
	
	cout << cluster->getrankOfCurrentNode() << " : " << endl;
	
	for (auto i : s)
	{
		cout << "\t" << i << endl;;
	}
}