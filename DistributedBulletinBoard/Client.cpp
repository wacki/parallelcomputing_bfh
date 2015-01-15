
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#include <random>

#include "Client.h"


void Client::run()
{
	std::random_device rd;
    
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(_minActionDelay, _maxActionDelay);
    
    
	while(true)
	{
		// choose a random sleep time
        float sleepTime = dis(gen);
        Log::add(LV_Diagnostic, "Client" + std::to_string(gRank) + ": sleep time: " + std::to_string(sleepTime) + "\n");
        boost::this_thread::sleep(boost::posix_time::seconds(sleepTime));

        performAction();
	}
}

void Client::performAction()
{

	std::random_device rd;
    
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0f, 1.0f);

	// generate a random percentage
	float rand = dis(gen);
	float randStart = 0.0f;
	float randEnd = 0.0f;

	float chances[] = { _queryChance, _replyChance, _postChance };

	// execute the action that won the random draw above
	for(int i = 0; i < 3; ++i) {
		randStart = randEnd;
		randEnd += chances[i];

		if(randStart <= rand && rand <= randEnd) {
			switch(i)
			{
			case 0: sendQueryRequest(); break;
			case 1: sendUpdateReply(); break;
			case 2: sendUpdate(); break;
			}

			break;
		}
	}

}


void Client::sendUpdate()
{
    Log::add(LV_Diagnostic, "Client" + std::to_string(gRank) + ": sending new update message\n");

    // construct message
    Message msg;
    msg.setTitle(std::to_string(_currentMsgId++));
    msg.setContent("some content");
    
    // perform update
    _FE.update(msg);
}

void Client::sendUpdateReply()
{
    Log::add(LV_Diagnostic, "Client" + std::to_string(gRank) + ": sending reply message.\n");

    const Message* originalMsg = _FE.getRandomMessage();

    if (originalMsg != nullptr) {
        
        // construct message
        Message msg;
        std::ostringstream oss;
        oss << "RE: " << originalMsg->getTitle() << " from FE ";
        oss << originalMsg->getUserId();
        // @note we don't increment or even mention the _curremtMsgId in replys

        msg.setTitle(oss.str());

        // perform update
        _FE.update(msg);
    }
    else {        
        Log::add(LV_Diagnostic, "Client" + std::to_string(gRank) + ": no previous messages, can't send reply message.\n");
        // Send a normal update if there are no messages previous to this
        sendUpdate();
    }
}

void Client::sendQueryRequest()
{
    const std::vector<Message>& value = _FE.query();
	
	// we could do something with the received value, but there's no need. the FE will log the received value for now anyway.
}
