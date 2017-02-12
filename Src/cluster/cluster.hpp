#ifndef MPI_PROTECT
#define MPI_PROTECT	

#include <mpi.h>
#include <stdexcept>
#include "../graph/graph.hpp"
#include <vector>

//enumeration of the possible (not complete) message tags that can be sent 
enum tags 
{ 
	OUT = 0, 
	IN = 1,
	END_OF_GRAPH_DISTRIBUTION = 2,
	HASH_INSERT = 3,
	END_SIG = 4,
	HASH_ID = 5,
	UPDATE = 6
};

/**
	This is a wrapper class around the c MPI functions. It was created because the MPI c++ libraries were 
	deprecated and boost wrappers needed additional libraries that were unnecessarily big for the project 
	at hand. This wrapper is also a bit less general (ex : use of tags instead of just integers for message 
	passing) than that of boost mpi-wrapper libraries which will make the library simpler
*/
class Cluster
{
	public :
		//initializez a multiple-thread cluster
		Cluster();
		//returns the total number of nodes contained in the cluster 
		int getNumberOfNodes(){return numberOfNodes;}
		//returns the rank (=ID (starting at zero)) of the cluster node calling the function
		int getrankOfCurrentNode(){return rankOfCurrentNode;}
		/**
			a non-blocking call that sends a messasge to another cluster node.

			@param destination the rank of the receiving cluster node 
			@param tag the tag of the message (see the tags enumeration) 
			@param buf the beginning of the memory buffer that will be sent 
				-->warning : the memory buffer should be contiguous and persistent until the send operation finishes
			@param count the number of types MPI_Datatype that should be sent from the buffer
			@param datatype datatype (of type MPI_DATATYPE) of the atomic elements of the buffer 
			@return A handle that can be used in the waitForSend function to wait for the message to be sent  
		*/
		void send(int destination,tags tag, const void *buf,int count,MPI_Datatype datatype);
		/**
			A blocking call that waits until a message is received. It the allocates enough memory for it, saves it
			in the allocated memory and returns the address of alocted memory. 

			@param datatype the type of data it will be willing to receive  
			@param count* it will save inside this variable the number of datatypes of data it received  
			@param source* it will save inside the rankd of the source cluster node that sent this data 
			@param tag* it will save inside the tag of the message that was received 
			@return the first address of the buffer it allocated where it saved the received message  
		*/
		unsigned char* receive(MPI_Datatype datatype, int* count, int* source, int* tag);
		/**
			whenever this call is made by one of the nodes of the culuster, the call will block until 
			all the other nodes of the custer make the same call. 
			main usage : cluster nodes wait for eachother until they are all in the same step of the code 
				-->node synchronisation
		*/
		void waitForOtherClusterNodes();
		//sends a given tag to all the cluster nodes (without any message attached to it)
		void sendSignalToAllClusterNodes(tags t);
		/**
			once the call is made by all the nodes of a cluste, the valuse each node passed as argument will
			be summed (over all the nodes) and the sum will be returned by the function
			@param myValue the contrubution of the calling node 
			@return The result of the sum over all the nodes 
			warning : all the nodes shoud make the call at the same type --> some synchrinissation needed
		*/
		int sumAllClusterNodeValues(int myValue);
	private : 
		//the total number of nodes in the cluster 
		int numberOfNodes;
		//the rank(Identifier) of the current node  
		int rankOfCurrentNode;
};


/**
	This class is a wrapper around the cluster class that makes sure that only one copy is created
	and takes care of the assignment operator and the copy constructor so that no copy of the object 
	is created and that when need be, the object is deleted.
	note that a singleton cluster would have ensured the uniqueness of Cluster but not that it is deleted 
	when the time comes, so that pattern was not used and given that it doesnt make sense to instantiate 
	a cluster each time we need it, the ClusterHandler class makes perfect sense
*/
class ClusterHandler
{
	public : 
		/**
			instantiates a cluster hander object and creates an size_t object that it attaches to the 
			number of references in the code that point to the wrapped object 
		*/
		ClusterHandler(Cluster* t):  cluster(t), refptr(new size_t(1)) { }
		/**
			instantiates a new clusterhandler object from another one (copy constructor) but ensures to
			copy only a pointer to the wrapped cluster and to update the size_t variable assocated 
			to it so that it will contain the total number of clusterhandler objects pointing to the 
			underlying cluster object .
		*/
		ClusterHandler(const ClusterHandler& h): cluster(h.cluster), refptr(h.refptr) {++*refptr;}
		/**
			from now on common operators are being overloaded so that they will be applies to the cluster 
			object and not the ClusterHandler. Note that given these functions will be called a huge amount 
			of time, to increase speed, checking the cluster object exists before applying the operators and 
			maybee throwing a runtime error has been ommitted !
		*/
		ClusterHandler& operator=(const ClusterHandler&);
		~ClusterHandler();
		operator bool() const { return cluster; }
		//the following are implementd as inline functions to make the code fadster :
		Cluster& operator*() const {
			return *cluster;
		}
		Cluster* operator->() const {
			return cluster;
		}

	private:
		//The wrapped cluster 
		Cluster* cluster;
		//Pointer to the total number of ClusterHandler objects that all point to the same cluster 
		size_t* refptr;
};


/**
	The following class wraps a cluster and can be used to send messages of the same type (buffer of 
	a given type corresponding to the template of the class), same datatype and same tag. It also 
	frees the memory buffers of the messages that have already been sent using the blocking waitAndFree
	member function 
*/
template <class BufferType>
class NonBlockingSendQueue 
{
	public :
		/**
			Constructor 
			
			@param cl the cluster handler object 
			@param t the tag of all the message that will be sent using this object 
			@param dt datatype of all the messages that will be sent using this object 
		*/
		NonBlockingSendQueue(ClusterHandler cl,tags t) : cluster(cl), tag(t) {}
		/**
			send the message (non blocking)
			
			@param destination the rank of the receiving cluster 
			@param message pointer to the beginning of a buffer to be sent 
				-->warning : the buffer has to be persistent until the send opetion has finished 
			@param count the number of datatype(defined in the constructor) elements to be sent 
		*/
		void send(int destination,const BufferType message,int count);
		void sendVector(int destination,std::vector<BufferType>* message,int count,bool eraseWhenSent);
	private :
		ClusterHandler cluster;
		tags tag;	
};

template <typename BufferType>
void NonBlockingSendQueue<BufferType>::send(int destination,const BufferType message,int count)
{
	cluster->send(destination,tag,(void *)message ,count,MPI_BYTE);
	delete message;
}

template <typename BufferType>
void NonBlockingSendQueue<BufferType>::sendVector(	int destination,std::vector<BufferType>* message,
													int count,bool eraseWhenSent)
{
	cluster->send(destination,tag,message->data(),count,MPI_BYTE);
	if (eraseWhenSent == true)
	{
		message->clear();
		delete message;
	}
}


#endif

