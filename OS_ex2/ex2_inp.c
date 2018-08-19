/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex2_inp
 *********************************************************/
#include <stdio.h>
#include <sys/fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define SIZE 100

void DataProcessor(int sig);
void PrintBoard(char* buffer);
void ClearScreen();
void SigintHandler(int sig);
//global variable
int flag = 1;

int main(){
    //signal setters
    struct sigaction userAction;
    sigset_t blockMask;

    sigfillset(&blockMask);
    sigdelset(&blockMask, SIGINT);
    userAction.sa_handler = DataProcessor;
    userAction.sa_mask = blockMask;
    userAction.sa_flags = 0;
    sigaction(SIGUSR1, &userAction, NULL);

    userAction.sa_handler = SigintHandler;
    sigfillset(&blockMask);
    userAction.sa_mask = blockMask;
    sigaction(SIGINT, &userAction, NULL);
    //in order to keep the program running until SIGINT signal
    do{
        pause();
    }while(flag);

    return 0;
}

/**********************************************************
 * function name: DataProcessor.
 * the input: int sig.
 * the output: none.
 * the function operation: the function handels the `SIGUSR1` aignal
 * by reading data from stdin and by using `PrintBoard` function in order to
 * print the data to the screen.
 *********************************************************/
void DataProcessor(int sig){
    //will use us in order to read from stdin
    char buffer[SIZE];
    memset(buffer, '\0', SIZE);
    int charsRead;
    int writeRes;
    //reading from stdin
    charsRead = read(0, buffer, SIZE);
    //in case of failure
    if (charsRead < 0){
        perror("read faild");
        exit(-1);
    }
    //checking for win
    if(strcmp(buffer, "Congratulations!") == 0){
        writeRes = write(1, "Congratulations!\n", 17);
        if (writeRes < 0){
            perror("write faild");
            exit(-1);
        }
        flag = 0;
        return;
    }
    //checking for lose
    if(strcmp(buffer, "Game Over!") == 0){
        writeRes = write(1, "Game Over!\n", 11);
        if (writeRes < 0){
            perror("write faild");
            exit(-1);
        }
        flag = 0;
        return;
    }
    //printing to the screen
    PrintBoard(buffer);
}

/**********************************************************
 * function name: PrintBoard.
 * the input: char* buffer.
 * the output: none.
 * the function operation: the function prints the passed buffer
 * to the screen in the wanted matrix form.
 *********************************************************/
void PrintBoard(char* buffer){
    //will use us for strtok function
    char* token;
    //in order to know which number is handled now
    int counter = 0;
    int writeRes;
    /*will use us in order to build the correct form of the matrix
    before printing it to the screen*/
    char printedBoard[120];
    memset(printedBoard, '\0', 120);
    int length;
    //breaking the buffer according to ","
    token = strtok(buffer, ",");
    counter++;
    //keep loopint until the end of the buffer
    while(token != NULL){
        strcat(printedBoard, "| ");
        if (strcmp(token, "0") == 0){
            strcat(printedBoard, "    ");
        }
        else{
            length = strlen(token);
            if (length == 1){
                strcat(printedBoard, "000");
                strcat(printedBoard, token);
            }
            if (length == 2){
                strcat(printedBoard, "00");
                strcat(printedBoard, token);
            }
            if (length == 3){
                strcat(printedBoard, "0");
                strcat(printedBoard, token);
            }
            if (length == 4){
                strcat(printedBoard, token);
            }
        }
        strcat(printedBoard, " ");
        if ((counter % 4) == 0){
            strcat(printedBoard, "|\n");
        }
        token = strtok(NULL, ",'\n'");
        counter++;
    }
    ClearScreen();
    writeRes = write(1, printedBoard, strlen(printedBoard));
    if (writeRes < 0){
        perror("write faild");
        exit(-1);
    }
}

/**********************************************************
 * function name: ClearScreen.
 * the input: none.
 * the output: none.
 * the function operation: credit to ohad cohen for sharing this
 * function, which clears from the screen the previous printings.
 *********************************************************/
void ClearScreen(){
    static int called = 0;
    char buffer[] = "\033[4A";
    if (!called){
        called = 1;
        return;
    }
    if(-1==write(STDOUT_FILENO, buffer, strlen(buffer))){
        perror("faild to clear the screen");
        exit(EXIT_FAILURE);
    }
}

/**********************************************************
 * function name: SigintHandler.
 * the input: int sig.
 * the output: none.
 * the function operation: the function handels the `SIGINT` aignal
 * by changing the global variable `flag`, and by printing to the
 * screen a message.
 *********************************************************/
void SigintHandler(int sig){
    int writeRes;
    writeRes = write(1, "BYE BYE\n", 8);
    if (writeRes < 0){
        perror("write faild");
        exit(-1);
    }
    flag = 0;
}
