#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

struct edge 
{
	int sourceID;
	int source;
	int edge;
	int dest;
};

using namespace std;
int main(int argc, char *argv[]) {
	
	vector<edge> graph;
		
		
	ifstream file("tpch.edge.hashed");
	string line;
	while(std::getline(file, line))
	{
		stringstream  lineStream(line);
		int fp , sn, lb, dn;
		// Read an integer at a time from the line
		while(lineStream >> fp >> sn >> lb >> dn)
		{
			graph.push_back((edge){fp,sn,lb,dn});
		}

	}
	cout << "just loaded the graph : " << graph.size() << endl;;
	cout << "last line is : " << graph[graph.size()-1].sourceID << " , " << graph[graph.size()-1].source << " , " << graph[graph.size()-1].edge  << " , " << graph[graph.size()-1].dest << endl;
	
	cout << "last line is : " << graph[5940000].sourceID << " , " << graph[5940000].source << " , " << graph[5940000].edge  << " , " << graph[5940000].dest << endl;
	
	

	return 0;
}