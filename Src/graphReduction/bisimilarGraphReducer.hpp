#ifndef BISIMILAR_PROTECT
#define BISIMILAR_PROTECT

#include <string>
#include <iostream>
#include "graphDistributor.hpp"
#include "../cluster/cluster.hpp"
#include <pthread.h>
#include <map>
#include <algorithm>


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
		static bool compareSignature (const Signature& lhs, const Signature& rhs);
		std::vector<Signature>* generateSignature(nodeType node);
		void createThreadToSendSignaturesAndHandleMessages();
		int hashSignature(std::vector<Signature>& signature);
		static bool compareSignatureVector (const std::vector<Signature>* lhs,const std::vector<Signature>* rhs);
		void clearPartialHashTable();
		void updateIDs();
		void printIDs();
		//variables : 
		ClusterHandler cluster;
		std::vector< std::vector<Out*> > out; 
		std::vector< std::vector<In*>  > in;
		std::vector<nodeType> s;
		std::vector< std::vector<Signature>* > signatures;
		std::vector<blockType> ID;
		int numberOfExpectedAnswers;
		std::map<std::vector<Signature>*,int,bool(*)(const std::vector<Signature>*,const std::vector<Signature>*)> H;
		int myNewCount;
};




#endif