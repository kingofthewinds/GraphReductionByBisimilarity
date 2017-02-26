#ifndef PROFILER_PROTECT
#define PROFILER_PROTECT

#include <vector>
#include <iostream>

class Profiler
{
public:
	void startProfiling();
	void endProfiling();
	void roundStarted();
	void roundFinished();
	void bytesSent(int numberOfSentBytes);
	void bytesReceived(int numberOfReceivedBytes);
	void totalNumberOfPartitionsInThisRound(int numberOfPartitions);
	void sizeOfThisNodesHashTable(int sizeOfHashtable);
	void setRankAndNumberOfNodes(int rank, int numberOfNodes);
	void addMemoryMeasure();
	void printResults(std::ostream& output);
	
	
	
	
private:
	int numberOfNodesInCluster;
	int rankOfCurrentNode;
	double totalExecutionTime;
	time_t timeOfStartOfAlgorithm;
	time_t tmpStartTime;
	std::vector<double> executionTimePR;
	
	int currentNumberOFSentBytes;
	std::vector<int> numberOfSentBytesPR;
	
	int currentNumberOfReceivedBytes;
	std::vector<int> numberOfReceivedBytesPR;
	
	std::vector<int> totalNumberOfPartitionsPR;
	std::vector<int> sizeOfHashTablePR;
	
	std::vector<std::string> memoryUsagePR;
};


#endif