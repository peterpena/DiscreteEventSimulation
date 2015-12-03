#include "server.h"

Server::Server()
{
}

Server::~Server()
{
    //dtor
}

void Server::initializeJobCount(int JobCountNum)
{
    JobCount.resize(JobCountNum);

    for(int i = 0; i < JobCount.size(); i++){
        JobCount[i] = 0;
    }
}
