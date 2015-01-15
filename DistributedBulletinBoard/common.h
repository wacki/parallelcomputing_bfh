
#pragma once

#include <boost/mpi.hpp>

// C style global variables, yea not nice in c++ whatever
extern boost::mpi::environment gEnv;
extern boost::mpi::communicator gWorld;
extern int gRank;
extern int gNumRM;
extern int gNumFE;