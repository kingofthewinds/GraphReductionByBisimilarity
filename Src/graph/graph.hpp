#ifndef GRAPH_PROTECT
#define GRAPH_PROTECT

#include <vector>

typedef int nodeType;
typedef int edgeType;
typedef int blockType;
typedef int graphSize;

struct edge 
{
	blockType sourceID;
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

struct nodeInfo
{
	blockType id;
	std::vector<Signature>* signature;
};

struct SignatureAnswer
{
	nodeType node;
	blockType blockID;
};



#endif