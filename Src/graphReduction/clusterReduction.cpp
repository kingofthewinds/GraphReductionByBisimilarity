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
	//somehow load the graph in the format : source , lable of the edge , destination
	vector<edge> graph;
	graph.push_back(edge{0,'a',1});
	graph.push_back(edge{0,'b',2});
	graph.push_back(edge{1,'a',4});
	graph.push_back(edge{1,'b',3});
	graph.push_back(edge{2,'a',4});
	graph.push_back(edge{2,'b',3});
	
	//put all the distinct nodes in a set and count them
	set<nodeType> nodes;
	typedef vector<edge>::const_iterator iterator;
	for (iterator it = graph.begin() ; it != graph.end() ; it++)
	{
		nodes.insert(it->source);
		nodes.insert(it->dest);
	}
	graphSize ngn = nodes.size(); //number of graph nodes 
	graphSize ncn = cluster->getNumberOfNodes(); // number of cluster nodes 
	
	graphSize npn = (ngn % ncn == 0 ? ngn/ncn : ngn/ncn+1); //#graph nodes per cluster node
	
	NonBlockingSendQueue<initOut*> outQueue(cluster,OUT,MPI_BYTE);
	for (iterator it = graph.begin() ; it != graph.end() ; it++)
	{
		//get source, lable and the destination of the edge
		nodeType source = it->source; 
		edgeType edge = it->edge;
		nodeType dest = it->dest;
		cout << "checking for (source,edge,destination) : " << source << " , " << edge << " , " << dest << endl;
		
		//find out to which OUTij it belongs 
		int i,j;
		set<nodeType>::const_iterator pos = find(nodes.begin(),nodes.end(),source);
		i = ((int)distance(nodes.begin(), pos))/npn;
		pos = find(nodes.begin(),nodes.end(),dest);
		j = ((int)distance(nodes.begin(), pos))/npn;
		cout << "\tout indexes (i,j) are : " << i << " , " << j << endl;
		
		//todo : send an OUTij signal to i with (source, edge, 0)
		initOut* o = new initOut;
		o->out.source = source;
		o->out.edge = edge;
		o->out.destinationBlock = 0;
		o->clusterDestinationNode = j;
		outQueue.send(i,o,sizeof(o));
		 
		//todo : send an INij to j with (dest)
	}
	outQueue.waitAndFree();
	
	
	
}
void BisimilarReduction::receiveGraphSegment()
{
	
}