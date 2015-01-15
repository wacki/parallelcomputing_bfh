#pragma once

#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>
#include "Frontend.h"
#include "Log.h"

// client class, runs the main client loop for sending querys and updates to the server
class Client
{
public:
    Client()
        :_currentMsgId(0)
    { }

	// setters
	void setMinActionDelay(float delay) { _minActionDelay = delay; }
	void setMaxActionDelay(float delay) { _maxActionDelay = delay; }
	void setQueryWeight(float w) { _queryWeight = w; recomputePercentageChances(); }
	void setReplyWeight(float w) { _replyWeight = w; recomputePercentageChances(); }
	void setPostWeight(float w) { _postWeight = w; recomputePercentageChances(); }

	// main run loop
	void run();

protected:
	Frontend _FE;	// frontent instance for this client

    int     _currentMsgId;

	float _minActionDelay;	// min wait time for a client before starting new actions
	float _maxActionDelay;	// max wait time for a client before it has to complete a new action
	float _queryWeight;
	float _replyWeight;
	float _postWeight;

	float _queryChance;		// percentage chance for a query to happen
	float _replyChance;		// percentage chance for a query to happen
	float _postChance;		// percentage chance for a query to happen


	void recomputePercentageChances() { 
		float sum = _queryWeight + _replyWeight + _postWeight; 
		_queryChance = _queryWeight / sum;
		_replyChance = _replyWeight / sum;
		_postChance = _postWeight / sum;
	}

	void performAction();
	
    void sendUpdate();
    void sendUpdateReply();
    void sendQueryRequest();
};