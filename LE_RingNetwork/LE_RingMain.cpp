#include <boost/mpi.hpp>

#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/generator_iterator.hpp>
#include <boost/random/mersenne_twister.hpp>

namespace mpi = boost::mpi;

enum eElectionPhase
{
	EP_CandidateFinding,
	EP_LeaderElected
};

class BufferObject
{
public:
	int val;		// random value of the current leader candidate
	int rank;	// rank of the current leader candidate
	eElectionPhase phase;

	BufferObject()
		: val(0),
		rank(0),
		phase(EP_CandidateFinding)
	{ }

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & val;
		ar & rank;
		ar & phase;
	}
};

bool genSendData(BufferObject& in, int val, int rank)
{
	// update min value
	// if the last minValue is bigger than the value of our process
	// OR if its equal to our process and our process has a lower rank
	// we set it to our current process' value
	if(in.val > val || (in.val == val && in.rank > rank)) {
		return false;
	}

	return true;
}

int main()
{
	// random generator
	boost::mt19937 rng;
	boost::random::uniform_int_distribution<> rand10(0, 9);

	// mpi stuff
	mpi::environment	env;
	mpi::communicator	world;
	
	int					cycleCount = 0;
	int					rank = world.rank();
	int					nextRank = (rank + 1) % world.size();
	int					prevRank = (rank - 1 + world.size()) % world.size();
	std::vector<int>	rndValues;
	BufferObject		data;

	// fill randomVal
	std::cout << "P" << rank << "'s random number list: " << std::endl;
	for(int i = 0; i < world.size(); ++i) {
		rndValues.push_back(rand10(rng));

		std::cout << rndValues[i] << " ";
	}
	std::cout << std::endl;
	

	// fill initial sendData
	data.rank = rank;
	data.val = rndValues[rank];

	std::cout << "P" << rank << " initialized with (" << rndValues[rank] << ")" << std::endl;

	while(true){

		mpi::request reqs[2];

		// we need to only send data to the next node if the current min value is lower than our own
		// if we own a lower value then we shold just wait for our own packet to arrive again
		bool needsToSend = (data.val <= rndValues[rank] || (data.val == rndValues[rank] && data.rank < rank));
		if(needsToSend) {
			std::cout << "P" << rank << " is worse than the current candidate and will relay the packet" << std::endl;
		}

		// send data to the next node in line if we need to
		if(needsToSend || cycleCount == 0) {
			reqs[0] = world.isend(nextRank, 0, data);
			std::cout << "P" << rank << " trying to send (rank:" << data.rank << ", val:" << data.val << ")" << std::endl;
		}


		std::cout << "P" << rank << " listening" << std::endl;
		// receive package from previous node
		reqs[1] = world.irecv(prevRank, 0, data);

		// wait for the send and receive operations to finish
		if(needsToSend) {
			reqs[0].wait();
			std::cout << "P" << rank << " just sent (rank:" << data.rank << ", val:" << data.val << ")" << std::endl;
		}

		reqs[1].wait();
		std::cout << "P" << rank << " just received (rank:" << data.rank << ", val:" << data.val << ")" << std::endl;

		// we're done if we receive a leader elected packet
		if(data.phase == EP_LeaderElected) {
			world.send(nextRank, 0, data);
			break;
		}

		// test exit condition
		if(data.rank == rank) {
			if(data.phase == EP_CandidateFinding) {
				std::cout << "P" << rank << " determined that it is the leader." << std::endl;
				// we received ourselves, change to phase 2
				data.phase = EP_LeaderElected;
			}
		}

		// increment cycle
		++cycleCount;
	}

	std::cout << "P" << world.rank() << " knows that " << data.rank << " is the leader. " << std::endl;


	return 0;
}




/* sending custom objects:

struct ObjectBuffer
{
std::string id;
int val;

template<class Archive>
void serialize(Archive & ar, const unsigned int version)
{
ar & id;
ar & val;
}
};

int main()
{
mpi::environment	env;
mpi::communicator	world;

// first test: see if we can send the above struct around
if(world.rank() == 0) {
ObjectBuffer buf;
buf.id = "hello";
buf.val = 13;
world.send(1, 0, buf);
}
else {
ObjectBuffer buf;
world.recv(0, 0, buf);
std::cout << "rcvd: " << buf.id << " " << buf.val << std::endl;
}

return 0;
}
*/