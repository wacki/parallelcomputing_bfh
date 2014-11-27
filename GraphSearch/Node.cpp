
#include "Windows.h"
#include "Node.h"
#include "Msg.h"


void logMsg(LogVerbosity verb, const std::string& msg)
{

}


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
    oss << "TreeNode [" << world.rank() << "] neighbours: ";
    for (auto i : _neighbours)
        oss << "(" << i << ") ";

	return oss.str();
}

void TreeNode::addNeighbour(int rank)
{
	if(std::find(_neighbours.begin(), _neighbours.end(), rank) != _neighbours.end())
		return;

	_neighbours.push_back(rank);
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
	GraphEdge edge;
	edge.to = to;
	edge.cost = cost;
	_neighbours.push_back(edge);
}

void GraphNode::baruvka()
{
    // keep track of current status
    TreeNode mstNode;          // mst node for this graph node
    //std::vector<GraphEdge> mstNodes; // candidate nodes
	std::map<int, int> mstNodes; // map containing <destination, cost> for mst candidate nodes
	std::vector<ConnectionObject<MSTGrowMsg>*> candidateRequests;

	for(auto& neighbour : _neighbours) {
		mstNodes[neighbour.to] = neighbour.cost;	// candidate neighbours to be added to the mst
		// create a request object for all candidates
		//candidateRequests.push_back(new ConnectionObject<MSTGrowMsg>(neighbour.to, false));
	}

	// print current candidates 
	std::cout << "[" << world.rank() << "] " << "Possible MST candidates: ";
	for(auto edge : mstNodes)
		std::cout << "(to: " << edge.first << ", cost: " << edge.second << ") ";
	std::cout << std::endl;
    
    
    while (true)    {
        // 1. find edge candidates for current node and add it to the msg
        TreeLeaderElectMsg msg;
        GraphEdge minEdge;
        minEdge.to = INT_MAX;
        minEdge.cost = INT_MAX;
        int minEdgeRank = INT_MAX;
        for (auto candidate : mstNodes) {
			int dest = candidate.first;
			int cost = candidate.second;
			if(cost < minEdge.cost ||
			   (cost == minEdge.cost) && dest < minEdge.to) {
				minEdge.cost = cost;
				minEdge.to = dest;
                minEdgeRank = min(dest, world.rank()); // get the min rank of the two nodes forming this edge
			}
        }
		// no candidate was found
		if(minEdge.to == INT_MAX) {

			std::cout << "[" << world.rank() << "] " << "No suitable candidate found" << std::endl;
		}
        else {
            std::cout << "[" << world.rank() << "] " << "candidate node found: (to: " << minEdge.to << ", cost: " << minEdge.cost << ")" << std::endl;
        }

        msg.minRank = world.rank();
        msg.minEdgeRank = minEdgeRank;
        msg.edgeTo = minEdge.to;
        msg.edgeCost = minEdge.cost;

        std::cout << "[" << world.rank() << "] current tree: " << mstNode.toString() << std::endl;

        // 2. run leader election to find the best edge in our mst
        mstNode.electLeader<TreeLeaderElectMsg>(&msg);
        
        // break if we didn't find anything
		if(msg.edgeTo == INT_MAX)
			break;

		std::cout << "[" << world.rank() << "] " << "BEST MST EDGE: " << msg.minRank << " -> " << msg.edgeTo << "(" << msg.edgeCost << ")" << std::endl;
        
		// leader election found no more edge, exit
        
                
        // 3. if our node won the election send a notification to the neighbour that was selected
        if (msg.minRank == world.rank()) {
            MSTGrowMsg growMsg;

            if (msg.minEdgeRank == world.rank()) {
			    std::cout << "[" << world.rank() << "] " << "Sending grow msg to: " << msg.edgeTo << std::endl;
                world.send(msg.edgeTo, growMsg.tag(), growMsg);
                //world.recv(msg.edgeTo, growMsg.tag(), growMsg); // we have to wait for an 'ack' response to go on with it
            }
            else {
			    std::cout << "[" << world.rank() << "] " << "Waiting for grow msg from: " << msg.edgeTo << std::endl;
                world.recv(msg.edgeTo, growMsg.tag(), growMsg); // we have to wait for an 'ack' response to go on with it
                
                std::cout << "[" << world.rank() << "] " << "successfully received grow msg from " << msg.edgeTo << std::endl;
                //world.send(msg.edgeTo, growMsg.tag(), growMsg);
                //std::cout << "[" << world.rank() << "] " << "successfully received grow msg from " << msg.edgeTo << std::endl;
                //if (world.rank() == 4) std::cout << "\n\n---------------------------------------" << std::endl;
            }

            mstNode.addNeighbour(msg.edgeTo); // also add the found node to the mst neighbour list
			mstNodes.erase(msg.edgeTo); // remove the new connection as possible candidate
        }
		
        // update MST node list
        CollectMstNodesMsg collectMsg;
        mstNode.insertNeighboursInSet(collectMsg.nodes);
        mstNode.electLeader<CollectMstNodesMsg>(&collectMsg);

		// listen to incoming grow messages
		/*for(auto req : candidateRequests) {
			if(!req->done() && req->poll()) { // todo, find a better way to remove request objects, need the same amount as candidates
				mstNode.addNeighbour(req->rank());
				mstNodes.erase(req->rank()); // remove the new connection as possible candidate

			    std::cout << "[" << world.rank() << "] " << "received new connection from " << req->rank() << std::endl;
			}				
		}*/
		// update candidate list based on list in the msg
		for(auto connected : collectMsg.nodes) {
			if(mstNodes.find(connected) != mstNodes.end()) {
				mstNodes.erase(connected);	// remove nodes that were encountered during the previous leader election as possible candidates
				std::cout << "[" << world.rank() << "] " << "removing " << connected << " as possible candidate since it's already in our MST" << std::endl;
			}
		}
        //Sleep(1000);
        // else just update our viable candidates based on the current mst
            // g oover the nodes list in the msg and remove all the nodes in there that are the same


        // 4. listen to incoming connect messages and update the list if necessary
        
    }

    // print out result
        std::cout << "[" << world.rank() << "] current tree: " << mstNode.toString() << std::endl;
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