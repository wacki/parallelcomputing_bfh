


#include <boost/mpi.hpp>

#include <iostream>
#include <string>
#include <map>
#include <boost/serialization/string.hpp>

#include "Msg.h"
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

int main()
{

	boost::mpi::environment		env;
	boost::mpi::communicator	world;
    
	if(world.size() != admat.size())
	{
		if(world.rank() == 0)
			std::cout << "Number of processes doesn't match the adjacency list, please use -n " << admat.size() << std::endl;
		return 1;
	}
    
	//TreeNode treeNode;
    Log::singleton().open("GraphSearch_" + std::to_string(world.rank()) + ".log"); // set log file
    Log::singleton().setVerbosity(LV_Detailed);
    GraphNode graphNode;
    GraphCheapestEdgeMsg msg;

    // fill neighbour list of graph node
    for (int i = 0; i < admat[world.rank()].size(); i++) {
		if(admat[world.rank()][i] > 0) {
			graphNode.addEdge(i, admat[world.rank()][i]);
		}
    }

    graphNode.baruvka();



	return 0;
}


