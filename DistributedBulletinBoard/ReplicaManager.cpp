
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#include "common.h"
#include "ReplicaManager.h"


ReplicaManager::ReplicaManager()
: _valueTS(gNumRM), _replicaTS(gNumRM),
_gossipRMindex(0)
{
    for (int i = 0; i < gNumRM; ++i) {
        if (gRank != i) {

            _gossipReq.push_back(new ConnectionObject<GossipMsg>(i, true));
        }
    }

    for (int i = gNumRM; i < gNumRM + gNumFE; ++i) {
        _updateReq.push_back(new ConnectionObject<UpdateRequestMsg>(i, true));
        _queryReq.push_back(new ConnectionObject<QueryRequestMsg>(i, true));
    }
}

void ReplicaManager::run()
{
	float tickRate = 10.0f;		// ticks / second
	float cycleTime = 1000.0f / tickRate;	// cycle time budget in ms
	float timeSinceLastAction = 0.0f;
	boost::posix_time::ptime prevTime = boost::posix_time::microsec_clock::local_time(); // time of last cycle start

	// the RM has a constant tickrate of 'tickRate' at which it will poll for new messages or send out it's own gossip messages
	while(true)
	{
		boost::posix_time::ptime cycleStart = boost::posix_time::microsec_clock::local_time();
		timeSinceLastAction += (float)(cycleStart - prevTime).total_seconds();
		prevTime = cycleStart;
		
        // poll all sources for possible requests
        poll();

        // generate gossip messages
        generateGossipMsg();

        // work off any pending requests
        checkPending();


		// wait for the remaining cycle time to keep a constant tickrate
		boost::posix_time::ptime cycleEnd = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration msdiff = cycleEnd - cycleStart;

		float sleepTime = cycleTime - msdiff.total_milliseconds();

		//std::cout << "waiting for " << sleepTime << " ms" << std::endl;
		if(sleepTime > 0.0f)
			boost::this_thread::sleep(boost::posix_time::millisec(sleepTime));
	}
}


void ReplicaManager::poll()
{
    // check for gossip messages
    for (auto req : _gossipReq) {
        if (req->poll()) {
            handleMsg(req->msg());
        }
    }
    
    // check for update requests
    for (auto req : _updateReq) {
        if (req->poll()) {
            _pendingUpdateReq.push_back(req->msg());
        }
    }
    
    // check for query requests
    for (auto req : _queryReq) {
        if (req->poll()) {
            Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": got query request from " + std::to_string(req->msg().origin() - gNumRM) + ": prev " + req->msg().prev.toString() +"\n");
            
            _pendingQueryReq.push_back(req->msg());
        }
    }
}
// handle updates
bool ReplicaManager::handleMsg(const UpdateRequestMsg& msg)
{
   Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": got update msg : " + msg.msg.toString() + "\n");

    // increase own replica timestamp
    _replicaTS[gRank] += 1;

    // add the message into the log
    Message m = msg.msg;

    // set timestamp of message to prev
    m.setTs(m.prev());
    m.ts()[gRank] = _replicaTS[gRank];
    
    Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": new replicaTS: " + _replicaTS.toString() + "\n");

	_log.insert(m);
    
    UpdateReplyMsg reply;
    reply.updateId = m.ts();

    gWorld.send(msg.origin(), reply.tag(), reply);

    // update the log after entering a new message
    updateLog();

    return true;
}
// handle queries
bool ReplicaManager::handleMsg(const QueryRequestMsg& msg)
{
    if (msg.prev <= _valueTS)
    {
        
        Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": answering pending query request from FE" + std::to_string(msg.origin()-gNumRM) + "\n");

        QueryReplyMsg reply;
        reply.value = _value;
        reply.updateId = _valueTS;

        gWorld.send(msg.origin(), reply.tag(), reply);

        return true;
    }

    return false;
}
bool ReplicaManager::handleMsg(const GossipMsg& msg)
{
    Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": got gossip message from RM" + std::to_string(msg.origin()) + "\n");
    Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": replicaTS: " + _replicaTS.toString() + "; Msg TS: " + msg.ts.toString() +"\n");

    // merge the new log in
    _log.insert(msg.log.begin(), msg.log.end());
    // update the timestamp
    _replicaTS = TSVec::max(_replicaTS, msg.ts);
    

    // update the log afer receiving a new gossip msg
    updateLog();
    return true;
}

void ReplicaManager::checkPending()
{
    // execute pending update requests
    {
        auto i = std::begin(_pendingUpdateReq);
        while (i != std::end(_pendingUpdateReq)) {
            if (handleMsg(*i))
                i = _pendingUpdateReq.erase(i);
            else
                ++i;
        }
    }
    // execute pending query requests
    {
        auto i = std::begin(_pendingQueryReq);
        while (i != std::end(_pendingQueryReq)) {
            if (handleMsg(*i))
                i = _pendingQueryReq.erase(i);
            else
                ++i;
        }
    }
}

void ReplicaManager::updateLog()
{
    Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": Checking log for new messages. valueTS: " + _valueTS.toString() + "\n");
    for (auto& msg : _log)
    {
        if (_executed.find(msg.getCid()) == _executed.end())
        {
            Log::add(LV_Normal, "  " + msg.toString() + "\n");
            // the current 'msg' has not been executed yet
            // @todo we currently walk the entire log every time we're here...
            //       pretty stupid and ugly. could be easily optimized.

            if (msg.getPrev() <= _valueTS) {
                _value.push_back(msg);
                _valueTS = TSVec::max(_valueTS, msg.getTs());
                _executed.insert(msg.getCid());
            }

        }
    }
}


void ReplicaManager::generateGossipMsg()
{
    float gossipInterval = 5.0f; // send gossip msg every 30 seconds
    
    // skip if index is our current rank
    if (_gossipRMindex == gRank) {
        _gossipRMindex++;
        _gossipRMindex %= gNumRM;
    }

    
    float deltaT = (boost::posix_time::microsec_clock::local_time() - _lastGossip).seconds();
        
    if (deltaT > gossipInterval) {
        Log::add(LV_Normal, "RM" + std::to_string(gRank) + ": sending gossip msg to: " + std::to_string(_gossipRMindex) + "\n");

        _lastGossip = boost::posix_time::microsec_clock::local_time();

        GossipMsg req;
        req.ts = _replicaTS;
        req.log.insert(_log.begin(), _log.end());
        gWorld.isend(_gossipRMindex, req.tag(), req);
    }

    // update the index
    _gossipRMindex++;
    _gossipRMindex %= gNumRM;
}