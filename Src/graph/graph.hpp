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
	END_OF_GRAPH_DISTRIBUTION = 2,
	HASH_INSERT = 3,
	END_SIG = 4,
	HASH_ID = 5
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

struct Signature
{
	edgeType a;
	blockType p;
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