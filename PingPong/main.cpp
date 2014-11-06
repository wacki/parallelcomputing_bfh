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
    
    int rankOther = (world.rank() == 0) ? 1 : 0;
    const int maxMsg = 50;
    int msgCount = 0;
    bool sender;
    
    boost::timer t;
    double deltaT = 0.0f;
    double totalTime = 0.0f;
    
    char buffer[100000];
    buffer[323] = 'u';
    char rcvbuffer[100000];

    // 0 sends first
    sender = (world.rank() == 0);
    std::cout << world.rank() << " started as a " << ((sender) ? " sender " : " receiver") << std::endl;

    while (msgCount < maxMsg) {
        t.restart();
        if (sender) {
            msgCount++; 
            world.send(rankOther, 0, buffer);
        }
        else {
            world.recv(rankOther, 0, rcvbuffer);
            std::cout << "rcvd: " << rcvbuffer[323] << std::endl;
        }

        std::cout << "process " << world.rank() << ((sender) ? " sent a msg to " : " received a msg from ") << rankOther << " and blocked for: " << t.elapsed() << "s" << std::endl;

        totalTime += t.elapsed();        
        sender = !sender;
    }

    std::cout << maxMsg << " messages were sent taking a total of " << totalTime << "s" << std::endl;


    return 0;
}