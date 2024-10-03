#include "types.h"
#include "user.h"

/* ATTENTION STUDENTS: to ensure correct compilation of the base code, 
   stub functions for the system call user space wrapper functions are provided. 
   REMEMBER to disable the stub functions (by commenting the following macro) to 
   allow your implementation to work properly. */
/*
#define STUB_FUNCS
#ifdef STUB_FUNCS
void pause_scheduling(int pause) {}
int mlfq_set_allotment(int priority, int allotment) {return 0;}
#endif*/

#define CHILD_COUNT  3
#define SLEEP_TICKS  30

#define LOOP_CNT 0x02000000
#define SCHEDULER_DEFAULT 0 // RR is the default scheduler
#define SCHEDULER_MLFQ 1

void usage(char * bn)
{
    printf(1, "Usage: %s scheduler_type \n"
              "scheduler_type:\n"
              "\t0: Use the default xv6 scheduler\n"
              "\t1: Use the MLFQ scheduler\n", bn);
}

int computation(int n, volatile int loop)
{
    int tmp = 0;
    
    while(n < loop)
    {
        tmp += n;
        n++;
    }
            
	return tmp;
}

void create_child_processes(int with_rg_proc)
{
    int i = 0, j = 0;
    int pid = 0;

    // Create the desired number of child processes,
    //   have the child process wait for parenet's command of going ahead
    for (i = 0; i < CHILD_COUNT; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            printf(1, "fork() failed!\n");
            exit();
        }
        else if (pid == 0) // child
        {
            computation(j, LOOP_CNT);
            
            // only the one of the chile process creates the rg process
            if (with_rg_proc 
                && getpid() % CHILD_COUNT == 0)
            {
                int r = fork();

                if (r == 0)
                {
                    computation(j, LOOP_CNT);
                    computation(j, LOOP_CNT);
                    exit();
                }
                else if (r > 0)
                {
                    wait();
                }
                else
                {
                    printf(1, "Generating RG process failed!\n");
                    exit();
                }
            }
            
            computation(j, LOOP_CNT);
            exit();
        }
        else // parent
        {      
            printf(1, "Parent: child (pid=%d) created!\n", pid);
        }
    }

    printf(1, "\n");
}

void wait_child_processes(void)
{
    int i = 0;

    for (i = 0; i < CHILD_COUNT; i++)
    {   
        if (wait() < 0)
        {
            printf(1, "wait() on child-%d failed!\n", i);
        }
    }

}

// with_rg_proc: with runtime generated process
void run_test(int scheduler_type, int with_rg_proc)
{    
    enable_sched_trace(1);

    set_sched(scheduler_type); 
    
    pause_scheduling(1);
    create_child_processes(with_rg_proc);
    pause_scheduling(0);
    wait_child_processes();

    enable_sched_trace(0);

    printf(1, "\n");
}

int
main(int argc, char *argv[])
{
    printf(1, "\n>>>>> Test case 1: testing default scheduler (RR) ...\n");
    run_test(SCHEDULER_DEFAULT, 0);

    printf(1, "\n\n>>>>> Test case 2: testing MLFQ scheduler with default allotment ...\n");
    run_test(SCHEDULER_MLFQ, 0);

    printf(1, "\n\n>>>>> Test case 3: testing default scheduler (RR) ..\n");
    run_test(SCHEDULER_DEFAULT, 0);

    printf(1, "\n\n>>>>> Test case 4: testing MLFQ scheduler with runtime generated process ...\n");
    run_test(SCHEDULER_MLFQ, 1);


    printf(1, "\n\n>>>>> Test case 5: testing MLFQ scheduler with new allotments ...\n");
    mlfq_set_allotment(3, 4);
    mlfq_set_allotment(2, 8);
    run_test(SCHEDULER_MLFQ, 0);
    // Change allotments back to default values
    mlfq_set_allotment(3, 2);
    mlfq_set_allotment(2, 4);
    exit();
}
