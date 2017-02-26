#ifndef BISIMILAR_PROTECT
#define BISIMILAR_PROTECT

#include <string>
#include <iostream>
#include "graphDistributor.hpp"
#include "profiler.hpp"
#include "../cluster/cluster.hpp"
#include <pthread.h>
#include <map>
#include <algorithm>

class BisimilarGraphReducer
{
	public : 
		/**
			the constructor takes a file containing the graph as parameter and runs the algorithm on it
			@param clusterHandler the cluster handler object
			@param path the path to the file  
		*/
		BisimilarGraphReducer(ClusterHandler& clusterHandler, std::string path, Profiler& profiler);
		
		/**
			send the signatures of the nodes to the approppriate node so they can be inserted to 
			the hash table 
		*/
		void sendSignatures();
		
		/**
			handles signature insertion messages and stores results 
		*/
		void handleMessages();
		
		void sendNewIDs();
		void receiveIDs();
		
	private :
		
		/**
			for debugging purposes : pretty prints the Out vector
		*/
		void printOutVector();
		
		/**
			for debugging purposes : pretty prints the In vector
		*/
		void printInVector();
		
		/**
			for debugging purposes : pretty prints the list of attributed nodes to the cluster 
		*/
		void printAttributedNodes();
		
		/**
			runs the Blom Orzan algorithm
		*/
		void runAlgorithm();
		
		/**
			generates the signature of a given node 
		*/
		void generateSignatures();
		
		/**
			compares two signature elements (used in a set as the comparison predicate)
		*/
		static bool compareSignature (const Signature& lhs, const Signature& rhs);
		
		static bool compareSignatureEqual (const Signature& lhs, const Signature& rhs);

		
		/**
			creates two threads that will ultimately call the sendSignatures and the handleMessages memeber functions 
			on this object 
		*/
		void createThreadToSendSignaturesAndHandleMessages();
		
		/**
			hash function that returns the rank of the cluster node containig the partial hash table in which
			the signature send in as parameter should be added 
			@param signature the signature to be hashed
			@return the cluster rank to which the signature should be sent for insertion 
		*/
		int hashSignature(std::vector<Signature>& signature);
		
		/**
			compares two complete signatures and return true if lhs is smaller than rhs (it is used 
			for the partial hash table as the comparison predicate)
		*/
		static bool compareSignatureVector (const std::vector<Signature>* lhs,const std::vector<Signature>* rhs);
		
		/**
			clears out the partial hash table and frees the memeory dedicated to each of the 
			signatures it was contained 
		*/
		void clearPartialHashTable();
		
		/**
			receives the new id of the block to which contain the destination elements of the out vector 
		*/
		void updateIDs();
		
		/**
			prints the list of all nodes and the latest id attributes to each 
		*/
		void printIDs();
		
		//variables : 
		ClusterHandler cluster;
		std::vector< std::vector<Out*> > out; 
		std::vector< std::vector<In*>  > in;
		std::map<nodeType,nodeInfo> s;
		int numberOfExpectedAnswers;
		std::map<std::vector<Signature>*,int,bool(*)(const std::vector<Signature>*,const std::vector<Signature>*)> H;
		int myNewCount;
		Profiler& profiler;
};




#endif