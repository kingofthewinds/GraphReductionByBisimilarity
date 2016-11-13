#include "bisimilarGraphReducer.hpp"

using namespace std;

BisimilarGraphReducer::BisimilarGraphReducer(ClusterHandler& clusterHandler,std::string path):
	cluster(clusterHandler),
	out(clusterHandler->getNumberOfNodes()), 
	in(clusterHandler->getNumberOfNodes()),
	s()
{
	std::set<nodeType> tempSet;
	GraphDistributor gd(cluster,path,out,in,tempSet);
	for (set<nodeType>::const_iterator i = tempSet.begin() ; i != tempSet.end() ; i++) 
	{
		s.push_back(*i);
	}
	signatures.resize(s.size());
	cluster->waitForOtherClusterNodes();
	runAlgorithm();
}

void BisimilarGraphReducer::runAlgorithm()
{
	int newCount = 1;
	
	//this should be an infinite while loop, it's a For loop for debugging purposes
	for (int i = 0 ; i < 1 ; i++)
	{
		for (int count = 0 ; count < s.size() ; count++)
		{
			vector<Signature>* sig = generateSignature(s[count]);
			signatures[count] = sig;
		}
		numberOfExpectedAnswers = 0;
		createThreadToSendSignaturesAndHandleMessages();
	}
}

void BisimilarGraphReducer::printOutVector()
{
	for (int j = 0 ; j < out.size() ; j++)
	{
		for (Out* k : out[j]) 
		{
			cout << "node "<< cluster->getrankOfCurrentNode() << " : " 
				 << "\tOut(" << cluster->getrankOfCurrentNode() << "," << j << ") = (" 
				 << k->source <<","<<k->edge <<"," <<k->destinationBlock<<")"<<endl;
		}
	}
}
void BisimilarGraphReducer::printInVector()
{
	for (int j = 0 ; j < in.size() ; j++)
	{
		for (In* k : in[j]) 
		{
			cout << "node "<< cluster->getrankOfCurrentNode() << " : " 
				 << "\tIn(" << j << "," << cluster->getrankOfCurrentNode() << ") = (" 
				 << k->dest << ")" <<endl;
		}
	}
}

void BisimilarGraphReducer::printAttributedNodes()
{
	for (int j = 0 ; j < s.size() ; j++)
	{
		cout << "node "<< cluster->getrankOfCurrentNode() << " : " 
			 << "\thas graph node : " <<  s[j]<< endl;
	}
}

bool compareSignature (const Signature& lhs, const Signature& rhs) 
{
	if (lhs.a < rhs.a)
	{
		return true;
	}
	if (lhs.a == rhs.a)
	{
		return lhs.p < rhs.p;
	}
	return false;
}

std::vector<Signature>* BisimilarGraphReducer::generateSignature(nodeType node)
{
	vector<Signature>* signature = new vector<Signature>;
	
	bool(*sigCmp)(const Signature&,const Signature&) = compareSignature;
	std::set<Signature,bool(*)(const Signature&,const Signature&)> sigSet (sigCmp);
	
	//compute signature 
	for (auto oj : out)
	{
		for (auto o : oj)
		{
			if (o->source == node)
			{
				//todo : somewhere here delete the contents of the last signature residing in vector
				sigSet.insert((struct Signature){o->edge,o->destinationBlock});
			}
		}
	}
	for (set<Signature>::const_iterator i = sigSet.begin() ; i != sigSet.end() ; i++) 
	{
		signature->push_back(*i);
	}
	return signature;
}

void* handleMessagesThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;
	gr->handleMessages();
	pthread_exit(NULL);
}

void* sendSignaturesThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;
	gr->sendSignatures();
	pthread_exit(NULL);
}

void BisimilarGraphReducer::createThreadToSendSignaturesAndHandleMessages()
{
	pthread_t sendSignatureThread;
	pthread_create(&sendSignatureThread, NULL, sendSignaturesThreadFunction, (void*)this);
	
	pthread_t handleMessagesThread;
	pthread_create(&handleMessagesThread, NULL, handleMessagesThreadFunction, (void*)this);
	
	pthread_join(sendSignatureThread, NULL);
	pthread_join(handleMessagesThread, NULL);
}

void BisimilarGraphReducer::sendSignatures()
{
	NonBlockingSendQueue< vector<Signature>* > signaturesQueue(cluster,HASH_INSERT,MPI_BYTE);
	for (vector<Signature>* sig : signatures)
	{
		int clusterToSendTo = hashSignature(*sig);
		signaturesQueue.send(clusterToSendTo,sig,sizeof(*sig));
		
		//todo : add mutexes around the following line! 
		numberOfExpectedAnswers++;
	}
	signaturesQueue.waitAndFree();
	cluster->sendSignalToAllClusterNodes(END_SIG);
}

int BisimilarGraphReducer::hashSignature(vector<Signature>& signature)
{
	int hash = 0;
	for (auto sig : signature)
	{
		hash += (int)(sig.a) + (int)(sig.p);
	}
	return (hash % (cluster->getNumberOfNodes()));
}

void BisimilarGraphReducer::handleMessages()
{
	//todo 
	//don't forget to use mutexes !
}