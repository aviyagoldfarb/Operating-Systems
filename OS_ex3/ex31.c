/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex31
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>

//1k shared memory segment
#define SHM_SIZE 1024

int main() {
    //will get the return value of ftok function
    key_t key;
    //shared memory id
    int shmid;
    //file descriptor of the fifo
    int fdFifo;
    //will hold the pid`s of the two clients
    int pid1, pid2;
    //will use us in order to check the return value of functions
    int readRes;
    int closeRes;
    //will hold the pointer to the shared memory
    char* shmPtr;
    //index
    int i;
    //creating a unique key for the shm
    if((key = ftok("ex31.c", 'k')) == -1){
        perror("ftok failed");
        exit(-1);
    }
    //allocating memory for the shm
    if((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0777)) == -1){
        perror("shmget failed");
        exit(-1);
    }
    //attach the allocated shm to the process
    shmPtr = (char*)shmat(shmid, 0, 0777);
    if(shmPtr == (char*)(-1)){
        perror("shmat failed");
        exit(-1);
    }
    //putting '\0' in the first four cells of the shm
    for(i = 0; i < 4; i++){
        *(shmPtr + i) = '\0';
    }
    //creating fifo named as fifo_clientTOserver
    if(mkfifo("fifo_clientTOserver", 0777) < 0){
        perror("unable to creat a fifo");
        exit(-1);
    }
    //using the fifo for the first client process
    fdFifo = open("fifo_clientTOserver", O_RDONLY);
    if (fdFifo < 0){
        perror("open failed");
        exit(-1);
    }
    readRes = read(fdFifo, &pid1, sizeof(int));
    if (readRes < 0){
        perror("read failed");
        exit(-1);
    }
    closeRes = close(fdFifo);
    if (closeRes  < 0){
        perror("close failed");
        exit(-1);
    }
    //in order to prevent some problems
    sleep(1);
    /*the execution will wait here until the second client will open the fifo
     for writing*/
    //using the fifo for the second client process
    fdFifo = open("fifo_clientTOserver", O_RDONLY| O_TRUNC);
    //in case of failure
    if (fdFifo < 0){
        perror("open failed");
        exit(-1);
    }
    readRes = read(fdFifo, &pid2, sizeof(int));
    if (readRes < 0){
        perror("read failed");
        exit(-1);
    }
    closeRes = close(fdFifo);
    if (closeRes  < 0){
        perror("close failed");
        exit(-1);
    }
    //delete the fifo
    unlink("fifo_clientTOserver");
    //sending SIGUSR1 to the first client process
    kill(pid1, SIGUSR1);
    //checking the shm until the first client process writes to it
    while(*shmPtr != 'b'){
        sleep(1);
    }
    //sending SIGUSR1 to the second client process
    kill(pid2, SIGUSR1);
    //checking the shm until the last client process declares end of game
    while(*shmPtr != 'g'){
        sleep(1);
    }
    printf("GAME OVER\n");
    //printing the right messages according to the shm
    if(*(shmPtr + 1) == 'b'){
        printf("Winning player: Black\n");
    }
    if(*(shmPtr + 1) == 'w'){
        printf("Winning player: White\n");
    }
    if(*(shmPtr + 1) == 'e'){
        printf("No winning player\n");
    }
    //detach the shm
    if(shmdt(shmPtr) == -1){
        perror("shmdt failed");
        exit(-1);
    }
    //delete the shm
    if(shmctl(shmid, IPC_RMID, NULL) == -1){
        perror("shmctl failed");
        exit(-1);
    }
    return 0;
}