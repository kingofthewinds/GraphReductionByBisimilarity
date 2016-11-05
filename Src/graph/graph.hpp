#ifndef GRAPH_PROTECT
#define GRAPH_PROTECT

#include "../types.hpp"

struct edge 
{
	nodeType source;
	edgeType edge;
	nodeType dest;
};

struct out
{
	nodeType source;
	edgeType edge;
	blockType destinationBlock;
};	

#endif