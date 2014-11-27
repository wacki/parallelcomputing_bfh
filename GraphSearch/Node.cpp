
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
		mstNodes[neighbour.to] = neighbour.cost;
	}
    logMsg(LV_Minimal, "Starting baruvka\n");
	// print current candidates 
    std::ostringstream oss;
    oss << "Possible MST candidates: ";
	for(auto edge : mstNodes)
       oss << "(" << std::to_string(world.rank()) << " -{" + std::to_string(edge.second) << "}-> " + std::to_string(edge.first) << ")";

    oss << "\n";
    logMsg(LV_Normal, oss.str());
    
    
    while (true)    {
        // 1. find edge candidates for current node and add it to the msg
        GraphCheapestEdgeMsg msg;
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
            logMsg(LV_Normal, "Found no suitable local candidate edge\n");
		}
        else {            
            logMsg(LV_Normal, "Chose (" + std::to_string(world.rank()) + " -{" + std::to_string(minEdge.cost) + "}-> " + std::to_string(minEdge.to) + ") as the best local candidate.\n");
        }

        msg.minRank = world.rank();
        msg.minEdgeRank = minEdgeRank;
        msg.edgeTo = minEdge.to;
        msg.edgeCost = minEdge.cost;

        logMsg(LV_Detailed,  "Current MST neighbours: " + mstNode.toString() + "\n");

        // 2. run leader election to find the best edge in our mst
        LogVerbosity prevVerb = Log::singleton().getVerbosity();
        Log::singleton().setVerbosity(LV_Quiet); 
        mstNode.electLeader<GraphCheapestEdgeMsg>(&msg);        
        Log::singleton().setVerbosity(prevVerb); 

        // break if we didn't find anything
		if(msg.edgeTo == INT_MAX)
			break;

        logMsg(LV_Normal, "Best candidate for growing the MST is: (" + std::to_string(msg.minRank) + " -{" + std::to_string(msg.edgeCost) + "}-> " + std::to_string(msg.edgeTo) + ")\n");
        
                
        // 3. if our node won the election send a notification to the neighbour that was selected
        if (msg.minRank == world.rank()) {
            MSTGrowMsg growMsg;

            if (msg.minEdgeRank == world.rank()) {
                logMsg(LV_Detailed, "Sending grow request to " + std::to_string(msg.edgeTo) + "\n");
                world.send(msg.edgeTo, growMsg.tag(), growMsg);
                logMsg(LV_Detailed, "Successfully sent grow request to " + std::to_string(msg.edgeTo) + "\n");
            }
            else {
                logMsg(LV_Detailed, "Waiting for grow request from " + std::to_string(msg.edgeTo) + "\n");
                world.recv(msg.edgeTo, growMsg.tag(), growMsg); 
                logMsg(LV_Detailed, "Successfully received grow request from " + std::to_string(msg.edgeTo) + "\n");
            }

            mstNode.addNeighbour(msg.edgeTo); // also add the found node to the mst neighbour list
			mstNodes.erase(msg.edgeTo); // remove the new connection as possible candidate
        }
		
        // update MST node list
        CollectMstNodesMsg collectMsg;
        mstNode.insertNeighboursInSet(collectMsg.nodes);

        prevVerb = Log::singleton().getVerbosity();// don't output the collection process
        Log::singleton().setVerbosity(LV_Quiet); 
        mstNode.electLeader<CollectMstNodesMsg>(&collectMsg); // collect nodes in current MST       
        Log::singleton().setVerbosity(prevVerb); 


		// update candidate list based on list in the msg
		for(auto connected : collectMsg.nodes) {
			if(mstNodes.find(connected) != mstNodes.end()) {
				mstNodes.erase(connected);	// remove nodes that were encountered during the previous leader election as possible candidates
                logMsg(LV_Detailed, "Removing " + std::to_string(connected) + " from the candidate list since it's already in our MST\n");
			}
		}
        
        Sleep(1000);
        
    }

    // print out result
    logMsg(LV_Minimal, "Done building the MST, final neighbours: " + mstNode.toString() + "\n");
}

