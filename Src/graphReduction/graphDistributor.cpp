#include "GraphDistributor.hpp"

using namespace std;

void GraphDistributor::reduceGraph(string path)
{
	//if the rank of the node is zero then it's the node that will read and distribute data 
	//otherwise it will just receive and save data (the receiveGraphSegment member function) 
	if (cluster->getrankOfCurrentNode() == 0)
		readAndDistributeGraph(path);
	receiveGraphSegment();
}

void GraphDistributor::readAndDistributeGraph(string pathToFile)
{
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
	
	NonBlockingSendQueue<initOut*> outQueue(cluster,OUT);
	NonBlockingSendQueue<initIn*> inQueue(cluster,IN);
	for (iterator it = graph.begin() ; it != graph.end() ; it++)
	{
		//get source, lable and the destination of the edge
		nodeType source = it->source; 
		edgeType edge = it->edge;
		nodeType dest = it->dest;
		
		//find out to which OUTij it belongs 
		int i,j;
		set<nodeType>::const_iterator pos = find(nodes.begin(),nodes.end(),source);
		i = ((graphSize)distance(nodes.begin(), pos))/npn; //the source cluster node rank
		pos = find(nodes.begin(),nodes.end(),dest);
		j = ((graphSize)distance(nodes.begin(), pos))/npn; //the destination cluster node rank
		
		//send an initOUT signal to i with (source, edge, 0 , j) --> we also send j so that the receiving node 
		//whill know where to which OUTij it belgongs 
		initOut* o = createInitOutStruct(source, edge, 0, j);
		outQueue.send(i,o,sizeof(initOut));
		 
		//send an INij to j with (dest , i)
		initIn* in = createInitInStruct(dest, i);
		inQueue.send(j,in,sizeof(initIn));
	}
	//wait for in and out messages to be sent
	outQueue.waitAndFree();  
	inQueue.waitAndFree();
	
	//send a signal to all nodes telling them the distribution has finished so they can 
	//start running the main algorithm 
	cluster->sendSignalToAllClusterNodes(END_OF_GRAPH_DISTRIBUTION);
}

void GraphDistributor::receiveGraphSegment()
{
	tags tag ;  
	int count;
	int source;
	do  
	{
		unsigned char* data = cluster->receive(MPI_BYTE, &count, &source, (int *)&tag); //receive data 
		if (tag == OUT)
		{
			addOut((initOut*) data);
		}else if (tag == IN)
		{
			addIn((initIn*) data);			
		}
	}while (tag != END_OF_GRAPH_DISTRIBUTION); //stop when root says graph distribution has finished 
	
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


void GraphDistributor::addOut(initOut* iO)
{
	Out* out = new Out;
	out->source = iO->source;
	out->edge = iO->edge;
	out->destinationBlock = iO->destinationBlock;
	int j = iO->clusterDestinationNode; //the cluster node to which the destination of the edge belongs
	
	outij[j].push_back(out);
	attributedNodes.insert(out->source);
	delete iO;//delete the initOut object passed in as parameter because it's not neede anymore 
	
}


void GraphDistributor::addIn(initIn* iI)
{
	In* in = new In;
	in->dest = iI->dest;
	int i = iI->clusterSourceNode;
	
	inij[i].push_back(in);
	attributedNodes.insert(in->dest);
	delete iI;//delete the initIn object passed in as parameter because it's not neede anymore
}