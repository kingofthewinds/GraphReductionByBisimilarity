#include "GraphDistributor.hpp"

/////////////debugging purposes//////////////
#include <iostream>
//////////////////////////////////////////////

using namespace std;

void GraphDistributor::reduceGraph(string path)
{
	if (cluster->getrankOfCurrentNode() == 0)
		readAndDistributeGraph(path);
	receiveGraphSegment();
}
void GraphDistributor::readAndDistributeGraph(string pathToFile)
{
	cout << "root : " << endl;
	//somehow load the graph in the format : source , lable of the edge , destination
	vector<edge> graph;
	graph.push_back(edge{0,'a',1});
	graph.push_back(edge{0,'b',2});
	graph.push_back(edge{1,'a',4});
	graph.push_back(edge{1,'b',3});
	graph.push_back(edge{2,'a',4});
	graph.push_back(edge{2,'b',3});
	
	graph.push_back(edge{4,'a',5});
	graph.push_back(edge{3,'a',6});
	
	graph.push_back(edge{5,'b',7});
	graph.push_back(edge{6,'b',7});
	
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
	NonBlockingSendQueue<initIn*> inQueue(cluster,IN,MPI_BYTE);
	for (iterator it = graph.begin() ; it != graph.end() ; it++)
	{
		//get source, lable and the destination of the edge
		nodeType source = it->source; 
		edgeType edge = it->edge;
		nodeType dest = it->dest;
		cout << "\tchecking for (source,edge,destination) : " << source << " , " << edge << " , " << dest << endl;
		
		//find out to which OUTij it belongs 
		int i,j;
		set<nodeType>::const_iterator pos = find(nodes.begin(),nodes.end(),source);
		i = ((int)distance(nodes.begin(), pos))/npn;
		pos = find(nodes.begin(),nodes.end(),dest);
		j = ((int)distance(nodes.begin(), pos))/npn;
		cout << "\t\tout indexes (i,j) are : " << i << " , " << j << endl;
		
		//todo : send an OUTij signal to i with (source, edge, 0)
		initOut* o = createInitOutStruct(source, edge, 0, j);
		outQueue.send(i,o,sizeof(initOut));
		 
		//todo : send an INij to j with (dest)
		initIn* in = createInitInStruct(dest, i);
		inQueue.send(j,in,sizeof(initIn));
	}
	outQueue.waitAndFree();
	inQueue.waitAndFree();
	
	cluster->sendSignalToAllClusterNodes(END_OF_GRAPH_DISTRIBUTION);
}


void GraphDistributor::receiveGraphSegment()
{
	tags tag = OUT;
	while (tag != END_OF_GRAPH_DISTRIBUTION)
	{
		int count;
		int source;
		unsigned char* data = cluster->receive(MPI_BYTE, &count, &source, (int *)&tag);
		if (tag == OUT)
		{
			addOut((initOut*) data);
		}else if (tag == IN)
		{
			addIn((initIn*) data);			
		}
	}
	
}

initOut* GraphDistributor::createInitOutStruct(nodeType source, edgeType edge, blockType destinationBlock, int clusterDestinationNode)
{
	initOut* out = new initOut;
	out->source = source;
	out->edge = edge;
	out->destinationBlock = destinationBlock;
	out->clusterDestinationNode = clusterDestinationNode;
	return out;
}


initIn* GraphDistributor::createInitInStruct(nodeType dest, int clusterSourceNode)
{
	initIn* in = new initIn;
	in->dest = dest;
	in->clusterSourceNode = clusterSourceNode;
	return in;
}

void GraphDistributor::addOut(initOut* iI)
{
	Out* out = new Out;
	out->source = iI->source;
	out->edge = iI->edge;
	out->destinationBlock = iI->destinationBlock;
	int j = iI->clusterDestinationNode; 
	
	outij[j].push_back(out);
	attributedNodes.insert(out->source);
	delete iI;
	
}
void GraphDistributor::addIn(initIn* iO)
{
	In* in = new In;
	in->dest = iO->dest;
	int clusterSourceNode = iO->clusterSourceNode;
	
	inij[clusterSourceNode].push_back(in);
	attributedNodes.insert(in->dest);
	delete iO;
	
}