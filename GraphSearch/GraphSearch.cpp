
/*

TODO:
	ok if we dont print with endl then mpi wont output that shit... wtf

	1. test the message sending and polling again and then implement the algorithm based on the slides



*/




/*
#include <boost/mpi.hpp>

#include <Windows.h>
#include <iostream>
#include <string>
#include <map>
#include <boost/serialization/string.hpp>

#include "common.h"


namespace mpi = boost::mpi;

mpi::environment		gEnv;
mpi::communicator		gWorld;

void printMsg(const std::string& msg)
{
	std::cout << "[P " << "]: " << msg << std::endl;
}

int main()
{
	// mpi stuff


	int recvMsg;


	// initialize request blocks
	mpi::request req = gWorld.irecv(0, 0, recvMsg);
	

	while(true)
	{
		printMsg("hello");

		Sleep(1000);
	}

	return 0;
}



*/







#include <boost/mpi.hpp>

#include <Windows.h>
#include <iostream>
#include <string>
#include <map>
#include <boost/serialization/string.hpp>

#include "Node.h"

namespace mpi = boost::mpi;

mpi::environment		gEnv;
mpi::communicator		gWorld;

// static world and env variables

void printMsg(const std::string& msg)
{
	std::cout << "[P " << "]: " << msg << std::endl;
}

// Forced sleep time after each tick per process in MS (set 0 to turn off sleep)
const int tickSleepTime = 1000;

// our default graph:   
//         [0]    [1]             [2]    
//        /  |   /    \          /  |
//      7   16  2      10      13   9
//     /     | /          \   /     |
//  [3]__3__[4]__5__[5]_18_[6]__12__[7] 
//     \     | \     |    / \        \
//      1   15  6    17  4   19      14
//        \  |    \  | /      \        \ 
//          [8]_13__[9]__8__[10]__11__[11]


//  weight matrix 
// (similar to adjacency matrix but with the
// following meaning: 
// if (m[i][k] == 0) --> no edge between node i and node k
// else there is an edge with weight  = m[i][k]

std::vector<std::vector<int>> admat = {
	// 0   1   2   3   4   5   6   7   8   9  10  11
	{  0, 0, 0, 7, 16, 0, 0, 0, 0, 0, 0, 0}, // 0
	{ 0, 0, 0, 0, 2, 0, 10, 0, 0, 0, 0, 0 }, // 1
	{ 0, 0, 0, 0, 0, 0, 13, 9, 0, 0, 0, 0 }, // 2
	{ 7, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0 }, // 3 
	{ 16, 2, 0, 3, 0, 5, 0, 0, 15, 6, 0, 0 }, // 4
	{ 0, 0, 0, 0, 5, 0, 18, 0, 0, 17, 0, 0 }, // 5
	{ 0, 10, 13, 0, 0, 18, 0, 12, 0, 4, 19, 0 }, // 6
	{ 0, 0, 9, 0, 0, 0, 12, 0, 0, 0, 0, 14 }, // 7
	{ 0, 0, 0, 1, 15, 0, 0, 0, 0, 13, 0, 0 }, // 8
	{ 0, 0, 0, 0, 6, 17, 4, 0, 13, 0, 8, 0 }, // 9
	{ 0, 0, 0, 0, 0, 0, 19, 0, 0, 8, 0, 11 }, //10
	{ 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 11, 0 }, //11
};

// temporary incidentlist for tree leader elect algo
// incident list
// 
//   0     1
//   |     |
// 2-3-4   5-6-7
//   |     |
//   8     9
// 
std::vector<std::vector<int>> incidentList = {
	{ 3 },			// 0
	{ 5 },			// 1
	{ 3 },			// 2
	{ 0, 2, 4, 8 },	// 3
	{ 3 },			// 4
	{ 1, 6, 9 },	// 5
	{ 5, 7 },		// 6
	{ 6 },			// 7
	{ 3 },			// 8
	{ 5 }			// 9
};

int main()
{
	/*/
	// early out if the number of processes doesn't match the adjecency list
	if(gWorld.size() != admat.size())
	{
	if(gWorld.rank() == 0)
	std::cout << "Number of processes doesn't match the adjacency list, please use -n " << admat.size() << std::endl;
	return 1;
	}


	// initialize the local graph node
	//GraphNode localNode(gWorld.rank());
	// add neighbour connections

	// fill outgoing edges list
	for(int i = 0; i < admat[localNode.getRank()].size(); ++i) {
	int cost = admat[localNode.getRank()][i];
	if(cost > 0) {
	localNode.addNeighbour(i, cost);
	}
	}

	// mpi stuff


	std::cout << "initialized: " << gWorld.rank() << "\n";

	bool sent = false;
	int count = 0;
	while(true)
	{
	std::cout << "[P " << "]: " << "hello" << std::endl;

	Sleep(1000);
	}

	return 0;

	// 1. find minimum spanning tree on the graph

	// 2. broadcast the graph structure so that each process has full information
	// @todo ...

	// 3. run a path finding algorithm in parallel or something...
	// @todo ...



	return 0;*/

	boost::mpi::environment		env;
	boost::mpi::communicator	world;


	if(world.size() != incidentList.size())
	{
		if(world.rank() == 0)
			std::cout << "Number of processes doesn't match the adjacency list, please use -n " << incidentList.size() << std::endl;
		return 1;
	}

	TreeNode treeNode;

	// fill the tree nodes neighbour struct
	for(auto neighbour : incidentList[world.rank()])
		treeNode.addNeighbour(neighbour);

	treeNode.electLeader();

	//TreeNode treeNode;
	//treeNode.leaderElect();




	return 0;
}




//
//#include <boost/mpi.hpp>
//
//#include <Windows.h>
//#include <iostream>
//#include <string>
//#include <map>
//#include <boost/serialization/string.hpp>
//
//#include "Node.h"
//
//namespace mpi = boost::mpi;
//
//
//// struct for keeping track of neighboring nodes and the edge cost that connects us with them
//struct EdgeConnection
//{
//	EdgeConnection(int node, int c)
//		: node(node), cost(c)
//	{ }
//
//	int node;
//	int cost;
//};
//
//
//struct Message
//{
//	enum Type {
//		MT_TreeLeaderCandidate,		// the sender is proposing the node 'rank' as the new leader of the local tree
//		MT_TreeLeaderFound,			// a new leader has been found for the local tree 'rank'
//		MT_OutgoingEdgeCandidate,	// the sender is proposing the node 'rank' with the smallest outgoing edge of length 'val' as a new tree connection edge
//		MT_OutgoingEdgeFound,		// the current tree has found a new connection frmo node 'rank' along its edge of size 'val'
//		MT_EdgeAddedNotification	// the sender just added the receiver as a tree neighbour
//	} type;
//
//	int rank;
//	int val;
//
//	Message()
//	{}
//
//	Message(Type type, int rank, int val)
//		: type(type), rank(rank), val(val)
//	{ }
//
//
//	template<class Archive>
//	void serialize(Archive& ar, const unsigned int version)
//	{
//		ar & type;
//		ar & rank;
//		ar & val;
//	}
//};
//
//mpi::environment	env;
//mpi::communicator	world;
//// local rank
//int					rank;
//// current tree leader
//int					treeLeader;
//// list of all outgoing edges including cost
//std::vector<EdgeConnection>	outgoingEdges;
//// list to keep track of known tree neighbours
//std::vector<int>	treeNeighbours;
//// list to keep track of remaining non tree neighbours
//std::vector<int>	nonTreeNeighbours;
//// request objects for all neighbours
//std::map<int, mpi::request> neighbourRequests;
//std::map<int, Message> neighbourMsgBuffer;
//
//
//
//// Forced sleep time after each tick per process in MS (set 0 to turn off sleep)
//const int tickSleepTime = 1000;
//
//// our default graph:   
////         [0]    [1]             [2]    
////        /  |   /    \          /  |
////      7   16  2      10      13   9
////     /     | /          \   /     |
////  [3]__3__[4]__5__[5]_18_[6]__12__[7] 
////     \     | \     |    / \        \
////      1   15  6    17  4   19      14
////        \  |    \  | /      \        \ 
////          [8]_13__[9]__8__[10]__11__[11]
//
//
////  weight matrix 
//// (similar to adjacency matrix but with the
//// following meaning: 
//// if (m[i][k] == 0) --> no edge between node i and node k
//// else there is an edge with weight  = m[i][k]
//
//std::vector<std::vector<int>> admat = {
//	// 0   1   2   3   4   5   6   7   8   9  10  11
//	{ 0, 0, 0, 7, 16, 0, 0, 0, 0, 0, 0, 0 }, // 0
//	{ 0, 0, 0, 0, 2, 0, 10, 0, 0, 0, 0, 0 }, // 1
//	{ 0, 0, 0, 0, 0, 0, 13, 9, 0, 0, 0, 0 }, // 2
//	{ 7, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0 }, // 3 
//	{ 16, 2, 0, 3, 0, 5, 0, 0, 15, 6, 0, 0 }, // 4
//	{ 0, 0, 0, 0, 5, 0, 18, 0, 0, 17, 0, 0 }, // 5
//	{ 0, 10, 13, 0, 0, 18, 0, 12, 0, 4, 19, 0 }, // 6
//	{ 0, 0, 9, 0, 0, 0, 12, 0, 0, 0, 0, 14 }, // 7
//	{ 0, 0, 0, 1, 15, 0, 0, 0, 0, 13, 0, 0 }, // 8
//	{ 0, 0, 0, 0, 6, 17, 4, 0, 13, 0, 8, 0 }, // 9
//	{ 0, 0, 0, 0, 0, 0, 19, 0, 0, 8, 0, 11 }, //10
//	{ 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 11, 0 }, //11
//};
//
//
//// helper functions
//void printMsg(const std::string& msg)
//{
//	std::cout << "[P " << rank << "]: " << msg << std::endl;
//}
//
//void removeFromContainer(int val, std::vector<int>& cont)
//{
//	cont.erase(std::remove(cont.begin(), cont.end(), val), cont.end());
//}
//
//
//// poll a specific node
//bool poll(Message& msg, int rank)
//{
//	// don't allow polling of non neighbours
//	assert(neighbourRequests.find(rank) != neighbourRequests.end() && "Polling of illegal neighbour rank!");
//
//	mpi::request& req = neighbourRequests[rank];
//
//	// poll the requestblock
//	if(req.test().get_ptr() == NULL) {
//		printMsg("polling " + std::to_string(rank) + " without success.");
//		return false;
//	}
//
//	// we received a msg
//	msg = neighbourMsgBuffer[rank];
//	printMsg("polling " + std::to_string(rank) + " and received a msg of type: " + std::to_string(msg.type));
//
//	// generate new request block for this neighbour
//	req = world.irecv(rank, 0, neighbourMsgBuffer[rank]);
//
//	return true;
//}
//
//
//// extend the current local tree by extending it on the shortest outgoing edge
//// 
////		for the simplest case we don't have a local tree yet so we simply add
////		our shortest outgoing graph edge as a tree edge and notify the neighbor
////		that is connected through that edge.
//void connectShortestOutgoingEdge()
//{
//	int candidateRank; // the node of the local tree from which the outgoing edge originates
//	EdgeConnection candidateEdge(-1, -1);
//
//	// 1. determine the edge to choose
//
//	// simplest case, we don't belong to a tree yet so we simply 
//	if(treeNeighbours.size() == 0) {
//		candidateRank = rank;
//		// find shortest local outgoing edge
//		candidateEdge = outgoingEdges.front();
//		for(auto& edgeCon : outgoingEdges) {
//			// choose the lowest cost edge, or the lowest rank for same cost edges
//			if(edgeCon.cost < candidateEdge.cost ||
//			   (edgeCon.cost == candidateEdge.cost && edgeCon.node < candidateEdge.node))
//			   candidateEdge = edgeCon;
//		}
//	}
//	// more complex case, we already belong to a local tree
//	// we therefore have to run a leader election process
//	// to find the shortest outgoing edge for the whole tree
//	else {
//		// @todo
//
//	}
//
//	// 2. update local tree structure and notify added edge neighbours
//
//	// only notify if the selected edge is connected to this node
//	if(candidateRank == rank) {
//		treeNeighbours.push_back(candidateEdge.node);
//		removeFromContainer(candidateEdge.node, nonTreeNeighbours);
//
//		Message msg(Message::MT_EdgeAddedNotification, candidateRank, candidateEdge.node);
//		world.isend(candidateEdge.node, 0, msg);
//	}
//}
//
//void handleMessage(const Message& msg)
//{
//	switch(msg.type) {
//	case Message::MT_TreeLeaderCandidate: break;
//	case Message::MT_TreeLeaderFound: break;
//	case Message::MT_OutgoingEdgeCandidate: break;
//	case Message::MT_OutgoingEdgeFound: break;
//	case Message::MT_EdgeAddedNotification:
//
//		break;
//	}
//}
//
//// run a leader election on the local tree with this nodes candidate being
//// candidateRank with candidateToken.
//void runLeaderElection()
//{
//	// @todo ...
//}
//
//void findMinimumSpanningTree()
//{
//	printMsg("Finding minimum spanning tree.");
//
//	exit(0);
//
//	while(true)
//	{
//		// 1. find shortest outgoing edge from current local tree
//		connectShortestOutgoingEdge();
//
//		// poll all neighbours for general messages
//		for(auto& neighbour : outgoingEdges) {
//			Message msg;
//			if(poll(msg, neighbour.node))
//				handleMessage(msg);
//		}
//
//		// 2. run leader election on local tree
//		runLeaderElection();
//
//		// 3. determine if we're done 
//		// @todo ...
//
//		if(tickSleepTime)
//			Sleep(tickSleepTime);
//	}
//}
//
//
//int main()
//{
//	// early out if the number of processes doesn't match the adjecency list
//	if(world.size() != admat.size())
//	{
//		if(world.rank() == 0)
//			std::cout << "Number of processes doesn't match the adjacency list, please use -n " << admat.size() << std::endl;
//		return 1;
//	}
//
//
//	// data for this process
//	rank = world.rank();
//	treeLeader = rank; // initial leader candidate is the local node
//
//	// fill outgoing edges list
//	for(int i = 0; i < admat[rank].size(); ++i) {
//		int cost = admat[rank][i];
//		if(cost > 0) {
//			// add edges with non zero costs to the neighbour list
//			outgoingEdges.push_back(EdgeConnection(i, cost));
//			nonTreeNeighbours.push_back(i); // seperate list to keep track of graph edges not in our local tree
//		}
//	}
//
//
//
//	printMsg("Initialized.");
//
//	// @todo the whole implementation in a more object oriented aproach 
//
//
//	// 1. find minimum spanning tree on the graph
//	findMinimumSpanningTree();
//
//	// 2. broadcast the graph structure so that each process has full information
//	// @todo ...
//
//	// 3. run a path finding algorithm in parallel or something...
//	// @todo ...
//
//
//
//
//
//	// initialize our
//
//
//
//
//	return 0;
//}