#include "cluster.hpp"

#include <iostream>
using namespace std;

Cluster::Cluster()
{
	int provided;
	MPI_Init_thread(NULL, NULL,MPI_THREAD_MULTIPLE, &provided);
	if (provided != MPI_THREAD_MULTIPLE)
	{
		cout << "Error : Your MPI implementation doesn't allow multiple threads !" << endl;
		exit(1);
	}
	
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfNodes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankOfCurrentNode);
}

//A blocking send 
//dataSize is the total size of the data (if a vactor then vector * size of each element)
void Cluster::send(int destination,tags tag, const void *buf,int dataSize,MPI_Datatype datatype)
{
	//MPI_Send(buf , dataSize , datatype, destination , tag, MPI_COMM_WORLD,request);
	MPI_Send(buf , dataSize , datatype, destination , tag, MPI_COMM_WORLD);
}

unsigned char* Cluster::receive(MPI_Datatype datatype, int* count, int* source, int* tag)
{
	MPI_Status status;
	MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	// Resize your incoming walker buffer based on how much data is
	// being received
	MPI_Get_count(&status, MPI_BYTE, count);
	unsigned char* data = new unsigned char[*count];
	*source = status.MPI_SOURCE;
	*tag = status.MPI_TAG;
	MPI_Recv((void*)data, *count, datatype, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	return data;
}

void Cluster::waitForOtherClusterNodes()
{
	MPI_Barrier(MPI_COMM_WORLD);
}

void Cluster::sendSignalToAllClusterNodes(tags t)
{
	for (int i = 0 ; i < getNumberOfNodes() ; i++)
	{
		send(i,t, NULL,0,MPI_BYTE);
	}
}

int Cluster::sumAllClusterNodeValues(int myValue)
{
	int sum = 0;
	int* receivedValues = new int[getNumberOfNodes()] ;
	MPI_Allgather( &myValue, 1, MPI_INT, receivedValues, 1 , MPI_INT, MPI_COMM_WORLD);
	for (int i = 0 ; i < getNumberOfNodes() ; i++)
	{
		sum += receivedValues[i];
	}
	delete[] receivedValues;
	return sum;
}

ClusterHandler& ClusterHandler::operator=(const ClusterHandler& rhs)
{
	++*rhs.refptr;
	// free the left-hand side, destroying pointers if appropriate
	if (--*refptr == 0) {
		delete refptr;
		delete cluster;
	}

	// copy in values from the right-hand side
	refptr = rhs.refptr;
	cluster = rhs.cluster;
	return *this;
}

ClusterHandler::~ClusterHandler()
{
	if (--*refptr == 0) {
		delete refptr;
		delete cluster;
	}
}




