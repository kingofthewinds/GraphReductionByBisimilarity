#include "profiler.hpp"
#include <vector>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

void Profiler::startProfiling()
{
	timeOfStartOfAlgorithm = time(0);
}
void Profiler::endProfiling()
{
	time_t now = time(0);
	totalExecutionTime = (double) (now-timeOfStartOfAlgorithm) ;
}

void Profiler::roundStarted()
{
	tmpStartTime = time(0);
	currentNumberOFSentBytes = 0;
	currentNumberOfReceivedBytes = 0;
}
void Profiler::roundFinished()
{
	time_t now = time(0);
	double roundTime = (double) (now-tmpStartTime) ;
	executionTimePR.push_back(roundTime);
	
	numberOfSentBytesPR.push_back(currentNumberOFSentBytes);
	numberOfReceivedBytesPR.push_back(currentNumberOfReceivedBytes);
	
}
void Profiler::bytesSent(int numberOfSentBytes)
{
	currentNumberOFSentBytes += numberOfSentBytes;
}
void Profiler::bytesReceived(int numberOfReceivedBytes)
{
	currentNumberOfReceivedBytes += numberOfReceivedBytes;
}

void Profiler::totalNumberOfPartitionsInThisRound(int numberOfPartitions)
{
	totalNumberOfPartitionsPR.push_back(numberOfPartitions);
}
void Profiler::sizeOfThisNodesHashTable(int sizeOfHashtable)
{
	sizeOfHashTablePR.push_back(sizeOfHashtable);
}

void Profiler::setRankAndNumberOfNodes(int rank, int numberOfNodes)
{
	numberOfNodesInCluster = numberOfNodes;
	rankOfCurrentNode = rank;
}

string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != NULL)
			result += buffer.data();
	}
	return result;
}

void Profiler::addMemoryMeasure()
{
	memoryUsagePR.push_back(exec("ls"));//top -aMn 1
}

void Profiler::printResults(std::ostream& output)
{
	output << "number of nodes in the cluster : " << numberOfNodesInCluster << endl;
	output << "rank : " << rankOfCurrentNode << endl;
	output << "total execution time : " << totalExecutionTime << endl;
	for (int i = 0 ; i < executionTimePR.size() ; i++)
	{
		output << "-------------------------------------------------------------------------------------------------" << endl;
		output << "duration : " << executionTimePR[i] << endl;
		output << "#bytes sent : " << numberOfSentBytesPR[i] << endl;
		output << "#bytes received : " << numberOfReceivedBytesPR[i] << endl;
		output << "# partitions in this node : " << sizeOfHashTablePR[i] << endl;
		output << "total number of partitions : " << totalNumberOfPartitionsPR[i] << endl;
		output << "************* memory usage : ******************" << endl;
		output << memoryUsagePR[i] << endl;
	}
	
}
	

	
	
	



