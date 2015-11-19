#include <iostream>
#include <queue>
#include "job.h"
#include "server.h"
#include <random>
#include <string>
#define NEWJOB -1
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
    int NJobsArrived = 10000000;
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
    int lastServer = 0;
    bool FoundServer = false;
    int totalArrivingJobs = 0;

    for(i = 0; i < NumberOfServers; i++)
    {
        Servers[i].ServerID = i;
        Servers[i].Capacity = MaxServerCapacity;
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
    JobQueue.push(NewJob);


    //Fill Initial Queue
    for(i = 0; i< NJobsArrived-1; i++)
    {
        double pastTime = clock;
        double arrivalTime = lamdaDistribution(generator) + clock;
        clock = arrivalTime;
        Job NewJob(NEWJOB, arrivalTime, i + 1);
        NewJob.JobID = i+1;
        NewJob.ArrivalTime = arrivalTime - pastTime;

        JobQueue.push(NewJob);
    }
    i = 0;
    while(!JobQueue.empty())
    {
        Job CurrentJob = JobQueue.top();
        JobQueue.pop();
        //string eventType = (CurrentJob.AssignedServerID == NEWJOB) ? " Arrival" : " Departure";
        //cout<<"Job "<<CurrentJob.JobID<<": "<<CurrentJob.PriorityTime<<eventType<<endl;

       if(CurrentJob.AssignedServerID == NEWJOB)
       {
           simulatedClock += CurrentJob.ArrivalTime;
           arrivalClock += CurrentJob.ArrivalTime;
           FoundServer = false;
           int loopCount = 0;
           int currentServer = (lastServer + 1) % NumberOfServers;
           Servers[currentServer].TotalArrivingJobs++;

            if(Servers[currentServer].NumCurrentJobs < Servers[currentServer].Capacity)
            {
                numberOfHops[0]++;
                CurrentJob.AssignedServerID = Servers[currentServer].ServerID;
                serverTime = muDistribution(generator);
                CurrentJob.PriorityTime += serverTime;
                CurrentJob.ServiceTime = serverTime;
                JobQueue.push(CurrentJob);
                Servers[currentServer].NumCurrentJobs++;
                Servers[currentServer].JobCount[(Servers[currentServer].NumCurrentJobs)]++;
                lastServer = Servers[currentServer].ServerID;
            }
            else if(Servers[currentServer].NumCurrentJobs >= Servers[currentServer].Capacity)
            {
                Servers[currentServer].JobCount[(Servers[currentServer].NumCurrentJobs)]++;
            }
            else
            {
              Servers[currentServer].TotalArrivingJobsDenied++;
              while((!FoundServer) && (loopCount < NumberOfServers))
              {
                  currentServer = (currentServer + 1) % NumberOfServers;
                  Servers[currentServer].JobCount[(Servers[currentServer].NumCurrentJobs)]++;
                  Servers[currentServer].TotalOldJobsArrived++;
                   loopCount++;
                   if(Servers[currentServer].NumCurrentJobs < Servers[currentServer].Capacity)
                   {
                       numberOfHops[loopCount]++;
                       FoundServer = true;
                       CurrentJob.AssignedServerID = Servers[currentServer].ServerID;
                       serverTime = muDistribution(generator);
                       CurrentJob.PriorityTime += serverTime;
                       CurrentJob.ServiceTime = serverTime;
                       JobQueue.push(CurrentJob);
                       Servers[currentServer].NumCurrentJobs++;
                       lastServer = Servers[currentServer].ServerID;
                   }
                   else
                   {
                       //cout<<"Job "<<CurrentJob.JobID<<" denied by Server "<<Servers[currentServer].ServerID<<endl;
                       Servers[currentServer].TotalJobsDenied++;
                   }
             }
            }

           if(loopCount == NumberOfServers)
            {
                DropJobs++;
            }
       }
       else
       {
           simulatedClock += CurrentJob.ServiceTime;
           serviceClock+= CurrentJob.ServiceTime;
           //cout<<"Job Finished: "<<CurrentJob.JobID<<" with Server "<<Servers[CurrentJob.AssignedServerID].ServerID<<endl;
           //cout<<"Job Server ID: "<<CurrentJob.AssignedServerID<<endl;
            //cout<<"Server "<<Servers[CurrentJob.AssignedServerID].ServerID<< " Job Count: "<<Servers[CurrentJob.AssignedServerID].NumCurrentJobs<<endl;
            Servers[CurrentJob.AssignedServerID].TotalJobsServed++;
            Servers[CurrentJob.AssignedServerID].NumCurrentJobs--;

            /*if(Servers[CurrentJob.AssignedServerID].NumCurrentJobs == 0)
            {
               Servers[CurrentJob.AssignedServerID].JobCount[Servers[CurrentJob.AssignedServerID].NumCurrentJobs]++;
            }*/
            if(Servers[CurrentJob.AssignedServerID].NumCurrentJobs < 0)
            {
                cout<<"ERROR: Negative Job Count..."<<endl;
            }
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
    for(i = 0; i < numPrintServers; i++){
        totalArrivingJobs += Servers[i].TotalArrivingJobs;
    }
    //cout<<"\nServers greater than average"<<endl;
    //cout<<"----------------------------"<<endl;

    int averageArrivingJobs = (double)totalArrivingJobs / NumberOfServers;
    int serversGreaterThanAvg = 0;
    int totalJobs = 0;
    int totalStateFive = 0;
    double expectedValue = 0;
        for(i = 0; i < numPrintServers; i++){
            if(Servers[i].TotalArrivingJobs > averageArrivingJobs){
                serversGreaterThanAvg++;
                //cout<<"Server id: "<<i<<endl;

            }
    }
    //cout<<"Servers that surpassed avg: "<<(double)serversGreaterThanAvg/NumberOfServers<<endl;
   // cout<<"Total arriving jobs from all servers: " << totalArrivingJobs<<endl;


    for(i = 0; i < numPrintServers; i++)
    {
        cout<<"\nServer "<<i<<endl;
        cout<<"----------"<<endl;
        totalJobs = 0;
        expectedValue = 0;
        for(int j = 0; j < Servers[i].JobCount.size(); j++)
        {
            totalJobs += Servers[i].JobCount[j];

            if(j == 5)
                totalStateFive += Servers[i].JobCount[j];
        }
        for(int k = 0; k < Servers[i].JobCount.size(); k++)
        {
            cout<<"Server State "<< k<<": "<<Servers[i].JobCount[k]/(double)totalJobs<<endl;
            expectedValue += ((double)k * ((double)Servers[i].JobCount[k]/totalJobs));
        }

        cout<<"Total Jobs: "<<totalJobs<<endl;

        cout<<"Expected Number of Jobs "<<expectedValue<<endl;

    }

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
