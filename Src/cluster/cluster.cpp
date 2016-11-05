#include "cluster.hpp"


Cluster::Cluster()
{
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfNodes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankOfCurrentNode);
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