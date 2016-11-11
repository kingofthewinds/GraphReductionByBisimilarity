#ifndef GRAPH_PROTECT
#define GRAPH_PROTECT

typedef int nodeType;
typedef char edgeType;
typedef int blockType;
typedef int graphSize;
enum tags 
{ 
	OUT = 0, 
	IN = 1,
	ENDOfGraphDistribution = 2
};

struct edge 
{
	nodeType source;
	edgeType edge;
	nodeType dest;
};


struct Out
{
	nodeType source;
	edgeType edge;
	blockType destinationBlock;
};

struct In
{
	nodeType dest;
};	

struct initOut
{
	nodeType source;
	edgeType edge;
	blockType destinationBlock;
	int clusterDestinationNode; 
};	

struct initIn
{
	nodeType dest;
	int clusterSourceNode;
};

#endif