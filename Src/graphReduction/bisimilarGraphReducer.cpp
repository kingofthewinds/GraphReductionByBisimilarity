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
	GraphDistributor gd(cluster,path,out,in,tempSet); // distribute the graph 
	//convert the temporary set to a vector because we need it's values elements not to change position anymore
	//under any more circomstances and it is guaranteed by vectors : 
	for (set<nodeType>::const_iterator i = tempSet.begin() ; i != tempSet.end() ; i++) 
	{
		s.push_back(*i);
	}
	
	//set the size the signatures vector (it will keep a signature per node attributed to cluster node)
	signatures.resize(s.size());
	ID.resize(s.size());//same for the ID's vector 
	for (auto id : ID) id = 0;//set the initial id's to zero 
	cluster->waitForOtherClusterNodes();//we wait for all nodes to get here before running the algorithm
	runAlgorithm();
}

void BisimilarGraphReducer::runAlgorithm()
{
	int newCount = 1;
	
	while(true)
	{
		for (int count = 0 ; count < s.size() ; count++)//for all nodes of the graph
		{
			vector<Signature>* sig = generateSignature(s[count]);//generate their signature 
			delete signatures[count];
			signatures[count] = sig;//save the generated signature
		}
		numberOfExpectedAnswers = 0;
		createThreadToSendSignaturesAndHandleMessages();
		int oldCount = newCount;
		cluster->waitForOtherClusterNodes();
		newCount = cluster->sumAllClusterNodeValues(myNewCount);
		if (oldCount == newCount)
		{
			break;
		}
		cluster->waitForOtherClusterNodes();
		updateIDs();
		cluster->waitForOtherClusterNodes();
	}
	printIDs();
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

std::vector<Signature>* BisimilarGraphReducer::generateSignature(nodeType node)
{
	vector<Signature>* signature = new vector<Signature>;
	
	//we create a set that uses our signature compare function to order elements 
	//the need for this ordering is to ease the comparison of signatures when inserting 
	//them into the hash table 
	bool(*sigCmp)(const Signature&,const Signature&) = BisimilarGraphReducer::compareSignature;
	std::set<Signature,bool(*)(const Signature&,const Signature&)> sigSet (sigCmp);
	
	//compute signature 
	for (auto oj : out)
	{
		for (auto o : oj)
		{
			if (o->source == node)//if we have an out with the source = the node signature of which were calculating 
			{
				sigSet.insert((struct Signature){o->edge,o->destinationBlock});
			}
		}
	}
	
	//convert our set to a vector so that it will be saved contiguously and be able to be sent 
	//using out cluster
	for (set<Signature>::const_iterator i = sigSet.begin() ; i != sigSet.end() ; i++) 
	{
		signature->push_back(*i);
	}
	return signature;
}

/*
	the following two functions are not members of this class, nevertheless, 
	once calles they take this object as parameter and call one of these classes memeber
	functions (in the thread they are)
*/
void* handleMessagesThreadFunction(void* arg)
{
	BisimilarGraphReducer* gr = (BisimilarGraphReducer*)arg;//the this variable was passed in 
	gr->handleMessages();//call the handleMessages function in this new thread ! 
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
	numberOfExpectedAnswers = signatures.size();//we will wait for #nodes signature ID's to be sent back
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
	
	for (vector<Signature>* sig : signatures)
	{
		int clusterToSendTo = hashSignature(*sig);
		signaturesQueue.sendVector(clusterToSendTo, sig, ((*sig).size())*sizeof(Signature), false);
	}
	signaturesQueue.waitAndFreeVector();
	cluster->sendSignalToAllClusterNodes(END_SIG);
}

int BisimilarGraphReducer::hashSignature(vector<Signature>& signature)
{
	int hash = 0;
	for (auto sig : signature)
	{
		hash += (int)(sig.a) + (int)(sig.p);//vectors with the same a and p will go to the same cluster ! 
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
	
	NonBlockingSendQueue<Signature> idResponseQueue(cluster,HASH_ID);
	
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
			//insert into hash map
			vector<Signature>* signatureToInsert = new vector<Signature>(sigs,sigs+count/(sizeof(Signature)));
			std::pair<std::map<std::vector<Signature>*,int>::iterator , bool> ret;
			ret = H.insert( std::pair<std::vector<Signature>*,int>(signatureToInsert,currentNumberOfBlocks) );
			if (ret.second == false) //signature already existed
			{
				blockNumberToReturnToTheSender = ret.first->second;
				//for facility, we add the new signature as the last element of the signature we're sending back 
				signatureToInsert->push_back( (struct Signature){'n',blockNumberToReturnToTheSender} );
				idResponseQueue.sendVector(source,signatureToInsert,(signatureToInsert->size())*sizeof(Signature),true);
			}else
			{
				blockNumberToReturnToTheSender = currentNumberOfBlocks;
				currentNumberOfBlocks += 100;
				
				vector<Signature>* signatureToSendBack = new vector<Signature>(sigs,sigs+count/(sizeof(Signature)));
				signatureToSendBack->push_back( (struct Signature){'n',blockNumberToReturnToTheSender} );
				idResponseQueue.sendVector(	source,signatureToSendBack, 
											(signatureToSendBack->size())*sizeof(Signature),true);
				
			}			
			delete[] data; //delete the received data object which at this point has been converted to a vector 			
		}else if (tag == END_SIG)
		{
			numberOfActiveWorkers--;
		}else if (tag == HASH_ID)
		{
			Signature* sigs = (Signature*)data;
			vector<Signature>* signatureToInsert = new vector<Signature>(sigs,sigs+count/(sizeof(Signature)));
			//retrieve the last element of the signature which nontains the new id in it's p value 
			blockType newID = ((*signatureToInsert)[(*signatureToInsert).size()-1]).p;
			signatureToInsert->pop_back();//delete this now useless last element
			
			//search whithin all the signatures to find which one it is equal to 
			for (int i = 0 ; i < signatures.size() ; i++)
			{
				if ( 
					BisimilarGraphReducer::compareSignatureVector(signatures[i],signatureToInsert)
					==
					BisimilarGraphReducer::compareSignatureVector(signatureToInsert,signatures[i])  
				)
				{
					ID[i] = newID;//set the id of the corresponding node 
				}
			}
			delete signatureToInsert;
			numberOfExpectedAnswers--;
			delete[] data;
		}
	}
	idResponseQueue.waitAndFreeVector();
	myNewCount = H.size();
}

void BisimilarGraphReducer::clearPartialHashTable()
{
	H.clear();
	for (map<vector<Signature>*,int>::iterator it = H.begin(); it != H.end(); ++it)
	{
		delete it->first;
	}
}

void BisimilarGraphReducer::updateIDs()
{
	NonBlockingSendQueue< blockType > inSendQueue(cluster,UPDATE);
	for (int j = 0 ; j < cluster->getNumberOfNodes() ; j++)
	{
		vector<In*> inj = in[j];
		vector<blockType>* toBeSent = new vector<blockType>;
		for (auto in : inj)//for all the In's going to j
		{
			int pos = find(s.begin(), s.end(), in->dest ) - s.begin();//if a node = destination of the in 
			toBeSent->push_back(ID[pos]);//find it's new Id and send it 
		} 
		inSendQueue.sendVector(j, toBeSent, (toBeSent->size())*sizeof(blockType), true);
	}
	
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
	inSendQueue.waitAndFreeVector();
}

void BisimilarGraphReducer::printIDs()
{
	for (int i = 0 ; i < s.size() ; i++)
	{
		cout << s[i] << " --> " << ID[i] << endl;
	}
}








