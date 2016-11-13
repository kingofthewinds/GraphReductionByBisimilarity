#ifndef BISIMILAR_PROTECT
#define BISIMILAR_PROTECT

#include <string>
#include <iostream>
#include "graphDistributor.hpp"
#include "../cluster/cluster.hpp"
#include <pthread.h>


class BisimilarGraphReducer
{
	public : 
		BisimilarGraphReducer(ClusterHandler& clusterHandler, std::string path );
		void sendSignatures();
		void handleMessages();
	private :
		//methods : 
		void printOutVector();
		void printInVector();
		void printAttributedNodes();
		void runAlgorithm();
		std::vector<Signature>* generateSignature(nodeType node);
		void createThreadToSendSignaturesAndHandleMessages();
		int hashSignature(std::vector<Signature>& signature);
		//variables : 
		ClusterHandler cluster;
		std::vector< std::vector<Out*> > out; 
		std::vector< std::vector<In*>  > in;
		std::vector<nodeType> s;
		std::vector< std::vector<Signature>* > signatures;
		int numberOfExpectedAnswers;
};




#endif