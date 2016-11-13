#include "bisimilarGraphReducer.hpp"

using namespace std;

BisimilarGraphReducer::BisimilarGraphReducer(ClusterHandler& clusterHandler,std::string path):
	cluster(clusterHandler),
	out(clusterHandler->getNumberOfNodes()), 
	in(clusterHandler->getNumberOfNodes()),
	s(), H(BisimilarGraphReducer::compareSignatureVector) 
{
	std::set<nodeType> tempSet;
	GraphDistributor gd(cluster,path,out,in,tempSet);
	for (set<nodeType>::const_iterator i = tempSet.begin() ; i != tempSet.end() ; i++) 
	{
		s.push_back(*i);
	}
	signatures.resize(s.size());
	ID.resize(s.size());
	for (auto id : ID) id = 0;
	cluster->waitForOtherClusterNodes();
	runAlgorithm();
}

void BisimilarGraphReducer::runAlgorithm()
{
	int newCount = 1;
	
	while(true)
	{
		for (int count = 0 ; count < s.size() ; count++)
		{
			vector<Signature>* sig = generateSignature(s[count]);
			delete signatures[count];
			signatures[count] = sig;
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
	
	bool(*sigCmp)(const Signature&,const Signature&) = BisimilarGraphReducer::compareSignature;
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
	numberOfExpectedAnswers = signatures.size();
	pthread_t sendSignatureThread;
	pthread_create(&sendSignatureThread, NULL, sendSignaturesThreadFunction, (void*)this);
	
	pthread_t handleMessagesThread;
	pthread_create(&handleMessagesThread, NULL, handleMessagesThreadFunction, (void*)this);
	
	pthread_join(sendSignatureThread, NULL);
	pthread_join(handleMessagesThread, NULL);
}

void BisimilarGraphReducer::sendSignatures()
{
	NonBlockingSendQueue< Signature* > signaturesQueue(cluster,HASH_INSERT,MPI_BYTE);
	
	for (vector<Signature>* sig : signatures)
	{
		int clusterToSendTo = hashSignature(*sig);
		vector<Signature>* copySignature = new vector<Signature>((*sig).data(),(*sig).data()+(*sig).size());//otherwise sendqueue deletes it!
		signaturesQueue.send(clusterToSendTo,(*copySignature).data(),((*sig).size())*sizeof(Signature));
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
	int currentNumberOfBlocks = 0*100+cluster->getrankOfCurrentNode(); //assuming we won't have a cluster of more than 100 nodes!
	NonBlockingSendQueue< Signature* > idResponseQueue(cluster,HASH_ID,MPI_BYTE);
	while (numberOfActiveWorkers > 0 || numberOfExpectedAnswers > 0)
	{
		tags tag = OUT;
		int count = 0;
		int source = 0;
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
			}else
			{
				blockNumberToReturnToTheSender = currentNumberOfBlocks;
				currentNumberOfBlocks += 100;
			}
			//return id (returned as the "p" element of the last element of the vector!)
			vector<Signature>* signatureToSendBack = new vector<Signature>(sigs,sigs+count/(sizeof(Signature)));
			signatureToSendBack->push_back( (struct Signature){'n',blockNumberToReturnToTheSender} );
			idResponseQueue.send(source,(*signatureToSendBack).data(),((*signatureToSendBack).size())*sizeof(Signature));
			delete data;			
		}else if (tag == END_SIG)
		{
			numberOfActiveWorkers--;
		}else if (tag == HASH_ID)
		{
			Signature* sigs = (Signature*)data;
			vector<Signature>* signatureToInsert = new vector<Signature>(sigs,sigs+count/(sizeof(Signature)));
			blockType newID = ((*signatureToInsert)[(*signatureToInsert).size()-1]).p;
			signatureToInsert->pop_back();
			for (int i = 0 ; i < signatures.size() ; i++)
			{
				
				if ( 
					BisimilarGraphReducer::compareSignatureVector(signatures[i],signatureToInsert)
					==
					BisimilarGraphReducer::compareSignatureVector(signatureToInsert,signatures[i])  
				)
				{
					ID[i] = newID;
				}
			}
			delete signatureToInsert;
			numberOfExpectedAnswers--;
		}
	}
	idResponseQueue.waitAndFree();
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
	NonBlockingSendQueue< blockType* > inSendQueue(cluster,UPDATE,MPI_BYTE);
	for (int j = 0 ; j < cluster->getNumberOfNodes() ; j++)
	{
		vector<In*> inj = in[j];
		vector<blockType>* toBeSent = new vector<blockType>;
		for (auto in : inj)
		{
			int pos = find(s.begin(), s.end(), (*in).dest ) - s.begin();
			toBeSent->push_back(ID[pos]);
		} 
		inSendQueue.send(j,(*toBeSent).data(),((*toBeSent).size())*sizeof(blockType));
	}
	
	int received = 0;
	while (received < cluster->getNumberOfNodes())
	{
		tags tag = OUT;
		int count = 0;
		int source = 0;
		unsigned char* data = cluster->receive(MPI_BYTE, &count, &source, (int *)&tag);
		if (tag == UPDATE)
		{
			blockType* newBlockNames = (blockType*)data;
			
			vector<Out*>& outij = out[source];
			for (int i = 0 ; i < outij.size() ; i++)
			{
				outij[i]->destinationBlock = newBlockNames[i];
			} 
			
			delete newBlockNames;
			received ++;	
		}
	}
	inSendQueue.waitAndFree();
}

void BisimilarGraphReducer::printIDs()
{
	for (int i = 0 ; i < s.size() ; i++)
	{
		cout << s[i] << " --> " << ID[i] << endl;
	}
}








