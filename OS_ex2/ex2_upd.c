/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex2_upd
 *********************************************************/

#include <stdio.h>
#include <sys/fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define SIZE 64

void CreatBoard(int pid);
void WriteToStdout();
void DefaultAction(int sig);
void UpdateBoardLeft();
void UpdateBoardRight();
void UpdateBoardUp();
void UpdateBoardDown();
int WinFunc();
int LoseFunc();
void SigintHandler(int sig);

//global variables
//will hold the game board
int newboard[16];
//will hold the current waiting time
int randomWait;
//will hold the pid of the first process
int pid;
int flag = 1;

int main(int argc, char* argv[]){
    //first argument- the pid of the first process
    pid = atoi(argv[1]);
    //in order to improve `rand` function
    time_t t;
    srand((unsigned) time(&t));
    //will get the player`s keyboard input
    char playerInput;
    //signal setters
    struct sigaction userAction;
    sigset_t blockMask;

    sigfillset(&blockMask);
    userAction.sa_handler = DefaultAction;
    userAction.sa_mask = blockMask;
    userAction.sa_flags = 0;
    sigaction(SIGALRM, &userAction, NULL);

    userAction.sa_handler = SigintHandler;
    sigaction(SIGINT, &userAction, NULL);
    //create new board
    CreatBoard(pid);
    //in order to keep the program running until SIGINT signal
    do{
        playerInput = '\0';
        //if the player wont enter any key, we want some default action to happen
        alarm(randomWait);
        //in order to press the key with no need to press `enter`
        system("stty cbreak -echo");
        playerInput = (char)getchar();
        system("stty cooked echo");
        //checking the player`s input and calling the right function
        switch (playerInput){
            case 'A':
                alarm(0);
                UpdateBoardLeft();
                break;
            case 'D':
                alarm(0);
                UpdateBoardRight();
                break;
            case 'W':
                alarm(0);
                UpdateBoardUp();
                break;
            case 'X':
                alarm(0);
                UpdateBoardDown();
                break;
            case 'S':
                alarm(0);
                CreatBoard(pid);
                break;
            default:
                alarm(0);
                break;
        }
        if (WinFunc() || LoseFunc()){
            flag = 0;
            kill(pid, SIGUSR1);
        }
    }while(flag);
    return 0;
}


/**********************************************************
 * function name: CreatBoard.
 * the input: int pid.
 * the output: none.
 * the function operation: the function creats a new board by
 * set `newboard` to zeros, choose a new `randomWait` value,
 * choose two random indexes, put the number 2 in `newboard`
 * in the place of the two indexes, call `WriteToStdout` function,
 * and send a `SIGUSR1` to the first process.
 *********************************************************/
void CreatBoard(int pid){
    int i, j, k;
    for(k = 0; k < 16; k++){
        newboard[k] = 0;
    }
    randomWait = ((rand() % 5) + 1);
    i = (rand() % 16);
    j = (rand() % 16);
    //in order to chose two different indexes
    while(i == j){
       j = (rand() % 16);
    }
    newboard[i] = 2;
    newboard[j] = 2;
    WriteToStdout();
    kill(pid, SIGUSR1);
}

/**********************************************************
 * function name: WriteToStdout.
 * the input: none.
 * the output: none.
 * the function operation: the function takes the `newboard` variable,
 * creates the `line format` in the `newBoardStr` variable,
 * and writes the result to stdout.
 *********************************************************/
void WriteToStdout(){
    int k;
    char numStr[4];
    char newBoardStr[120];
    memset(newBoardStr, '\0', 120);
    int writeRes;
    for (k = 0; k < 16; k++){
        memset(numStr, '\0', 4);
        sprintf(numStr, "%d", newboard[k]);
        strcat(newBoardStr, numStr);
        if (k != 15){
            strcat(newBoardStr, ",");
        }
    }
    writeRes = write(1, newBoardStr, strlen(newBoardStr));
    if (writeRes < 0){
        perror("write faild");
        exit(-1);
    }
}

/**********************************************************
 * function name: DefaultAction.
 * the input: int sig.
 * the output: none.
 * the function operation: the function handels the `SIGALRM` aignal
 * by choosing one random index, put the number 2 in `newboard` in the place
 * of this index, choose a new `randomWait` value, call `WriteToStdout` function,
 * and send a `SIGUSR1` to the first process.
 *********************************************************/
void DefaultAction(int sig){
    int i;
    do{
        i = (rand() % 16);
    }while(newboard[i] != 0);
    newboard[i] = 2;
    randomWait = ((rand() % 5) + 1);
    WriteToStdout(newboard);
    kill(pid, SIGUSR1);
}

/**********************************************************
 * function name: UpdateBoardLeft.
 * the input: none.
 * the output: none.
 * the function operation: the function handels left movement,
 * choose a new `randomWait` value, call `WriteToStdout` function,
 * and send a `SIGUSR1` to the first process.
 *********************************************************/
void UpdateBoardLeft(){
    int i, j, k, d = 0;
    for (i = 0; i < 16; i += 4) {
        for (j = 0; j < 3; j++) {
            if (newboard[i + j] == 0) {
                if (j == 0) {
                    if (newboard[i + j + 1] != 0 || newboard[i + j + 2] != 0 ||
                        newboard[i + j + 3] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < (3 - j); k++) {
                                newboard[i + j + k] = newboard[i + j + k + 1];
                            }
                            newboard[i + j + k] = 0;
                        }
                    }
                }
                if (j == 1) {
                    if (newboard[i + j + 1] != 0 || newboard[i + j + 2] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < (3 - j); k++) {
                                newboard[i + j + k] = newboard[i + j + k + 1];
                            }
                            newboard[i + j + k] = 0;
                        }
                    }
                }
                if (j == 2) {
                    if (newboard[i + j + 1] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < (3 - j); k++) {
                                newboard[i + j + k] = newboard[i + j + k + 1];
                            }
                            newboard[i + j + k] = 0;
                        }
                    }
                }
            }
            if (newboard[i + j] != 0) {
                if (j == 0) {
                    if (newboard[i + j] == newboard[i + j + 1]) {
                        newboard[i + j] += newboard[i + j + 1];
                        newboard[i + j + 1] = newboard[i + j + 2];
                        newboard[i + j + 2] = newboard[i + j + 3];
                        newboard[i + j + 3] = 0;
                    }
                    if (newboard[i + j + 1] == 0 && newboard[i + j] == newboard[i + j + 2]) {
                        newboard[i + j] += newboard[i + j + 2];
                        newboard[i + j + 1] = newboard[i + j + 3];
                        newboard[i + j + 2] = 0;
                        newboard[i + j + 3] = 0;
                    }
                    if (newboard[i + j + 1] == 0 && newboard[i + j + 2] == 0 && newboard[i + j] == newboard[i + j + 3]) {
                        newboard[i + j] += newboard[i + j + 3];
                        newboard[i + j + 1] = 0;
                        newboard[i + j + 2] = 0;
                        newboard[i + j + 3] = 0;
                    }
                }
                if (j == 1) {
                    if (newboard[i + j] == newboard[i + j + 1]) {
                        newboard[i + j] += newboard[i + j + 1];
                        newboard[i + j + 1] = newboard[i + j + 2];
                        newboard[i + j + 2] = 0;
                    }
                    if (newboard[i + j + 1] == 0 && newboard[i + j] == newboard[i + j + 2]) {
                        newboard[i + j] += newboard[i + j + 2];
                        newboard[i + j + 1] = 0;
                        newboard[i + j + 2] = 0;
                    }
                }
                if (j == 2) {
                    if (newboard[i + j] == newboard[i + j + 1]) {
                        newboard[i + j] += newboard[i + j + 1];
                        newboard[i + j + 1] = 0;
                    }
                }
            }
        }
    }
    randomWait = ((rand() % 5) + 1);
    WriteToStdout();
    kill(pid, SIGUSR1);
}

/**********************************************************
 * function name: UpdateBoardLeft.
 * the input: none.
 * the output: none.
 * the function operation: the function handels right movement,
 * choose a new `randomWait` value, call `WriteToStdout` function,
 * and send a `SIGUSR1` to the first process.
 *********************************************************/
void UpdateBoardRight(){
    int i, j, k, d = 0;
    for (i = 0; i < 16; i += 4) {
        for (j = 3; j > 0; j--) {
            if (newboard[i + j] == 0) {
                if (j == 3) {
                    if (newboard[i + j - 1] != 0 || newboard[i + j - 2] != 0 ||
                        newboard[i + j - 3] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 3; k++) {
                                newboard[i + j - k] = newboard[i + j - k - 1];
                            }
                            newboard[i + j - k] = 0;
                        }
                    }
                }
                if (j == 2) {
                    if (newboard[i + j - 1] != 0 || newboard[i + j - 2] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 2; k++) {
                                newboard[i + j - k] = newboard[i + j - k - 1];
                            }
                            newboard[i + j - k] = 0;
                        }
                    }
                }
                if (j == 1) {
                    if (newboard[i + j - 1] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 1; k++) {
                                newboard[i + j - k] = newboard[i + j - k - 1];
                            }
                            newboard[i + j - k] = 0;
                        }
                    }
                }
            }
            if (newboard[i + j] != 0) {
                if (j == 3) {
                    if (newboard[i + j] == newboard[i + j - 1]) {
                        newboard[i + j] += newboard[i + j - 1];
                        newboard[i + j - 1] = newboard[i + j - 2];
                        newboard[i + j - 2] = newboard[i + j - 3];
                        newboard[i + j - 3] = 0;
                    }
                    if (newboard[i + j - 1] == 0 && newboard[i + j] == newboard[i + j - 2]) {
                        newboard[i + j] += newboard[i + j - 2];
                        newboard[i + j - 1] = newboard[i + j - 3];
                        newboard[i + j - 2] = 0;
                        newboard[i + j - 3] = 0;
                    }
                    if (newboard[i + j - 1] == 0 && newboard[i + j - 2] == 0 && newboard[i + j] == newboard[i + j - 3]) {
                        newboard[i + j] += newboard[i + j - 3];
                        newboard[i + j - 1] = 0;
                        newboard[i + j - 2] = 0;
                        newboard[i + j - 3] = 0;
                    }
                }
                if (j == 2) {
                    if (newboard[i + j] == newboard[i + j - 1]) {
                        newboard[i + j] += newboard[i + j - 1];
                        newboard[i + j - 1] = newboard[i + j - 2];
                        newboard[i + j - 2] = 0;
                    }
                    if (newboard[i + j - 1] == 0 && newboard[i + j] == newboard[i + j - 2]) {
                        newboard[i + j] += newboard[i + j - 2];
                        newboard[i + j - 1] = 0;
                        newboard[i + j - 2] = 0;
                    }
                }
                if (j == 1) {
                    if (newboard[i + j] == newboard[i + j - 1]) {
                        newboard[i + j] += newboard[i + j - 1];
                        newboard[i + j - 1] = 0;
                    }
                }
            }
        }
    }
    randomWait = ((rand() % 5) + 1);
    WriteToStdout();
    kill(pid, SIGUSR1);
}

/**********************************************************
 * function name: UpdateBoardLeft.
 * the input: none.
 * the output: none.
 * the function operation: the function handels up movement,
 * choose a new `randomWait` value, call `WriteToStdout` function,
 * and send a `SIGUSR1` to the first process.
 *********************************************************/
void UpdateBoardUp(){
    int i, j, k, d = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 12; j+=4) {
            if (newboard[i + j] == 0) {
                if (j == 0) {
                    if (newboard[i + j + 4] != 0 || newboard[i + j + 8] != 0 ||
                        newboard[i + j + 12] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 12; k+=4) {
                                newboard[i + j + k] = newboard[i + j + k + 4];
                            }
                            newboard[i + j + k] = 0;
                        }
                    }
                }
                if (j == 4) {
                    if (newboard[i + j + 4] != 0 || newboard[i + j + 8] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 8; k+=4) {
                                newboard[i + j + k] = newboard[i + j + k + 4];
                            }
                            newboard[i + j + k] = 0;
                        }
                    }
                }
                if (j == 8) {
                    if (newboard[i + j + 4] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 4; k+=4) {
                                newboard[i + j + k] = newboard[i + j + k + 4];
                            }
                            newboard[i + j + k] = 0;
                        }
                    }
                }
            }
            if (newboard[i + j] != 0) {
                if (j == 0) {
                    if (newboard[i + j] == newboard[i + j + 4]) {
                        newboard[i + j] += newboard[i + j + 4];
                        newboard[i + j + 4] = newboard[i + j + 8];
                        newboard[i + j + 8] = newboard[i + j + 12];
                        newboard[i + j + 12] = 0;
                    }
                    if (newboard[i + j + 4] == 0 && newboard[i + j] == newboard[i + j + 8]) {
                        newboard[i + j] += newboard[i + j + 8];
                        newboard[i + j + 4] = newboard[i + j + 12];
                        newboard[i + j + 8] = 0;
                        newboard[i + j + 12] = 0;
                    }
                    if (newboard[i + j + 4] == 0 && newboard[i + j + 8] == 0 && newboard[i + j] == newboard[i + j + 12]) {
                        newboard[i + j] += newboard[i + j + 12];
                        newboard[i + j + 4] = 0;
                        newboard[i + j + 8] = 0;
                        newboard[i + j + 12] = 0;
                    }
                }
                if (j == 4) {
                    if (newboard[i + j] == newboard[i + j + 4]) {
                        newboard[i + j] += newboard[i + j + 4];
                        newboard[i + j + 4] = newboard[i + j + 8];
                        newboard[i + j + 8] = 0;
                    }
                    if (newboard[i + j + 4] == 0 && newboard[i + j] == newboard[i + j + 8]) {
                        newboard[i + j] += newboard[i + j + 8];
                        newboard[i + j + 4] = 0;
                        newboard[i + j + 8] = 0;
                    }
                }
                if (j == 8) {
                    if (newboard[i + j] == newboard[i + j + 4]) {
                        newboard[i + j] += newboard[i + j + 4];
                        newboard[i + j + 4] = 0;
                    }
                }
            }
        }
    }
    randomWait = ((rand() % 5) + 1);
    WriteToStdout();
    kill(pid, SIGUSR1);
}

/**********************************************************
 * function name: UpdateBoardLeft.
 * the input: none.
 * the output: none.
 * the function operation: the function handels down movement,
 * choose a new `randomWait` value, call `WriteToStdout` function,
 * and send a `SIGUSR1` to the first process.
 *********************************************************/
void UpdateBoardDown(){
    int i, j, k, d = 0;
    for (i = 0; i < 4; i++) {
        for (j = 12; j > 0; j-=4) {
            if (newboard[i + j] == 0) {
                if (j == 12) {
                    if (newboard[i + j - 4] != 0 || newboard[i + j - 8] != 0 ||
                        newboard[i + j - 12] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 12; k+=4) {
                                newboard[i + j - k] = newboard[i + j - k - 4];
                            }
                            newboard[i + j - k] = 0;
                        }
                    }
                }
                if (j == 8) {
                    if (newboard[i + j - 4] != 0 || newboard[i + j - 8] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 8; k+=4) {
                                newboard[i + j - k] = newboard[i + j - k - 4];
                            }
                            newboard[i + j - k] = 0;
                        }
                    }
                }
                if (j == 4) {
                    if (newboard[i + j - 4] != 0) {
                        while (newboard[i + j] == 0) {
                            for (k = 0; k < 4; k+=4) {
                                newboard[i + j - k] = newboard[i + j - k - 4];
                            }
                            newboard[i + j - k] = 0;
                        }
                    }
                }
            }
            if (newboard[i + j] != 0) {
                if (j == 12) {
                    if (newboard[i + j] == newboard[i + j - 4]) {
                        newboard[i + j] += newboard[i + j - 4];
                        newboard[i + j - 4] = newboard[i + j - 8];
                        newboard[i + j - 8] = newboard[i + j - 12];
                        newboard[i + j - 12] = 0;
                    }
                    if (newboard[i + j - 4] == 0 && newboard[i + j] == newboard[i + j - 8]) {
                        newboard[i + j] += newboard[i + j - 8];
                        newboard[i + j - 4] = newboard[i + j - 12];
                        newboard[i + j - 8] = 0;
                        newboard[i + j - 12] = 0;
                    }
                    if (newboard[i + j - 4] == 0 && newboard[i + j - 8] == 0 && newboard[i + j] == newboard[i + j - 12]) {
                        newboard[i + j] += newboard[i + j - 12];
                        newboard[i + j - 4] = 0;
                        newboard[i + j - 8] = 0;
                        newboard[i + j - 12] = 0;
                    }
                }
                if (j == 8) {
                    if (newboard[i + j] == newboard[i + j - 4]) {
                        newboard[i + j] += newboard[i + j - 4];
                        newboard[i + j - 4] = newboard[i + j - 8];
                        newboard[i + j - 8] = 0;
                    }
                    if (newboard[i + j - 4] == 0 && newboard[i + j] == newboard[i + j - 8]) {
                        newboard[i + j] += newboard[i + j - 8];
                        newboard[i + j - 4] = 0;
                        newboard[i + j - 8] = 0;
                    }
                }
                if (j == 4) {
                    if (newboard[i + j] == newboard[i + j - 4]) {
                        newboard[i + j] += newboard[i + j - 4];
                        newboard[i + j - 4] = 0;
                    }
                }
            }
        }
    }
    randomWait = ((rand() % 5) + 1);
    WriteToStdout();
    kill(pid, SIGUSR1);
}

/**********************************************************
 * function name: WinFunc.
 * the input: none.
 * the output: none.
 * the function operation: the function checks for win.
 *********************************************************/
int WinFunc(){
    int k;
    int writeRes;
    for (k = 0; k < 16; k++){
        if (newboard[k] == 2048){
            sleep(1);
            writeRes = write(1, "Congratulations!", 16);
            if (writeRes < 0){
                perror("write faild");
                exit(-1);
            }
            return 1;
        }
    }
    return 0;
}

/**********************************************************
 * function name: LoseFunc.
 * the input: none.
 * the output: none.
 * the function operation: the function checks for lose.
 *********************************************************/
int LoseFunc(){
    int k, flag = 0;
    int writeRes;
    for (k = 0; k < 16; k++){
        if (newboard[k] == 0){
            flag = 1;
        }
    }
    if (flag == 0){
        writeRes = write(1, "Game Over!", 10);
        if (writeRes < 0){
            perror("write faild");
            exit(-1);
        }
        return 1;
    }
    else{
        return 0;
    }
}

/**********************************************************
 * function name: SigintHandler.
 * the input: int sig.
 * the output: none.
 * the function operation: the function handels the `SIGINT` aignal
 * by setting the global variable `flag` to zero.
 *********************************************************/
void SigintHandler(int sig){
    flag = 0;
}
