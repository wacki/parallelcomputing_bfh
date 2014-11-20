
#include <boost/mpi.hpp>


enum MessageType {
	MT_TreeLeaderElect = 0,
	MT_TreeLeaderFound = 1,
	MT_TreeGrowElect = 2,
	MT_TreeGrowFound = 3,
	MT_NeighbourConnected = 4
};


class BaseMessage
{
	friend class boost::serialization::access;
public:
	BaseMessage(MessageType type)
		: _type(type)
	{ }

	int tag() { return (int)_type; }

private:
	MessageType _type;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & type;
	}
};



class TreeLeaderElectMsg : public BaseMessage
{
	friend class boost::serialization::access;
public:
	int minRank;

	TreeLeaderElectMsg()
		: BaseMessage(MT_TreeLeaderElect)
	{ }

private:
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & minRank;
	}
};


class GraphLeaderElectMsg;




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

typedef ConnectionObject<TreeLeaderElectMsg> TreeLeaderElectCO;