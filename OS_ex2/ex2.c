/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex2
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

void SigAlarmHandler(int sig);
//global variables
pid_t pid1, pid2;

int main(int argc, char* argv[]) {
    //first argument- the total time of the game
    int totalTime = atoi(argv[1]);
    //in order to use pipe function
    int fileDes[2];
    //in order to use waitpid
    int stat1, stat2;
    //signal setters
    struct sigaction userAction;
    sigset_t blockMask;

    sigfillset(&blockMask);
    userAction.sa_handler = SigAlarmHandler;
    userAction.sa_mask = blockMask;
    userAction.sa_flags = 0;
    sigaction(SIGALRM, &userAction, NULL);

    //after `totalTime` we want the game to be ended by sigalrm
    alarm(totalTime);
    //in order to communicate and transfer information between two processes
    if (pipe(fileDes) == -1){
        perror("pipe error");
        exit(-1);
    }

    //in order to run another c file we must use fork()
    if ((pid1 = fork()) == 0) {//child process
        //`fileDes[0]` gets the `job` of `0` fd - stdin
        dup2(fileDes[0], 0);

        close(fileDes[1]);
        close(fileDes[0]);

        //execute the ex2_inp.out file
        if (execlp("./ex2_inp.out", "./ex2_inp.out", NULL) == -1){
            perror("execution of ex2_inp.out faild");
            exit(-1);
        }
    }
    if (pid1 == -1){
        perror("fork faild");
        exit(-1);
    }
    //creating another process in order to execute another c file
    if ((pid2 = fork()) == 0) {//child process
        //`fileDes[1]` gets the `job` of `1` fd - stdout
        dup2(fileDes[1], 1);

        close(fileDes[0]);
        close(fileDes[1]);

        //in order to convert from int to string
        char strPid[10];
        memset(strPid, '\0', 10);
        sprintf(strPid,"%d",pid1);
        //execute the ex2_upd.out file
        if (execlp("./ex2_upd.out", "./ex2_upd.out", strPid, NULL) == -1){
            perror("execution of ex2_upd.out faild");
            exit(-1);
        }
    }
    if (pid2 == -1){
        perror("fork faild");
        exit(-1);
    }
    //sleep(1);
    //waiting for the two son processes to terminate before returning
    waitpid(pid1, &stat1, 0);
    waitpid(pid2, &stat2, 0);
    //sleep(1);
    return 0;
}

/**********************************************************
 * function name: SigAlarmHandler.
 * the input: int sig.
 * the output: none.
 * the function operation: the function handels the `SIGALRM` aignal
 * by sending two `SIGINT` signals, one to each of the son processes,
 * in order to terminate the execution of the two son processes.
 *********************************************************/
void SigAlarmHandler(int sig){
    kill(pid1, SIGINT);
    kill(pid2, SIGINT);
}