//Version 2
#include <iostream>
#include <queue>
#include "job.h"
#include "server.h"
#include <random>
#include <string>
#include <fstream>
#define NEWJOB -1
#define OLDJOB -2
#define DEPARTURE -3

using namespace std;

class TimeComparison
{
public:
    bool operator() (const Job&lhs, const Job &rhs) const
    {
        return (lhs.PriorityTime>rhs.PriorityTime);
    }
};

int main()
{
    priority_queue<Job, vector<Job>, TimeComparison> JobQueue;
    int DropJobs = 0;
    int NumberOfServers = 100;
    int numberOfHops[NumberOfServers];
    int NJobsServed[NumberOfServers];
    int NJobsArrived = 1000000;
    int NWarmUpJobs = 100000;
    int numPrintServers = 2;
    double perServerLoad = 2;
    double load = NumberOfServers * perServerLoad;
    double simRatio = 0.001;
    double Lambda = simRatio * load;
    double Mu = simRatio;
    int MaxServerCapacity = 5;
    Server Servers[NumberOfServers];
    int i;
    double clock;
    double simulatedClock = 0;
    double totalJobsServed = 0;
    double arrivalClock = 0;
    double serviceClock = 0;
    double serverTime = 0;
    int currentServer = 0;
    int warmUpJobsCounter = 0;
    bool startStatistics = false;
    bool FoundServer = false;
    int numVisitedServers = 0;
    int totalArrivingJobs = 0;
    int ServiceBins[20];
    int ArrivalBins[20];

    for(i = 0; i < 20; i++)
    {
        ServiceBins[i] = 0;
        ArrivalBins[i] = 0;
    }

    for(i = 0; i < NumberOfServers; i++)
    {
        Servers[i].ServerID = i;
        Servers[i].Capacity = MaxServerCapacity;
        //Servers[i].NumCurrentJobs = perServerLoad;
        Servers[i].initializeJobCount(Servers[i].Capacity + 1);
        numberOfHops[i] = 0;
    }

    default_random_engine generator(100);
    exponential_distribution<double> lamdaDistribution(Lambda);
    exponential_distribution<double> muDistribution(Mu);

    cout<<"Number of Servers: "<<NumberOfServers<<endl;
    cout<<"Server Capacity: "<<MaxServerCapacity<<endl;
    cout<<"Mu: "<<Mu<<endl;
    cout<<"Lambda: "<<Lambda<<endl;
    cout<<"Lambda : Mu Ratio : "<<Lambda/Mu<<endl;

    //intial job
    clock = 0;
    Job NewJob(NEWJOB, clock, 0);
    NewJob.JobID = 0;
    NewJob.status = NEWJOB;
    JobQueue.push(NewJob);


    //Fill Initial Queue
    for(i = 0; i< NJobsArrived-1; i++)
    {
        double pastTime = clock;
        double arrivalTime = lamdaDistribution(generator) + clock;
        clock = arrivalTime;
        Job NewJob(NEWJOB, arrivalTime, i + 1);
        NewJob.JobID = i+1;
        NewJob.status = NEWJOB;
        NewJob.ArrivalTime = arrivalTime - pastTime;

        JobQueue.push(NewJob);
    }
    int numberOfJobs = 0, lastServer = currentServer, lastSuccessful = 0, currentSuccessful = 0;
    while(!JobQueue.empty())
    {
        if((lastSuccessful - currentSuccessful) > 1 || (lastSuccessful - currentSuccessful) < -1)
        {
            numberOfJobs++;
            cout<<"Current Server: "<<currentSuccessful<<" Last Server: "<<lastSuccessful<<endl;
        }
        Job CurrentJob = JobQueue.top();
        JobQueue.pop();

        switch(CurrentJob.status)
        {

        case DEPARTURE:
            simulatedClock += CurrentJob.ServiceTime;
            serviceClock+= CurrentJob.ServiceTime;
            Servers[CurrentJob.AssignedServerID].NumCurrentJobs--;

            Servers[CurrentJob.AssignedServerID].TotalJobsServed++;
            Servers[CurrentJob.AssignedServerID].JobCount[Servers[CurrentJob.AssignedServerID].NumCurrentJobs]++;

            if(Servers[CurrentJob.AssignedServerID].NumCurrentJobs < 0)
            {
                cout<<"ERROR: Negative Job Count..."<<endl;
            }
            break;

        case NEWJOB:
                arrivalClock += CurrentJob.ArrivalTime;
                Servers[currentServer].TotalArrivingJobs++;

            if(Servers[currentServer].NumCurrentJobs == Servers[currentServer].Capacity)
            {
                CurrentJob.status = OLDJOB;
                JobQueue.push(CurrentJob);
                Servers[currentServer].JobCount[Servers[currentServer].NumCurrentJobs]++;
                Servers[currentServer].TotalArrivingJobsDenied++;
                lastServer = currentServer;
                currentServer = (currentServer + 1) % NumberOfServers;
                while(Servers[currentServer].NumCurrentJobs == Servers[currentServer].Capacity)
                {
                    currentServer = (currentServer + 1) % NumberOfServers;
                }
            }
            else
            {
                CurrentJob.status = DEPARTURE;
                CurrentJob.AssignedServerID = Servers[currentServer].ServerID;
                serverTime = muDistribution(generator);
                CurrentJob.PriorityTime += serverTime;
                CurrentJob.ServiceTime = serverTime;
                JobQueue.push(CurrentJob);
                Servers[currentServer].NumCurrentJobs++;
                numberOfHops[0]++;
                Servers[currentServer].JobCount[(Servers[currentServer].NumCurrentJobs)]++;
                Servers[currentServer].Jobs.push_back(CurrentJob);
                lastServer = currentServer;
                lastSuccessful = currentSuccessful;
                currentSuccessful = currentServer;
                currentServer = (currentServer+1) % NumberOfServers;
                while(Servers[currentServer].NumCurrentJobs == Servers[currentServer].Capacity)
                {
                    currentServer = (currentServer + 1) % NumberOfServers;
                }
            }
            break;

        case OLDJOB:
                CurrentJob.numberOfServersVisited++;

                Servers[currentServer].TotalOldJobsArrived++;

                if(CurrentJob.numberOfServersVisited > NumberOfServers)
                {
                    DropJobs++;
                }
                else
                {
                    if(Servers[currentServer].NumCurrentJobs == Servers[currentServer].Capacity)
                    {

                        Servers[currentServer].JobCount[Servers[currentServer].NumCurrentJobs]++;
                        Servers[currentServer].TotalJobsDenied++;
                        JobQueue.push(CurrentJob);

                        lastServer = currentServer;
                        currentServer = (currentServer + 1) % NumberOfServers;
                        while(Servers[currentServer].NumCurrentJobs == Servers[currentServer].Capacity)
                        {
                            currentServer = (currentServer + 1) % NumberOfServers;
                        }
                    }
                    else if(Servers[currentServer].NumCurrentJobs < Servers[currentServer].Capacity)
                    {
                        CurrentJob.status = DEPARTURE;
                        CurrentJob.AssignedServerID = Servers[currentServer].ServerID;
                        serverTime = muDistribution(generator);
                        CurrentJob.PriorityTime += serverTime;
                        CurrentJob.ServiceTime = serverTime;
                        JobQueue.push(CurrentJob);
                        Servers[currentServer].NumCurrentJobs++;
                        numberOfHops[CurrentJob.numberOfServersVisited]++;
                        Servers[currentServer].JobCount[(Servers[currentServer].NumCurrentJobs)]++;
                        Servers[currentServer].Jobs.push_back(CurrentJob);
                        lastServer = currentServer;
                        lastSuccessful = currentSuccessful;
                        currentSuccessful = currentServer;
                        currentServer = (currentServer + 1) % NumberOfServers;
                        while(Servers[currentServer].NumCurrentJobs == Servers[currentServer].Capacity)
                        {
                            currentServer = (currentServer + 1) % NumberOfServers;
                        }
                    }
                }
            break;

        default:
            cout<<"ERROR: Job Status is Undefined....."<<endl;
            break;
        }
    }
    double maxNum = 0, maxInterval;

    for(i = 0; i < Servers[0].Jobs.size(); i++)
    {
        if(Servers[0].Jobs[i].ServiceTime > maxNum)
        {
            maxNum = Servers[0].Jobs[i].ServiceTime;
        }
    }

    maxInterval = maxNum/20;

    for(i = 0; i < Servers[0].Jobs.size(); i++)
    {
        int binNumber = Servers[0].Jobs[i].ServiceTime /maxInterval;
        if(binNumber < 20)
        {
            ServiceBins[binNumber]++;
        }

    }
    maxNum = 0;
    for(i = 0; i < Servers[0].Jobs.size(); i++)
    {
        if(Servers[0].Jobs[i].ArrivalTime > maxNum)
        {
            maxNum = Servers[0].Jobs[i].ArrivalTime;
        }
    }

    maxInterval = maxNum/20;

    for(i = 0; i < Servers[0].Jobs.size(); i++)
    {
        int binNumber = Servers[0].Jobs[i].ArrivalTime /maxInterval;
        if(binNumber < 20)
        {
            ArrivalBins[binNumber]++;
        }

    }
    ofstream ServiceFile, ArrivalFile;
    ServiceFile.open("ServiceTime.dat");
    ArrivalFile.open("ArrivalTime.dat");
    for(i = 0; i < 20; i++)
    {
        ServiceFile<<i/2.0<<" "<<ServiceBins[i]<<endl;
        ArrivalFile<<i/2.0<<" "<<ArrivalBins[i]<<endl;

    }
    ServiceFile.close();
    ArrivalFile.close();
    return 0;
}

