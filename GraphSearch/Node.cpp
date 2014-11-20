
#include "Windows.h"
#include "Node.h"
#include "Msg.h"


Node::Node()
{

}

void Node::print()
{
	std::cout << "[NODE " << world.rank() << "] " << toString() << std::endl;
}

std::string Node::toString()
{
	std::ostringstream oss;
	oss << "Base node class";
	return oss.str();
}

TreeNode::TreeNode()
{

}

std::string TreeNode::toString()
{
	std::ostringstream oss;
	oss << "Tree Node";
	return oss.str();
}

void TreeNode::addNeighbour(int rank)
{
	_neighbours.push_back(rank);
}

// todo make this process generic so that we can reuse it for election of min edge selection
void TreeNode::electLeader()
{
	// todo make le work with only one message type
	struct NeighbourRequest
	{
		int rank;
		bool gotMsg;
		TreeLeaderElectMsg msg;
		mpi::request req;
	};

	std::vector<NeighbourRequest> neighbourRequests(_neighbours.size());
	// build neighbour requests
	for(int i = 0; i < _neighbours.size(); i++)
	{
		NeighbourRequest& nr = neighbourRequests[i];
		nr.rank = _neighbours[i];
		nr.gotMsg = false;
		nr.req = world.irecv(nr.rank, nr.msg.tag(), nr.msg);
	}

	// current leader
	int requestsLeft = neighbourRequests.size();
	int currentLeader = world.rank();
	TreeLeaderElectMsg myMessage;
	myMessage.minRank = currentLeader;

	std::cout << "[" << world.rank() << "] " << "Starting leader election." << std::endl;
	std::cout << "[" << world.rank() << "] " << "remaining requests: " << _neighbours.size() << "." << std::endl;


	// wait for leader elect messages 
	while(requestsLeft > 1)
	{
		for(auto& nr : neighbourRequests)
		{
			if(!nr.gotMsg && nr.req.test().get_ptr() != NULL)
			{

				nr.gotMsg = true;
				if(nr.msg.minRank < myMessage.minRank) {
					std::cout << "[" << world.rank() << "] " << "received a BETTER candidate." << std::endl;
					myMessage.minRank = nr.msg.minRank;
				}
				else {
					std::cout << "[" << world.rank() << "] " << "received a WORSE candidate." << std::endl;
				}
				requestsLeft--;
			}
			else
				std::cout << "[" << world.rank() << "] " << "POLLING " << nr.rank << std::endl;

		}

		Sleep(1000);
	}

	if(requestsLeft == 0) {

		std::cout << "[" << world.rank() << "] " << "Found leader (" << myMessage.minRank << "), notifying neighbours." << std::endl;
		// we know the leader
		for(auto neighbour : _neighbours)
			world.isend(neighbour, myMessage.tag(), myMessage);
	}
	else
	{

		std::cout << "[" << world.rank() << "] " << "Only " << requestsLeft << " remaining neighbours, sending candidate" << std::endl;

		// send to the remaining neighbour (can only be one)
		NeighbourRequest* remainingReq = nullptr;
		for(auto& nr : neighbourRequests) {
			if(!nr.gotMsg) {
				remainingReq = &nr;
				break;
			}
		}

		// notify remaining neighbour

		std::cout << "[" << world.rank() << "] " << " sending msg to " << remainingReq->rank << std::endl;
		world.isend(remainingReq->rank, myMessage.tag(), myMessage);

		// wait for answers
		remainingReq->req.wait();


		std::cout << "[" << world.rank() << "] " << "Received election msg, leader is " << std::endl;
		// find better soluton
		if(remainingReq->msg.minRank < myMessage.minRank)
			myMessage.minRank = remainingReq->msg.minRank;

		
		std::cout << "[" << world.rank() << "] " << "Found leader (" << myMessage.minRank << "), notifying neighbours." << std::endl;
		// notify all other neighbours
		for(auto neighbour : _neighbours)
			if(neighbour != remainingReq->rank)
				world.isend(neighbour, myMessage.tag(), myMessage);
	}

	/*
	while(true)
	{
		// we received from all neighbours except one
		if(remainingNeighbours.size() == 1) {
			Message msg(Message::MT_Candidate, currentLeader, currentLeaderToken);
			printMsg("single remaining neighbour (" + std::to_string(remainingNeighbours[0]) + ") sending msg");
			world.send(remainingNeighbours[0], 0, msg);
		}
		// we received from everyone we know the leader
		else if(remainingNeighbours.size() == 0) {
			for(int neighbour : neighbours) {
				// TODO: dont always send to all neighbours
				printMsg("found a leader, sending election msg " + std::to_string(currentLeader));
				Message msg(Message::MT_LeaderFound, currentLeader, currentLeaderToken);
				world.isend(neighbour, 0, msg);
			}
			break;
		}

		// poll all neighbours
		for(int neighbour : remainingNeighbours) {

			// TODO: can we do it without the lock here? if two remain then they could get locked here
			if(remainingNeighbours.size() == 1) {
				printMsg("one neighbour remaining, waiting for answer");
				requests[neighbour].wait();
			}
			// skip if we didn't receive anything on the poll
			else if(requests[neighbour].test().get_ptr() == NULL) {
				printMsg("polling " + std::to_string(neighbour) + " without success");
				continue;
			}
			else
				printMsg("POLLING SUCCESS");


			const Message& msg = recvBuffer[neighbour];
			// received a message
			if(msg.token < currentLeaderToken || msg.token == currentLeaderToken && msg.leader < currentLeader) {
				printMsg("received a better candidate than current");
				currentLeader = msg.leader;
				currentLeaderToken = msg.token;
			}

			// we only ever receive one message
			// todo: we could solve this problem by using a copy of remainingneighbours...
			requests.erase(neighbour);
			remainingNeighbours.erase(std::remove(remainingNeighbours.begin(), remainingNeighbours.end(), neighbour), remainingNeighbours.end());
			break;
		}

		Sleep(1000);
	}*/
}



GraphNode::GraphNode()
{

}

std::string GraphNode::toString()
{
	std::ostringstream oss;
	oss << "GraphNode class";
	return oss.str();
}

void GraphNode::addEdge(int to, int cost)
{

}

void GraphNode::findMST()
{

}





/*

Neighbour::Neighbour(GraphNode* origin, int destination, int cost)
: _origin(origin), _dest(destination), _cost(cost)
{ 
	// set up a request block to listen to incoming messages from this neighbour
	std::cout << "initializing request";
	_req = gWorld.irecv(_dest, 0, _messageBuffer);
}

bool Neighbour::poll(Message& msg)
{
	assert(_origin != nullptr && "trying to poll on an illegal node.");

	// poll the requestblock
	if(_req.test().get_ptr() == NULL) {
		std::cout << std::to_string(_origin->getRank()) + " polling " + std::to_string(_dest) + " without success.\n";
		return false;
	}

	// we received a msg
	msg = _messageBuffer;
	std::cout << std::to_string(_origin->getRank()) + " polling " + std::to_string(_dest) + ". received msg id: " + std::to_string(msg.type) + "\n";

	// generate new request block for this neighbour
	_req = gWorld.irecv(_dest, 0, _messageBuffer);

	return true;
}

void Neighbour::send(const Message& msg, bool block)
{
	if(block)
		gWorld.send(_dest, 0, msg);
	else
		gWorld.isend(_dest, 0, msg);
}
//_------------------------


void GraphNode::handleMessage(int sender, const Message& msg)
{
	switch(msg.type) {
	case Message::MT_TreeLeaderCandidate: 
		
		break;
	case Message::MT_TreeLeaderFound: 
		
		break;
	case Message::MT_OutgoingEdgeCandidate: 
		
		break;
	case Message::MT_OutgoingEdgeFound: 
		
		break;
	case Message::MT_EdgeAddedNotification:

		break;
	}

	std::cout << "received a message from " << sender << std::endl;
}


void GraphNode::addNeighbour(int rank, int cost)
{
	if(_neighbours.find(rank) != _neighbours.end())
		std::cout << "\n\ntrying to add same neighbour twice\n\n";
	assert(_neighbours.find(rank) == _neighbours.end() && "trying to add the same neighbour twice");
	_neighbours.insert(std::pair<int, Neighbour>(rank, Neighbour(this, rank, cost)));
}*/