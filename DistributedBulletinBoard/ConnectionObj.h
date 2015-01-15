
#pragma once

#include <algorithm>
#include <set>
#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>

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