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
        int AssignedServerID;

    protected:
    private:
};

#endif // JOB_H
