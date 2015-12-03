#ifndef JOB_H
#define JOB_H


class Job
{
    public:
        Job();
        Job(int s, double p, int ID){JobID = ID; AssignedServerID = s; PriorityTime = p;}
        double PriorityTime;
        double ArrivalTime;
        double ServiceTime;
        int JobID;
        int status;
        int AssignedServerID;
        int numberOfServersVisited = 0;

    protected:
    private:
};

#endif // JOB_H
