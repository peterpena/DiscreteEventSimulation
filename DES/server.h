#ifndef SERVER_H
#define SERVER_H
#define NONVALID -1;
#include <vector>

using namespace std;

class Server
{
    public:
        Server();
        virtual ~Server();
        int ServerID = NONVALID;
        int TotalJobsServed = 0;
        int NumCurrentJobs = 0;
        int Capacity = 0;
        int TotalJobsArrived = 0;
        int TotalOldJobsArrived = 0;
        int TotalJobsDenied = 0;
        int TotalArrivingJobsDenied = 0;
        int TotalArrivingJobs = 0;
        vector<int> JobCount;

        void initializeJobCount(int JobCountNum);
};

#endif // SERVER_H
