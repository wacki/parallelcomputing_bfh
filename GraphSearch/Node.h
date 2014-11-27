
#pragma once
#include <boost/mpi.hpp>

#include <map>
#include <set>
#include <vector>
#include <string>
#include <boost/serialization/string.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

/*
todo: 

	1. finish this oo concept here based on what is already done in graphsearch.cpp
	2. make the local tree and leader election work

*/

namespace mpi = boost::mpi;
/*

struct Message
{
	enum Type {
		MT_TreeLeaderCandidate,		// the sender is proposing the node 'rank' as the new leader of the local tree
		MT_TreeLeaderFound,			// a new leader has been found for the local tree 'rank'
		MT_OutgoingEdgeCandidate,	// the sender is proposing the node 'rank' with the smallest outgoing edge of length 'val' as a new tree connection edge
		MT_OutgoingEdgeFound,		// the current tree has found a new connection frmo node 'rank' along its edge of size 'val'
		MT_EdgeAddedNotification	// the sender just added the receiver as a tree neighbour
	} type;

	int rank;
	int val;

	Message()
	{}

	Message(Type type, int rank, int val)
		: type(type), rank(rank), val(val)
	{ }


	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & type;
		ar & rank;
		ar & val;
	}
};


class GraphNode;


class Neighbour
{
public:
	Neighbour()
		: _origin(nullptr), _dest(-1), _cost(-1)
	{ }

	Neighbour(GraphNode* origin, int destination, int cost);

	// check if this neighbour sent any message to us
	bool poll(Message& message);

	// send a message to this neighbour
	void send(const Message& msg, bool block = false);

	bool operator<(const Neighbour& rhs) const
	{
		// a neighbour connection has higher priority (returns true here)
		// if it's cost is lower than the other's OR if costs are equal but 
		// it's rank is lower.
		return (_cost < rhs._cost) || (_cost == rhs._cost && _dest < rhs._dest);
	}

	int getDestination() const { return _dest; }
	int getCost() const { return _cost; }

protected:
	GraphNode* _origin;			// origin GraphNode (the rank owning this connection, needed for polling)
	int _dest;				// destination rank (the rank of the actual neighbour
	int _cost;				// edge cost for this connection
	mpi::request _req;		// requestblock that has an open irecv call
	Message _messageBuffer;	// buffer for incoming messages on this edge
};


// @todo 
class GraphNode
{
public:
	GraphNode(int rank)
		: _rank(rank)
	{ }

	void handleMessage(int sender, const Message& msg);
	void update();

	void buildMinSpanningTree();

	// add a neighbour connection to the initial graph
	void addNeighbour(int rank, int cost);

	int getRank() const { return _rank; }

	void testSend()
	{
		std::cout << "sending";
		Message msg(Message::MT_TreeLeaderFound, 2, 4);
		_neighbours[3].send(msg, true);
	}

	void testRec()
	{
		Message msg;
		if(_neighbours[0].poll(msg))
			std::cout << "success";
	}

	std::map<int, Neighbour>& testtest() { return _neighbours; }
protected:
	int _rank;						// rank of this node
	std::map<int, Neighbour> _neighbours;	// graph neighbour map for this node
	std::vector<int> _treeConnections;	// neighbour edges that belong to our tree




	// internal poll function that polls all neighbours


};
*/
class GraphCheapestEdgeMsg;


enum LogVerbosity {
    LV_Quiet,
    LV_Minimal,
    LV_Normal,
    LV_Detailed,
    LV_Diagnostic
};

class Log
{
public:
    Log(LogVerbosity verb, const std::string& fileAll, const std::string& fileSelf)
    {
        _verbosity = verb;
        _ofsAll.open(fileAll, std::ofstream::out | std::ofstream::app); // append to the general file
        _ofsSelf.open(fileSelf);
    }

    ~Log()
    {
        if (_ofsAll.is_open())
            _ofsAll.close();
        
        if (_ofsSelf.is_open())
            _ofsSelf.close();
    }

    void logMsg(LogVerbosity verb, const std::string& msg)
    {
        if (_verbosity < verb)
            return;

        std::cout << msg;
        std::cout.flush();
        _ofsAll << msg;
        _ofsAll.flush();
        _ofsSelf << msg;
        _ofsSelf.flush();
    }

private:
    LogVerbosity    _verbosity;
    std::ofstream   _ofsAll;
    std::ofstream   _ofsSelf;

};

class Node
{
public:
	Node(const std::string& logName);

	void print();
	virtual std::string toString();
    void logMsg(LogVerbosity verb, const std::string& msg)
    {
        std::ostringstream oss;
        oss << "[" << std::setw(2) << std::setfill('0') << world.rank() << "]: ";
        log.logMsg(verb, oss.str() + msg);
    }

protected:
	boost::mpi::communicator	world;
	boost::mpi::environment		env;
    Log log;
};



class TreeNode : public Node
{
public:
	TreeNode(const std::string& logName);

	virtual std::string toString();
	void addNeighbour(int rank);



    // todo make this process generic so that we can reuse it for election of min edge selection
    template<class MT>
    void electLeader(MT* myMessage);

    void insertNeighboursInSet(std::set<int>& neighbours) const
    {
        neighbours.insert(_neighbours.begin(), _neighbours.end());
    }

protected:
	std::vector<int> _neighbours;
};

struct GraphEdge
{
	int to;
	int cost;
};

class GraphNode : public Node
{
public:
	GraphNode(const std::string& logName);

	virtual std::string toString();
	void addEdge(int to, int cost);

    void baruvka();

protected:
	std::vector<GraphEdge> _neighbours;
};





// template impl

template<class MT>
void TreeNode::electLeader(MT* myMessage)
{
	std::vector<ConnectionObject<MT>*> neighbourRequests;
	// build neighbour requests
	for(auto neighbour : _neighbours)
		neighbourRequests.push_back(new ConnectionObject<MT>(neighbour));

	// current leader
	int requestsLeft = neighbourRequests.size();

    //logMsg(LV_Normal, "Starting leader election process.\n");
    //logMsg(LV_Diagnostic, "Remaining requests: " + std::to_string(_neighbours.size()) + "\n");
	// wait for leader elect messages 
	while(requestsLeft > 1)
	{
		for(auto& nr : neighbourRequests)
		{
			if(!nr->done() && nr->poll())
			{
				if(nr->msg() < (*myMessage)) {
                    myMessage->merge(nr->msg());
                    //logMsg(LV_Detailed, "Received a better candidate: " + myMessage->toString() + "\n");
				}
				requestsLeft--;
			}

		}
	}

	if(requestsLeft == 0) {

		//if(outputLog) std::cout << "[" << world.rank() << "] " << "Found leader (" << myMessage->minRank << "), notifying neighbours." << std::endl;
        //logMsg(LV_Minimal, "Found the leader: " + myMessage->toString() + "; distributing to neighbours.\n");
		// we know the leader
		for(auto neighbour : _neighbours)
			world.isend(neighbour, myMessage->tag(), *myMessage);
	}
	else
	{
        // we land here if we have only one neighbour left, if there are more then something went wrong.
        assert(requestsLeft == 1 && "Remaining request count higher than expected.");


		// send to the remaining neighbour (can only be one)
		ConnectionObject<MT>* remainingReq = nullptr;
		for(auto& nr : neighbourRequests) {
			if(!nr->done()) {
				remainingReq = nr;
				break;
			}
		}

		// notify remaining neighbour
		//if(outputLog) std::cout << "[" << world.rank() << "] " << " sending msg to " << remainingReq->rank() << " payload: " << myMessage->minRank << ", " << myMessage->edgeTo << ", " << myMessage->edgeCost << ", " << myMessage->minEdgeRank << ", " << std::endl;
		world.isend(remainingReq->rank(), myMessage->tag(), *myMessage);
        
        //logMsg(LV_Normal, "Only one remaining neighbour, sending candidate: " + myMessage->toString() + "\n");
		// wait for answers
		remainingReq->wait();

        
		// find better soluton
        if (remainingReq->msg() < (*myMessage)) {
            myMessage->merge(remainingReq->msg());
            //logMsg(LV_Diagnostic, "Received a better candidate from the last neighbour\n");
        }
       // if (outputLog) std::cout << "[" << world.rank() << "] message received: " << remainingReq->msg().minRank << ", " << remainingReq->msg().edgeTo << ", " << remainingReq->msg().edgeCost << ", " << remainingReq->msg().minEdgeRank << ", " << std::endl;
		//if(outputLog) std::cout << "[" << world.rank() << "] " << "Received election msg, leader is " << myMessage->minRank << std::endl;
        

		//if(outputLog) std::cout << "[" << world.rank() << "] " << "Found leader (" << myMessage->minRank << "), notifying neighbours." << std::endl;
		// notify all other neighbours
        
    //logMsg(LV_Diagnostic, "Notifying all neighbours about result.\n");
		for(auto neighbour : _neighbours)
			if(neighbour != remainingReq->rank())
				world.isend(neighbour, myMessage->tag(), *myMessage);
	}

    
    //logMsg(LV_Minimal, "Leader election finished, result: " + myMessage->toString() + "\n");
}