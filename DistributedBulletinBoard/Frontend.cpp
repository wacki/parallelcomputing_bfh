
#include "Frontend.h"

#include <random>

#include "common.h"
    
Frontend::Frontend()
: _timestamp(gNumRM)
{
    userId = gRank - gNumRM;
    _cid = userId << 16;
    
    for (int i = 0; i < gNumRM; ++i)
        _connectedRM.push_back(i);
}

const std::vector<Message>& Frontend::query()
{
	// choose a random RM
    int rmId = getRandomRM();
    
    Log::add(LV_Normal, "FE" + std::to_string(gRank-gNumRM) + ": sending query to RM" + std::to_string(rmId) + "\n");

	// generate request message
    QueryRequestMsg request;
    request.prev = _timestamp;
    gWorld.send(rmId, request.tag(), request);
    
	// wait for the reply
    QueryReplyMsg reply;
    gWorld.recv(rmId,reply.tag(), reply);

	// update local value with received value
    _value.clear();
    _value = reply.value;
    
    Log::add(LV_Normal, "FE" + std::to_string(gRank-gNumRM) + ": received query reply from RM" + std::to_string(rmId) + "\n");
    for (auto& msg : _value)
    {
        Log::add(LV_Normal, "  " +  msg.toString() + "\n");
    }
    
    // update own timestamp
    _timestamp = TSVec::max(reply.updateId, _timestamp);
    
    Log::add(LV_Normal, "FE" + std::to_string(gRank-gNumRM) + ": new timestamp: " + _timestamp.toString() + "\n");

    return _value;
}

void Frontend::update(Message& msg)
{
	// the msg will only contain a title and content, we append additional information here
    msg.setUserId(userId);
    msg.setCid(_cid++);
    msg.setPrev(_timestamp);

	// choose a random rm
    int rmId = getRandomRM();
    
    Log::add(LV_Normal, "FE" + std::to_string(gRank-gNumRM) + ": sending update to RM" + std::to_string(rmId) + " " + msg.toString() + "\n");
    
	// generate update request
	UpdateRequestMsg request;
    request.msg = msg;

    gWorld.send(rmId, request.tag(), request);
    
	// wait for the reply
    UpdateReplyMsg reply;
    gWorld.recv(rmId,reply.tag(), reply);

    // update own timestamp
    _timestamp = TSVec::max(reply.updateId, _timestamp);
    
    Log::add(LV_Normal, "FE" + std::to_string(gRank-gNumRM) + ": received new timestamp after sending message to " + std::to_string(rmId) + " ts: " + _timestamp.toString() + "\n");
}


int Frontend::getRandomRM() const
{
	std::random_device rd;

	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> uniform_dist(0, _connectedRM.size() - 1);

    int index = uniform_dist(e1);
    return _connectedRM[index];
}

const Message* Frontend::getRandomMessage() const
{
    if (_value.empty())
        return nullptr;

	std::random_device rd;

	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> uniform_dist(0, _value.size());
    
    int index = uniform_dist(e1);
    return &_value[index];
}