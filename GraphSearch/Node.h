
#pragma once
#include <boost/mpi.hpp>

#include <map>
#include <vector>
#include <boost/serialization/string.hpp>


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
class TreeLeaderElectMsg;


class Node
{
public:
	Node();

	void print();
	virtual std::string toString();

protected:
	boost::mpi::communicator	world;
	boost::mpi::environment		env;
};



class TreeNode : public Node
{
public:
	TreeNode();

	virtual std::string toString();
	void addNeighbour(int rank);

	void electLeader(TreeLeaderElectMsg* myMessage);

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
	GraphNode();

	virtual std::string toString();
	void addEdge(int to, int cost);

    void baruvka();

protected:
	std::vector<GraphEdge> _neighbours;
};