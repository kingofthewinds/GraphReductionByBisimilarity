#include "GraphDistributor.hpp"
#include <ctime>
#include <unordered_map>
using namespace std;

void GraphDistributor::readAndDistributeGraph(string pathToFile)
{
	vector<edge> graph;
	
	
	ifstream file(pathToFile);
	string line;
	while(std::getline(file, line))
	{
		stringstream  lineStream(line);
		int fp , sn, lb, dn;
		// Read an integer at a time from the line
		while(lineStream >> fp >> sn >> lb >> dn)
		{
			graph.push_back(edge{fp,sn,lb,dn});
		}

	}
	cout << "just loaded the graph" << endl;
	
	
	/*
	graph.push_back(edge{0,0,0,1});
	graph.push_back(edge{0,0,1,2});
	graph.push_back(edge{0,1,0,4});
	graph.push_back(edge{0,1,1,3});
	graph.push_back(edge{0,2,0,4});
	graph.push_back(edge{0,2,1,3});
	graph.push_back(edge{0,4,0,5});
	graph.push_back(edge{0,3,0,6});
	graph.push_back(edge{0,5,1,7});
	graph.push_back(edge{0,6,1,7});
	*/
	
	unordered_map<nodeType,blockType> initialIDs;
	for (edge x : graph)
	{
		initialIDs[x.source] = x.sourceID;
	}
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

	
	vector<nodeType> boundaries;
	int i = 0;
	for (set<nodeType>::iterator it = nodes.begin() ; it != nodes.end() ; it++)
	{
		if (i % npn == 0) 
			boundaries.push_back(*it);
		i++;
	}
	
	for (iterator it = graph.begin() ; it != graph.end() ; it++)
	{
		//clock_t begin = clock();
		//get source, lable and the destination of the edge
		blockType sourceID = it->sourceID;
		nodeType source = it->source; 
		edgeType edge = it->edge;
		nodeType dest = it->dest;
		//cout << double(clock() - begin) / CLOCKS_PER_SEC  << " "  ;
		//find out to which OUTij it belongs 
		
		int i = boundaries.size() - 1;
		for (i = boundaries.size() - 1 ; i >= 0 ; i--)
		{
			if ( boundaries[i] <= source)
				break;
		}
		int j = boundaries.size() - 1;
		for (j = boundaries.size() - 1 ; j >= 0 ; j--)
		{
			if ( boundaries[j] <= dest)
				break;
		}
		
		if (i == cluster->getrankOfCurrentNode())
		{
			Out* out = new Out;
			out->source = source;
			out->edge = edge;
			out->destinationBlock = initialIDs[dest];
			outij[j].push_back(out);
			attributedNodes[source] = nodeInfo{sourceID,new vector<Signature>} ;
		}
		if (j == cluster->getrankOfCurrentNode())
		{
			In* in = new In;
			in->dest = dest;			
			inij[i].push_back(in);
			attributedNodes[dest] = nodeInfo{initialIDs[dest],new vector<Signature>} ;
		}
		
	}
	graph.clear();
	initialIDs.clear();
	nodes.clear();
	cluster->waitForOtherClusterNodes();
	cout << "graph distribution finished ! " << endl;
}
