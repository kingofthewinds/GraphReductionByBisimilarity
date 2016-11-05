#ifndef MPI_PROTECT
#define MPI_PROTECT	

#include <mpi.h>
#include <stdexcept>


class Cluster
{
	public :
		Cluster();
		int getNumberOfNodes(){return numberOfNodes;}
		int getrankOfCurrentNode(){return rankOfCurrentNode;}
		
	private : 
		int numberOfNodes;
		int rankOfCurrentNode;
};

class ClusterHandler
{
	public : 
		ClusterHandler(): cluster(0), refptr(new size_t(1)) { }
		ClusterHandler(Cluster* t):  cluster(t), refptr(new size_t(1)) { }
		ClusterHandler(const ClusterHandler& h): cluster(h.cluster), refptr(h.refptr) {++*refptr;}
		ClusterHandler& operator=(const ClusterHandler&);
		~ClusterHandler();
		operator bool() const { return cluster; }
		Cluster& operator*() const {
			if (cluster)
				return *cluster;
			throw std::runtime_error("cluster not initialized");
		}
		Cluster* operator->() const {
			if (cluster)
				return cluster;
			throw std::runtime_error("cluster not initialized");
		}


	
	private:
		Cluster* cluster;
		size_t* refptr;
};

#endif

