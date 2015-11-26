//Version 2
#include <iostream>
#include <queue>
#include "job.h"
#include "server.h"
#include <random>
#include <string>
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
    double perServerLoad = 4;
    double load = NumberOfServers * perServerLoad;
    double simRatio = 0.001;
    double Lambda = simRatio*load;
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

    for(i = 0; i < NumberOfServers; i++)
    {
        Servers[i].ServerID = i;
        Servers[i].Capacity = MaxServerCapacity;
        //Servers[i].NumCurrentJobs = perServerLoad;
        Servers[i].initializeJobCount(Servers[i].Capacity + 1);
        numberOfHops[i] = 0;
    }

    default_random_engine generator(0);
    exponential_distribution<double> lamdaDistribution(Lambda);
    exponential_distribution<double> muDistribution(Mu);

    cout<<"Number of Servers: "<<NumberOfServers<<endl;
    cout<<"Server Capacity: "<<MaxServerCapacity<<endl;
    cout<<"Mu: "<<Mu<<endl;
    cout<<"Lambda: "<<Lambda<<endl;

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

    while(!JobQueue.empty())
    {

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

                currentServer = (currentServer + 1) % NumberOfServers;

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
                currentServer = (currentServer+1) % NumberOfServers;
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


                        currentServer = (currentServer + 1) % NumberOfServers;
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
                        currentServer = (currentServer + 1) % NumberOfServers;
                    }
                }
            break;

        default:
            cout<<"ERROR: Job Status is Undefined....."<<endl;
            break;
        }
    }
    for(i = 0; i <NumberOfServers; i++)
    {
        totalJobsServed+= Servers[i].TotalJobsServed;
    }
    //cout<<"Number of new jobs: "<<i<<endl;
    cout<<"\nTotal Simulation Time: "<<simulatedClock<<endl;
    cout<<"Total Arrival Time:      "<<arrivalClock<<endl;
    cout<<"Total Service Time:      "<<serviceClock<<endl;
    cout<<"Average Service Time:    "<<serviceClock/totalJobsServed<<endl;
    cout<<"Inter Arrival Time:      "<<(arrivalClock/NJobsArrived)<<endl;
    // cout<<"Service Ratio:           "<<serviceClock/simulatedClock<<endl;
    cout<<"Number of Jobs Arrived:  "<<NJobsArrived<<endl;
    cout<<"Total Number of Jobs Served: "<<totalJobsServed<<endl;



    for(i = 0; i <numPrintServers; i++)
    {
        cout<<"\n\nServer "<<Servers[i].ServerID<<endl;
        cout<<"------------"<<endl;
        cout<<"Old and New Jobs Served:  "<<Servers[i].TotalJobsServed<<endl;
        cout<<"Old and New Jobs Denied:  "<<Servers[i].TotalJobsDenied + Servers[i].TotalArrivingJobsDenied<<endl;
        cout<<"New Arriving Jobs:        "<<Servers[i].TotalArrivingJobs<<endl;
        cout<<"New Arriving Jobs Served: "<<Servers[i].TotalArrivingJobs-Servers[i].TotalArrivingJobsDenied<<endl;
        cout<<"New Arriving Jobs Denied: "<<Servers[i].TotalArrivingJobsDenied<<endl;
        cout<<"Old Arriving Jobs:        "<<Servers[i].TotalOldJobsArrived<<endl;
        cout<<"Old Arriving Jobs Served: "<<Servers[i].TotalOldJobsArrived-Servers[i].TotalJobsDenied<<endl;
        cout<<"Old Arriving Jobs Denied: "<<Servers[i].TotalJobsDenied<<endl;

    }
    //cout<<"\nDrop Rate: "<<(double)DropJobs/NJobsArrived<<endl;
    cout<<"\nJobs Dropped: "<<DropJobs<<endl;
    for(i = 0; i < numPrintServers; i++)
    {
        totalArrivingJobs += Servers[i].TotalArrivingJobs;
    }
    //cout<<"\nServers greater than average"<<endl;
    //cout<<"----------------------------"<<endl;

    int averageArrivingJobs = (double)totalArrivingJobs / NumberOfServers;
    int serversGreaterThanAvg = 0;
    int totalJobs = 0;
    int totalStateFive = 0;
    double expectedValue = 0;
    for(i = 0; i < numPrintServers; i++)
    {
        if(Servers[i].TotalArrivingJobs > averageArrivingJobs)
        {
            serversGreaterThanAvg++;
            //cout<<"Server id: "<<i<<endl;

        }
    }
    //cout<<"Servers that surpassed avg: "<<(double)serversGreaterThanAvg/NumberOfServers<<endl;
    // cout<<"Total arriving jobs from all servers: " << totalArrivingJobs<<endl;

    double totalExpected = 0;
    int sumOfTotalJobs = 0;
    for(i = 0; i < NumberOfServers; i++)
    {
        if(i < numPrintServers){
            cout<<"\nServer "<<i<<endl;
            cout<<"----------"<<endl;
        }
        totalJobs = 0;
        expectedValue = 0;
        for(int j = 0; j < Servers[i].JobCount.size(); j++)
        {
            totalJobs += Servers[i].JobCount[j];

            if(j == 5)
            {
                totalStateFive += Servers[i].JobCount[j];
            }

        }
        sumOfTotalJobs += totalJobs;
        for(int k = 0; k < Servers[i].JobCount.size(); k++)
        {
            if(i < numPrintServers)
                cout<<"Server State "<< k<<": "<<Servers[i].JobCount[k]/(double)totalJobs<<endl;
            expectedValue += ((double)k * ((double)Servers[i].JobCount[k]/totalJobs));
        }

        if(i < numPrintServers){
            cout<<"Total Jobs: "<<totalJobs<<endl;

            cout<<"Expected Number of Jobs "<<expectedValue<<endl;
        }
    totalExpected+=expectedValue;
    }
    cout<<"The Average Expected Value: "<<totalExpected/NumberOfServers<<endl;
    cout<<"\n\nThe sum of all jobs.: "<<sumOfTotalJobs<<endl;
    totalStateFive = 0;
    for(i = 0; i < 1; i++)
    {
        for(int j = 0; j < Servers[i].JobCount.size(); j++)
        {
            if(j == 5)
                totalStateFive += Servers[i].JobCount[j];
        }
    }
    //cout<<"Total of State 5: "<<totalStateFive/(double)NJobsArrived<<endl;

    for(i = 0; i < NumberOfServers; i++)
    {
        // cout<<"Number of hops "<<i<<": "<<numberOfHops[i]/(double)NJobsArrived<<endl;
    }

    return 0;
}
