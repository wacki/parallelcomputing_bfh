#include <boost/mpi.hpp>

#include <Windows.h>
#include <iostream>
#include <string>
#include <map>
#include <boost/serialization/string.hpp>


namespace mpi = boost::mpi;


// data for this process
int					rank;
int					token;
std::vector<int>	neighbours;
int					neighbourCount;
std::vector<int>	remainingNeighbours;
std::vector<int>	neighboursToNotify;

// incident list
// 
//   0     1
//   |     |
// 2-3-4   5-6-7
//   |     |
//   8     9
// 
std::vector<std::vector<int>> incidentList = {
	{ 3 },			// 0
	{ 5 },			// 1
	{ 3 },			// 2
	{ 0, 2, 4, 8 },	// 3
	{ 3 },			// 4
	{ 1, 6, 9 },	// 5
	{ 5, 7 },		// 6
	{ 6 },			// 7
	{ 3 },			// 8
	{ 5 }			// 9
};


struct Message
{
	enum Type {
		MT_Candidate,
		MT_LeaderFound
	} type;

	int leader;
	int token;

	Message()
	{}

	Message(Type type, int leader, int token)
		: type(type), leader(leader), token(token)
	{ }


	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & type;
		ar & leader;
		ar & token;
	}
};

void printMsg(const std::string& msg)
{
	std::cout << "[P " << rank << "]: " << msg << std::endl;
}


int main()
{
	// mpi stuff
	mpi::environment	env;
	mpi::communicator	world;

	// early out if the number of processes doesn't match the incident list above
	// @todo allow the user to generate a random graph later, for now we just use this debug graph above
	if(world.size() != incidentList.size())
	{
		if(world.rank() == 0)
			std::cout << "Number of processes doesn't match the incident list, please use -n " << incidentList.size() << std::endl;
		return 1;
	}

	// seed the random gen
	std::srand(world.rank() * 4);

	// data for this process
	rank = world.rank();;
	token = std::rand() % 100;;
	neighbours = incidentList[world.rank()];
	neighbourCount = neighbours.size();
	remainingNeighbours = neighbours;
	neighboursToNotify = neighbours;
	std::map<int, mpi::request> requests;
	std::map<int, Message> recvBuffer;
	int currentLeader = rank;
	int currentLeaderToken = token;

	printMsg("my token is: " + std::to_string(token));

	// initialize request blocks
	for(int i : neighbours) {
		requests[i] = world.irecv(i, 0, recvBuffer[i]);
	}

	while(true)
	{
		// we received from all neighbours except one
		if(remainingNeighbours.size() == 1) {
			Message msg(Message::MT_Candidate, currentLeader, currentLeaderToken);
			printMsg("single remaining neighbour (" + std::to_string(remainingNeighbours[0]) + ") sending msg");
			world.send(remainingNeighbours[0], 0, msg);
		}
		// we received from everyone we know the leader
		else if(remainingNeighbours.size() == 0) {
			for(int neighbour : neighbours) {
				// TODO: dont always send to all neighbours
				printMsg("found a leader, sending election msg " + std::to_string(currentLeader));
				Message msg(Message::MT_LeaderFound, currentLeader, currentLeaderToken);
				world.isend(neighbour, 0, msg);
			}
			break;
		}

		// poll all neighbours
		for(int neighbour : remainingNeighbours) {

			// TODO: can we do it without the lock here? if two remain then they could get locked here
			if(remainingNeighbours.size() == 1) {
				printMsg("one neighbour remaining, waiting for answer");
				requests[neighbour].wait();
			}
			// skip if we didn't receive anything on the poll
			else if(requests[neighbour].test().get_ptr() == NULL) {
				printMsg("polling " + std::to_string(neighbour) + " without success");
				continue;
			}
			else
				printMsg("POLLING SUCCESS");


			const Message& msg = recvBuffer[neighbour];
			// received a message
			if(msg.token < currentLeaderToken || msg.token == currentLeaderToken && msg.leader < currentLeader) {
				printMsg("received a better candidate than current");
				currentLeader = msg.leader;
				currentLeaderToken = msg.token;
			}

			// we only ever receive one message
			// todo: we could solve this problem by using a copy of remainingneighbours...
			requests.erase(neighbour);
			remainingNeighbours.erase(std::remove(remainingNeighbours.begin(), remainingNeighbours.end(), neighbour), remainingNeighbours.end());
			break;
		}

		Sleep(1000);
	}

	return 0;
}