
#pragma once

#include <algorithm>
#include <set>
#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>

namespace mpi = boost::mpi;

enum MessageType {
	MT_GraphCheapestEdge = 0,
	MT_TreeGrow = 1,
    MT_CollectMST = 2
};


class BaseMessage
{
	friend class boost::serialization::access;
public:
	BaseMessage(MessageType type)
		: _type(type)
	{ }

	int tag() { return (int)_type; }
    std::string toString();

private:
	MessageType _type;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & _type;
	}
};

class MSTGrowMsg : public BaseMessage
{
public:
    MSTGrowMsg()
        : BaseMessage(MT_TreeGrow)
    { }

    std::string toString()
    {
        return "[MSTGrowMsg]";
    }
};

class CollectMstNodesMsg : public BaseMessage
{
	friend class boost::serialization::access;
public:
    std::set<int> nodes;

    CollectMstNodesMsg()
        : BaseMessage(MT_CollectMST)
    { }
    
    bool operator<(const CollectMstNodesMsg& rhs) const
    {
        return true;
    }
    void merge(const CollectMstNodesMsg& other)
    {
        nodes.insert(other.nodes.begin(), other.nodes.end());
    }
    std::string toString()
    {
        std::ostringstream oss;
        oss << "[CollectMstNodesMsg] ";
        for (auto node : nodes)
            oss << "(" << node << ") ";
        return oss.str();
    }
private:
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<BaseMessage>(*this);
        ar & nodes;
	}
};

class GraphCheapestEdgeMsg : public BaseMessage
{
	friend class boost::serialization::access;
public:
	int minRank;          // cheapest edge origin
    int minEdgeRank;      // min rank of the two end points in the edge
    int edgeTo;           // cheapest edge destination
    int edgeCost;         // cheapest edge cost

	GraphCheapestEdgeMsg()
		: BaseMessage(MT_GraphCheapestEdge)
	{ }

    // only a merge in the sense of the collected nodes
    // its more of an assignment for the other parameters
    void merge(const GraphCheapestEdgeMsg& other)
    {
        minRank = other.minRank;
        minEdgeRank = other.minEdgeRank;
        edgeTo = other.edgeTo;
        edgeCost = other.edgeCost;
    }

    bool operator<(const GraphCheapestEdgeMsg& rhs) const
    {
        //std::cout << "comparing " << minRank << "(" << edgeCost << ") to " << rhs.minRank << "(" << rhs.edgeCost << ")" << std::endl;

        return (edgeCost < rhs.edgeCost // our edge cost is better
            ||
            (edgeCost == rhs.edgeCost // edge costs are the same, select lower rank
            && minRank < rhs.minRank
            ));
    }
    std::string toString()
    {
        std::ostringstream oss;
        oss << "[GraphCheapestEdgeMsg] " << "(" << minRank << " -{" << edgeCost << "}-> " << edgeTo << ") min endpoint rank: " << minEdgeRank;
        return oss.str();
    }

private:
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<BaseMessage>(*this);
		ar & minRank;
        ar & minEdgeRank;
        ar & edgeTo;
        ar & edgeCost;
	}
};



// class that handles polling through a request block
// also added functionality of sending to the connection
// but the sending of messages can't be checked with a request, not needed.
template<class MT>
class ConnectionObject
{
public:
	ConnectionObject(int rank, bool continous = false)
		: _rank(rank), _continousRequests(continous), _reqActive(false)
	{
		start();
	}

	bool done() { return !_reqActive; }
	void start();
	bool poll();
	void send(const MT& msg, bool block = false);
	void wait();

	const MT& msg() { return _buffer; }
	int rank() { return _rank; }

private:
	mpi::communicator	world;

	bool				_reqActive;
	bool				_continousRequests; // create new requests automatically if a poll is successfull
	int					_rank;		// rank of the other connection
	mpi::request		_req;

	MT					_buffer;
};


template<class MT>
void ConnectionObject<MT>::start()
{
	_req = world.irecv(_rank, _buffer.tag(), _buffer);
	_reqActive = true;
}


template<class MT>
bool ConnectionObject<MT>::poll()
{
	if(_req.test())
	{
		_reqActive = false;
		if(_continousRequests)
			start();

		return true;
	}
	return false;
}

template<class MT>
void ConnectionObject<MT>::wait()
{
	if(_reqActive)
		_req.wait();
}

template<class MT>
void ConnectionObject<MT>::send(const MT& msg, bool block = false)
{
	if(block)
		world.send(_rank, msg.tag(), msg);
	else
		world.isend(_rank, msg.tag(), msg);
}

typedef ConnectionObject<GraphCheapestEdgeMsg> TreeLeaderElectCO;