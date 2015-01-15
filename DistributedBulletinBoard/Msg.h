#pragma once

#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>

#include "common.h"
#include "TSVec.h"

namespace mpi = boost::mpi;

/**
This file contains the update message class (Message) as well as several classes
used as MPI message containers.
*/

/** Main update message class used as value entries*/
class Message
{
	friend class boost::serialization::access;
public:
    Message()
        : _prev(gNumRM), _ts(gNumRM)
    { }
    
    void                setTitle(const std::string& title) { _title = title; }
    void                setUserId(int id) { _userId = id; }
    void                setContent(const std::string& content) { _content = content; }
    void                setCid(long cid) { _cid = cid; }
    void                setPrev(TSVec& prev) { _prev = prev; }
    void                setTs(TSVec& ts) { _ts = ts; }

    const std::string&  getTitle() const { return _title; }
    int                 getUserId() const { return _userId; }
    const std::string&  getContent() const { return _content; }
    long                getCid() const { return _cid; }
    const TSVec&        getPrev() const { return _prev; }
    const TSVec&        getTs() const { return _ts; }

    // special accessorts for direct manipulation
    TSVec&              prev() { return _prev; }
    TSVec&              ts() { return _ts; }

    std::string toString() const;
    
    
    bool operator<(const Message& rhs) const
    {
        if (_prev == rhs._prev ||
			(!(_prev < rhs._prev) && !(rhs._prev < _prev))) // this second line is necessary because thats how std::set tests equality. if we don't add it some of the msgs get tossed! (<2,0> and <0,2>) are equal when tested like this
            return _cid < rhs._cid;

        return (_prev < rhs._prev);
    }

protected:
    std::string     _title;
    std::string     _content;
    int             _userId;
    /** High 16 bit for user id, low 16 bit for message number. */
    long            _cid;   // unique caller id for this update

    TSVec           _prev;  // state of the sender of this message
    TSVec           _ts;    // timestamp for this message

private:
    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & _title;
		ar & _content;
		ar & _userId;
		ar & _cid;
		ar & _prev;
        ar & _ts;
	}
};

/** base mpi message class */
class MPIMessage
{
	friend class boost::serialization::access;
public:
    enum Type {
        UpdateRequest,
        UpdateReply,
        QueryRequest,
        QueryReply,
        Gossip
    };
    
    MPIMessage(Type type)
        : _type(type), _origin(gRank)
    { }

	int tag() const { return (int)_type; }
    int origin() const { return _origin; }
    
    Type getType() const { return _type; }


private:    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & _type;
        ar & _origin;
	}
    
    Type    _type;
    int     _origin;
};

/** update request message used to send new update messages from FEs to RMs */
class UpdateRequestMsg : public MPIMessage
{
	friend class boost::serialization::access;
public:
    UpdateRequestMsg()
        : MPIMessage(MPIMessage::UpdateRequest)
    { }

    Message msg;

private:    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<MPIMessage>(*this);
		ar & msg;
	}
};

/** Reply message used by RMs to notify FEs about receiving an update request */
class UpdateReplyMsg : public MPIMessage
{
	friend class boost::serialization::access;
public:
    UpdateReplyMsg()
        : MPIMessage(MPIMessage::UpdateReply),
        updateId(gNumRM)
    { }

    TSVec updateId;

private:    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<MPIMessage>(*this);
		ar & updateId;
	}
};

/** message requesting a query reply from an RM */
class QueryRequestMsg : public MPIMessage
{
	friend class boost::serialization::access;
public:
    QueryRequestMsg()
        : MPIMessage(MPIMessage::QueryRequest),
        prev(gNumRM)
    { }

    TSVec prev;

private:    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<MPIMessage>(*this);
		ar & prev;
	}
};

/** Message containing value and new valueTS as a reply to Query requests*/
class QueryReplyMsg : public MPIMessage
{
	friend class boost::serialization::access;
public:
    QueryReplyMsg()
        : MPIMessage(MPIMessage::QueryReply),
        updateId(gNumRM)
    { }

    TSVec updateId;
    std::vector<Message> value;

private:    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<MPIMessage>(*this);
		ar & updateId;
        ar & value;
	}
};

/** Gossip message used to send a RM's log around */
class GossipMsg : public MPIMessage
{
	friend class boost::serialization::access;
public:
    GossipMsg()
        : MPIMessage(MPIMessage::Gossip), ts(gNumRM)
    { }

    // TODO
    TSVec ts;
    std::set<Message> log;

private:    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
        ar & boost::serialization::base_object<MPIMessage>(*this);
        ar & ts;
        ar & log;
	}
};