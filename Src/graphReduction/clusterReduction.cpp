#include "clusterReduction.hpp"

/////////////debugging purposes//////////////
#include <iostream>
//////////////////////////////////////////////

using namespace std;

void BisimilarReduction::reduceGraph()
{
	if (cluster->getrankOfCurrentNode() == 0)
		readAndDistributeGraph();
	else 
		receiveGraphSegment();
}
void BisimilarReduction::readAndDistributeGraph()
{
	vector<edge> graph;
	graph.push_back(edge{1,'a',2});
	graph.push_back(edge{1,'b',3});
	graph.push_back(edge{2,'a',5});
	graph.push_back(edge{2,'b',4});
	graph.push_back(edge{3,'a',5});
	graph.push_back(edge{3,'b',4});
	
	set<nodeType> nodes;
	typedef vector<edge>::const_iterator iterator;
	for (iterator it = graph.begin() ; it != graph.end() ; it++)
	{
		nodes.insert(it->source);
		nodes.insert(it->dest);
	}


}
void BisimilarReduction::receiveGraphSegment()
{
	
}