
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


namespace mpi = boost::mpi;

class GraphCheapestEdgeMsg;


enum LogVerbosity {
    LV_Quiet = 0,
    LV_Minimal = 1,
    LV_Normal = 2,
    LV_Detailed = 3,
    LV_Diagnostic = 4
};

class Log
{
public:

    ~Log()
    {
        if (_ofs.is_open())
            _ofs.close();
    }

    static Log& singleton()
    {
        static Log log;
        return log;
    }
    
    void open(const std::string& file)
    {
        _ofs.open(file);
    }
    
    void setVerbosity(LogVerbosity verb) { _verbosity = verb; }
    LogVerbosity getVerbosity() const { return _verbosity; }
    
    void logMsg(LogVerbosity verb, const std::string& msg)
    {
        if (_verbosity < verb)
            return;

        std::cout << msg;
        std::cout.flush();
        _ofs << msg;
        _ofs.flush();
    }

private:
    // prevent instantiation
    Log()
        : _verbosity(LV_Diagnostic)
    {}

    LogVerbosity    _verbosity;
    std::ofstream   _ofs;

};

class Node
{
public:
	Node();

	void print();
	virtual std::string toString();
    void logMsg(LogVerbosity verb, const std::string& msg)
    {
        std::ostringstream oss;
        oss << "[" << std::setw(2) << std::setfill('0') << world.rank() << "]: ";
        Log::singleton().logMsg(verb, oss.str() + msg);
    }

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
	GraphNode();

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

    logMsg(LV_Normal, "  LE: Starting leader election process.\n");
    logMsg(LV_Diagnostic, "  LE: Remaining requests: " + std::to_string(_neighbours.size()) + "\n");
	// wait for leader elect messages 
	while(requestsLeft > 1)
	{
		for(auto& nr : neighbourRequests)
		{
			if(!nr->done() && nr->poll())
			{
				if(nr->msg() < (*myMessage)) {
                    myMessage->merge(nr->msg());
                    logMsg(LV_Detailed, "  LE: better candidate: " + myMessage->toString() + "\n");
				}
				requestsLeft--;
			}

		}
	}

	if(requestsLeft == 0) {

		//if(outputLog) std::cout << "[" << world.rank() << "] " << "Found leader (" << myMessage->minRank << "), notifying neighbours." << std::endl;
        logMsg(LV_Minimal, "  LE: Found leader: " + myMessage->toString() + "; distributing to neighbours.\n");
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
        world.isend(remainingReq->rank(), myMessage->tag(), *myMessage);
        
        logMsg(LV_Normal, "  LE: sending candidate: " + myMessage->toString() + "\n");
		// wait for answers
		remainingReq->wait();

        
		// find better soluton
        if (remainingReq->msg() < (*myMessage)) {
            myMessage->merge(remainingReq->msg());
            logMsg(LV_Diagnostic, "  LE: Received a better candidate from the last neighbour\n");
        }
        
    logMsg(LV_Diagnostic, "  LE: Notifying all neighbours about result.\n");
		for(auto neighbour : _neighbours)
			if(neighbour != remainingReq->rank())
				world.isend(neighbour, myMessage->tag(), *myMessage);
	}

    
    logMsg(LV_Minimal, "  LE: done: " + myMessage->toString() + "\n");
}