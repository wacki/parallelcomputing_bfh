#pragma once

#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>

#include "TSVec.h"
#include "Msg.h"
#include "Log.h"

class Frontend
{
public:
    Frontend();

    const std::vector<Message>& query();
    void update(Message& msg);

    const std::vector<Message>& value() { return _value; }
        
    const Message* Frontend::getRandomMessage() const;

protected:

	/** unique user id for this frontend */
    int userId;

    /** RM's this front end has direct access to */
    std::vector<int> _connectedRM;

    /** last value received from a RM */
    std::vector<Message> _value;

    /** timestamp of the currnt status of this FE */
    TSVec _timestamp;

    /** choose a random RM id */
    int getRandomRM() const;
    
	/** current cid*/
    int _cid;	// @note  cid would be more useful as a special class that saves userId and message number as a unique hash. currently we use bit shift of a long which only works for 2 frontends max....
};