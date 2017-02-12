#include "bisimilarGraphReducer.hpp"

using namespace std;

BisimilarGraphReducer::BisimilarGraphReducer(ClusterHandler& clusterHandler,std::string path):
	cluster(clusterHandler),
	//initialize the sizes of out and in vectors to the number of cluster nodes 
	out(clusterHandler->getNumberOfNodes()), 
	in(clusterHandler->getNumberOfNodes()),
	s(), H(BisimilarGraphReducer::compareSignatureVector) 
{
	std::set<nodeType> tempSet;//create a tempporary set to pass to the graph distributor 
	GraphDistributor gd(cluster,path,out,in,s); // distribute the graph 

	cluster->waitForOtherClusterNodes();//we wait for all nodes to get here before running the algorithm
	runAlgorithm();
}

void BisimilarGraphReducer::runAlgorithm()
{
	int newCount = 1;
	
	while(true)
	{
		cout << "node "<< cluster->getrankOfCurrentNode() << " started running the algorithm ! " << endl;
		
		generateSignatures();//generate their signature 
		cluster->waitForOtherClusterNodes();
		cout << "node "<< cluster->getrankOfCurrentNode() << " calculated it's signatures !" << endl;
		numberOfExpectedAnswers = 0;
		createThreadToSendSignaturesAndHandleMessages();
		cout << "node "<< cluster->getrankOfCurrentNode() << " finished internode communication ! " << endl;
		int oldCount = newCount;
		cluster->waitForOtherClusterNodes();
		newCount = cluster->sumAllClusterNodeValues(myNewCount);
		cout << "current number of partitions : " << newCount << endl ;
		if (oldCount == newCount)
		{
			break;
		}
		cluster->waitForOtherClusterNodes();
		updateIDs();
		cout << "node "<< cluster->getrankOfCurrentNode() << " updated it's IDs !" << endl;
		cluster->waitForOtherClusterNodes();
		cout << "----------------------------------------------------------------------------" << endl;
	}
	printIDs();
}



bool BisimilarGraphReducer::compareSignature (const Signature& lhs, const Signature& rhs) 
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

void BisimilarGraphReducer::generateSignatures()
{
	for (map<nodeType,nodeInfo>::iterator it = s.begin() ; it != s.end() ; it++)//for all nodes of the graph
	{
		it->second.signature->clear();
		it->second.signature->reserve(5);
	}
	//compute signature 
	for (auto oj : out)
	{
		for (auto o : oj)
		{
			s[o->source].signature->push_back(Signature{o->edge,o->destinationBlock});
		}
	}
	bool(*sigCmp)(const Signature&,const Signature&) = BisimilarGraphReducer::compareSignature;
	for (map<nodeType,nodeInfo>::iterator it = s.begin() ; it != s.end() ; it++)//for all nodes of the graph
	{
		sort(it->second.signature->begin(), it->second.signature->end(), sigCmp);		
	}
}

/*
	the following two functions are not members of this class, nevertheless, 
	once called they take this object as parameter and call one of these classes memeber
	functions (in the thread they are)
*/
void* sendSignaturesThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;
	gr->sendSignatures();
	pthread_exit(NULL);
}

void* handleMessagesThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;//the this variable was passed in 
	gr->handleMessages();//call the handleMessages function in this new thread ! 
	pthread_exit(NULL);
}

void BisimilarGraphReducer::createThreadToSendSignaturesAndHandleMessages()
{	
	numberOfExpectedAnswers = s.size();//we will wait for #nodes signature ID's to be sent back
	//creating a thread for handling messages (see the algorithm) : 
	pthread_t sendSignatureThread;
	pthread_create(&sendSignatureThread, NULL, sendSignaturesThreadFunction, (void*)this);
	
	//creating a thread for handling messages (see the algorithm) : 
	pthread_t handleMessagesThread;
	pthread_create(&handleMessagesThread, NULL, handleMessagesThreadFunction, (void*)this);
	
	//wait for both threads to finish before proceeding 
	pthread_join(sendSignatureThread, NULL);
	pthread_join(handleMessagesThread, NULL);
}

void BisimilarGraphReducer::sendSignatures()
{	
	NonBlockingSendQueue< Signature > signaturesQueue(cluster,HASH_INSERT);
	for (map<nodeType,nodeInfo>::iterator it = s.begin() ; it != s.end() ; it++)
	{
		vector<Signature>* sig = it->second.signature;
		int clusterToSendTo = hashSignature(*sig);
		sig->push_back(Signature{0,it->first});
		signaturesQueue.sendVector(clusterToSendTo, sig, ((*sig).size())*sizeof(Signature), false);
	}
	cluster->sendSignalToAllClusterNodes(END_SIG);
}

int BisimilarGraphReducer::hashSignature(vector<Signature>& signature)
{
	int hash = 0;
	for (auto sig : signature)
	{
		hash += (int)(sig.a)%cluster->getNumberOfNodes() + (int)(sig.p)% cluster->getNumberOfNodes();
	}
	return (hash % (cluster->getNumberOfNodes()));
}


bool BisimilarGraphReducer::compareSignatureVector (const std::vector<Signature>* lhs,const std::vector<Signature>* rhs)
{
	const std::vector<Signature>& leftVec = (*lhs);
	const std::vector<Signature>& rightVec = (*rhs);
	int minSize = min(leftVec.size() , rightVec.size());
	for (int i = 0 ; i < minSize ; i++)
	{
		Signature left = leftVec[i];
		Signature right = rightVec[i];
		if (left.a < right.a)
		{
			return true;
		}
		if (left.a == right.a)
		{
			if (left.p < right.p)
			{
				return true;
			}
			if (left.p == right.p)
			{
				continue;
			}
			return false;
		}
		return false;
	}
	return leftVec.size() < rightVec.size();
}

void BisimilarGraphReducer::handleMessages()
{ 
	clearPartialHashTable();
	int numberOfActiveWorkers = cluster->getNumberOfNodes();
	int currentNumberOfBlocks = 0*100+cluster->getrankOfCurrentNode();//assuming we won't have a cluster of > 100 nodes!
	
	NonBlockingSendQueue<SignatureAnswer*> idResponseQueue(cluster,HASH_ID);
	
	tags tag = OUT;
	int count = 0;
	int source = 0;
	while (numberOfActiveWorkers > 0 || numberOfExpectedAnswers > 0)
	{
		unsigned char* data = cluster->receive(MPI_BYTE, &count, &source, (int *)&tag);
		if (tag == HASH_INSERT)
		{
			int blockNumberToReturnToTheSender = 0;
			Signature* sigs = (Signature*)data;
			vector<Signature>* signatureToInsert = new vector<Signature>(sigs,sigs+count/(sizeof(Signature)));
			blockType node = ((*signatureToInsert)[signatureToInsert->size()-1]).p;
			signatureToInsert->pop_back();
			std::pair<std::map<std::vector<Signature>*,int>::iterator , bool> ret;
			ret = H.insert( std::pair<std::vector<Signature>*,int>(signatureToInsert,currentNumberOfBlocks) );
			if (ret.second == false) //signature already existed
			{
				blockNumberToReturnToTheSender = ret.first->second;
				delete signatureToInsert;

			}else
			{
				blockNumberToReturnToTheSender = currentNumberOfBlocks;
				currentNumberOfBlocks += 100;
			}	
			SignatureAnswer* answer = new SignatureAnswer;
			answer->node = node;
			answer->blockID = blockNumberToReturnToTheSender;
			idResponseQueue.send(source,answer,sizeof(SignatureAnswer));
			delete[] data; //delete the received data object which at this point has been converted to a vector 			
		}else if (tag == END_SIG)
		{
			numberOfActiveWorkers--;
		}else if (tag == HASH_ID)
		{
			//clock_t begin = clock();
			SignatureAnswer* siganswer = (SignatureAnswer*)data;
			//cout << double(clock() - begin) / CLOCKS_PER_SEC  << " "  ;
			s[siganswer->node].id = siganswer->blockID;
			//cout << double(clock() - begin) / CLOCKS_PER_SEC  << " " << endl ;
			numberOfExpectedAnswers--;
			delete[] data;
		}
	}
	myNewCount = H.size();
}

void BisimilarGraphReducer::clearPartialHashTable()
{
	for (map<vector<Signature>*,int>::iterator it = H.begin(); it != H.end(); ++it)
	{
		delete it->first;
	}
	H.clear();
}

void* sendIDsThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;
	gr->sendNewIDs();
	pthread_exit(NULL);
}

void* receiveIDsThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;
	gr->receiveIDs();
	pthread_exit(NULL);
}

void BisimilarGraphReducer::updateIDs()
{
	
	pthread_t sendIDs;
	pthread_create(&sendIDs, NULL, sendIDsThreadFunction, (void*)this);
	
	pthread_t receiveIDs;
	pthread_create(&receiveIDs, NULL, receiveIDsThreadFunction, (void*)this);
	
	//wait for both threads to finish before proceeding 
	pthread_join(sendIDs, NULL);
	pthread_join(receiveIDs, NULL);
	
	
}

void BisimilarGraphReducer::sendNewIDs()
{
	NonBlockingSendQueue< blockType > inSendQueue(cluster,UPDATE);
	for (int j = 0 ; j < cluster->getNumberOfNodes() ; j++)
	{
		vector<In*> inj = in[j];
		vector<blockType>* toBeSent = new vector<blockType>;
		for (auto in : inj)//for all the In's going to j
		{
			toBeSent->push_back(s[in->dest].id);
		} 
		inSendQueue.sendVector(j, toBeSent, (toBeSent->size())*sizeof(blockType), true);
	}
}

void BisimilarGraphReducer::receiveIDs()
{
	int received = 0;
	tags tag = OUT;
	int count = 0;
	int source = 0;
	while (received < cluster->getNumberOfNodes())
	{
		unsigned char* data = cluster->receive(MPI_BYTE, &count, &source, (int *)&tag);
		if (tag == UPDATE)
		{
			blockType* newBlockNames = (blockType*)data;
			
			vector<Out*>& outij = out[source];//outij corresponding to the source of the message 
			for (int i = 0 ; i < outij.size() ; i++)
			{
				outij[i]->destinationBlock = newBlockNames[i];//replace the destination block with the new value 
			} 
			delete[] newBlockNames;
			received ++;	
		}
	}
}

void BisimilarGraphReducer::printIDs()
{
	for (map<nodeType,nodeInfo>::iterator it = s.begin() ; it != s.end() ; it++)//for all nodes of the graph
	{
		cout << it->first << " --> " << it->second.id << endl;
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
	for (map<nodeType,nodeInfo>::iterator it = s.begin() ; it != s.end() ; it++)//for all nodes of the graph
	{
		cout << "node "<< cluster->getrankOfCurrentNode() << " : " 
		 << "\thas graph node : " <<  it->first << endl;
	}
}








