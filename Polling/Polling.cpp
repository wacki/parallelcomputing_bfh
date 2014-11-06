#include <boost/mpi.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>
namespace mpi = boost::mpi;

int main()
{
	mpi::environment env;
	mpi::communicator world;

	boost::timer t;
	int rank = world.rank();
	double lastPollTime = 0.0;
	int sendChance = 5;	// chance to send a random message in %
	bool sendingMsg = false;

	std::srand(rank);

	mpi::request sendRequest;
	std::vector<mpi::request> recvRequests;
	std::vector<std::string> recvMsg;

	// initialize recv msg and requests
	for(int i = 0; i < world.size(); ++i) {
		if(i == rank)
			continue;
		recvMsg.push_back("");
		recvRequests.push_back(world.irecv(i, 0, recvMsg[i]));
	}

	while(true)
	{
		// random chance to send
		if(!sendingMsg && (std::rand() % 100) < sendChance) {
			// send to a random receiver
			int receiver = std::rand() % world.size();
			
			if(receiver != rank) {
				sendingMsg = true;
				sendRequest = world.isend(receiver, 0, "hello");
				std::cout << "P[" << rank << "] sending msg to " << receiver << "." << std::endl;
			}
		}
		else if(sendingMsg) {
			// if the request test call does not fail then our msg was sent
			if(sendRequest.test().get_ptr() != NULL) {
				std::cout << "sendrequest success" << rank << std::endl;
				sendingMsg = false;
			}
		}

		// poll
		if(t.elapsed() > lastPollTime + 0.1) {
			std::cout << "P[" << rank << "] polling." << std::endl;
			lastPollTime = t.elapsed();

			for(int i = 0; i < world.size(); ++i) {
				if(i == rank)
					continue;
				mpi::request& req = recvRequests[i];
				mpi::status* status = req.test().get_ptr();
				if(status != NULL) {
					// we received something, print success and generate a new request

					std::cout << "P[" << rank << "] received msg from " << status->source() << ": " << recvMsg[i] << std::endl;
					recvRequests.push_back(world.irecv(i, 0, recvMsg[i]));
				}
			}
		}

		if(t.elapsed() > 50)
			break;
	}

	std::cout << "P[" << rank << "] terminated." << std::endl;


	return 0;
}