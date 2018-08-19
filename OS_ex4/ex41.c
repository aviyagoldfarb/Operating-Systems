/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex41
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<errno.h>

//will use us for the semaphores
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

//1k shared memory segment
#define SHM_SIZE 1024

int main() {
    //will get the return value of ftok function
    key_t key;
    key_t semKey;
    //shared memory id
    int shmid;
    //will hold the pointer to the shared memory
    char* shmPtr;
    //will get the user`s input for operation code
    char operation;
    //will get the return value of semget
    int semid;
    //variables that will use us for the semaphores
    union semun arg;
    struct sembuf sops;
    //creating a unique key for the shm
    if((key = ftok("201509635.txt", 'k')) == -1){
        perror("ftok failed");
        exit(-1);
    }
    //allocating memory for the shm
    if((shmid = shmget(key, SHM_SIZE, 0)) == -1){
        perror("shmget failed");
        exit(-1);
    }
    //attach the allocated shm to the process
    shmPtr = (char*)shmat(shmid, 0, 0777);
    if(shmPtr == (char*)(-1)){
        perror("shmat failed");
        exit(-1);
    }
    //creating a unique key for the semaphore
    if((semKey = ftok("201509635.txt", 'r')) == -1){
        perror("ftok failed");
        exit(-1);
    }
    //getting the id of the semaphore that created by the server
    semid = semget(semKey, 2, 0);
    if (semid == -1){
        perror("semget failed");
        exit(-1);
    }
    //scanning from the user his wanted operation
    do {
        printf("Please enter request code\n");
        scanf(" %c", &operation);
        //if the operation is 'i', end the program
        if (operation == 'i' || operation == 'I') {
            break;
        }
        //setting the semaphore- locking the write to the shm
        sops.sem_num = 1;
        sops.sem_flg = 0;
        sops.sem_op = -1;
        //operating the write locking
        if(semop(semid ,&sops ,1) == -1){
            if(errno == EINVAL || errno == EIDRM){
                //detach the shm
                if(shmdt(shmPtr) == -1){
                    perror("shmdt failed");
                    exit(-1);
                }
                exit(0);
            }
            perror("semop failed");
            exit(-1);
        }
        //writing to the shm
        *shmPtr = operation;
        //setting the semaphore- openning the read from the shm
        sops.sem_num = 0;
        sops.sem_flg = 0;
        sops.sem_op = 1;
        //operating the read opening
        if(semop(semid ,&sops ,1) == -1){
            if(errno == EINVAL || errno == EIDRM){
                //detach the shm
                if(shmdt(shmPtr) == -1){
                    perror("shmdt failed");
                    exit(-1);
                }
                exit(0);
            }
            perror("semop failed");
            exit(-1);
        }
    }while(operation != 'i' && operation != 'I');
    //detach the shm
    if(shmdt(shmPtr) == -1){
        perror("shmdt failed");
        exit(-1);
    }
    return 0;
}