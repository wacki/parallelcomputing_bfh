#pragma once

#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

// project includes
#include "TSVec.h"
#include "Msg.h"
#include "ConnectionObj.h"
#include "Log.h"



class ReplicaManager
{
public:
    ReplicaManager();

	/** main run loop for the RM */
    void run();

private:
	/** timestamp vector for this RM's value. */
	TSVec _valueTS;

    /** value of this RM (current status of messages */
    std::vector<Message> _value;

	/** timestamp vector for this RM's replica log. */
	TSVec _replicaTS;

	/** set for unique log updates */
    std::set<Message> _log;

	/** cid's of executed msgs in the log*/
    std::set<long> _executed;


    /** Walking index for gossip messages */
    int _gossipRMindex;
    boost::posix_time::ptime _lastGossip;
    
	/** connection objects for the possible incoming messages */
    std::vector<ConnectionObject<UpdateRequestMsg>*> _updateReq;
    std::vector<ConnectionObject<QueryRequestMsg>*> _queryReq;
    std::vector<ConnectionObject<GossipMsg>*> _gossipReq;
    
	// queue of pending updates and query
    std::vector<UpdateRequestMsg> _pendingUpdateReq;
    std::vector<QueryRequestMsg> _pendingQueryReq;
	
	// helper functions
    void poll();
    bool handleMsg(const UpdateRequestMsg& msg);
    bool handleMsg(const QueryRequestMsg& msg);
    bool handleMsg(const GossipMsg& msg);
    
    void checkPending();
    void updateLog();
    void generateGossipMsg();
};