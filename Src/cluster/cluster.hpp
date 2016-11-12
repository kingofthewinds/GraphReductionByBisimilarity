#ifndef MPI_PROTECT
#define MPI_PROTECT	

#include <mpi.h>
#include <stdexcept>
#include "../graph/graph.hpp"
#include <vector>


class Cluster
{
	public :
		Cluster();
		int getNumberOfNodes(){return numberOfNodes;}
		int getrankOfCurrentNode(){return rankOfCurrentNode;}
		MPI_Request* send(int destination,tags tag, const void *buf,int count,MPI_Datatype datatype);
		void waitForSend(MPI_Request *request);
		unsigned char* receive(MPI_Datatype datatype, int* count, int* source, int* tag);
		void waitForOtherClusterNodes();
		
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

template <class BufferType>
class NonBlockingSendQueue 
{
	public :
		NonBlockingSendQueue(ClusterHandler cl,tags t,MPI_Datatype dt) : cluster(cl), tag(t), datatype(dt) {}
		void send(int destination,const BufferType message,int count);
		void waitAndFree();
	private :
		std::vector<MPI_Request*> sentMessageHandlers;
		std::vector<BufferType> sentMessages;
		ClusterHandler cluster;
		tags tag;
		MPI_Datatype datatype;
	
};

template <typename BufferType>
void NonBlockingSendQueue<BufferType>::send(int destination,const BufferType message,int count)
{
	sentMessages.push_back(message);
	sentMessageHandlers.push_back(cluster->send(destination,tag,(void *)message ,count,datatype));
}

template <typename BufferType>
void NonBlockingSendQueue<BufferType>::waitAndFree()
{
	for (int i = 0; i < sentMessageHandlers.size() ; i++)
	{
		cluster->waitForSend(sentMessageHandlers[i]);
		delete sentMessages[i];
	}
	sentMessages.clear();
	sentMessageHandlers.clear();
}


#endif

