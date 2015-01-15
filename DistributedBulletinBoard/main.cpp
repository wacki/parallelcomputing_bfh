#include <boost/mpi.hpp>
#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>

#include "common.h"
#include "Frontend.h"
#include "ReplicaManager.h"
#include "Client.h"
#include "TSVec.h"
#include "Log.h"

namespace mpi = boost::mpi;

boost::mpi::environment gEnv;
boost::mpi::communicator gWorld;
int gRank;
int gNumRM;
int gNumFE;

int main()
{
  int numClients = 2;
  
  // client settings
  float minClientActionDelay = 0.2f;	// min wait time for a client before starting new actions
  float maxClientActionDelay = 0.4f;	// max wait time for a client before it has to complete a new action
  float clientQueryWeight = 30.0f;		// possibility of a query to happen
  float clientReplyWeight = 30.0f;		// possibility of a reply to happen
  float clientPostWeight = 40.0f;		// possibility of a new post (update) to happen
  
  // set the global MPI variabes
  gRank = gWorld.rank();
  gNumFE = numClients;
  gNumRM = gWorld.size() - numClients;

  // early out if there are not enough nodes for at least one RM
  if(numClients + 1 > gWorld.size() && gWorld.rank() == 0) {
	  std::cout << "ERROR: there are not enough nodes for at least 1 RM, please increase the number of nodes" << std::endl;
	  exit(-1);
  }

  if (gWorld.rank() == 0) {
      std::cout << " num RM: " << gNumRM << " num FE: " << gNumFE << std::endl;
  }

    Log::singleton().open("Bulletin_" + std::to_string(gRank) + ".log"); // set log file
    Log::singleton().setVerbosity(LV_Normal);

  //the last 'numClients' ranks are front ends
  if (gWorld.rank() >= gWorld.size() - numClients) {
	  std::cout << "P" << gWorld.rank() << ": assigned as a client" << std::endl;

	  // create client instance
	  Client client;

	  // set client variables as defined above
	  client.setMinActionDelay(minClientActionDelay);
	  client.setMaxActionDelay(maxClientActionDelay);
	  client.setQueryWeight(clientQueryWeight);
	  client.setReplyWeight(clientReplyWeight);
	  client.setPostWeight(clientPostWeight);

	  // run the client
	  // the client will now call the Frontend classes specific functions
	  // whenever it wants to complete an action.
	  client.run();
  }
  else
  {
	  std::cout << "P" << gWorld.rank() << ": assigned as a replicator" << std::endl;
      ReplicaManager RM;

      RM.run();
  }

  return 0;
}