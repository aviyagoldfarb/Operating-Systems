/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex42
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<pthread.h>
#include<sys/sem.h>
#include <string.h>
#include <unistd.h>

//will use us for the semaphores
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

//will use us for the queue
struct node {
    char data;
    struct node *link;
}*front, *rear;

//1k shared memory segment
#define SHM_SIZE 1024

//function declarations;
void* Operation(void* arg);
void Enqueue(char data);
char Dequeue();
char TopElement();

//global variable
int internal_count = 0;
pthread_mutex_t lock;
//will get the return value of open
int fileFd;
int globalFlag = 1;

int main(){
    //will get the return value of ftok function
    key_t key;
    key_t semKey;
    //shared memory id
    int shmid;
    //will hold the pointer to the shared memory
    char* shmPtr;
    //will hold the 5 created threads
    pthread_t threadsArray[5];
    //index
    int i;
    //return value of pthread_create
    int retVal;
    //in order to improve `rand` function
    time_t t;
    srand((unsigned) time(&t));
    //will use us for while loop
    int flag = 1;
    //will get the return value of semget
    int semid;
    //variables that will use us for the semaphore
    union semun arg;
    struct sembuf sops;
    //front and rear of the queue
    front = NULL;
    rear = NULL;
    //will get the return value of pthread_self
    pthread_t tid;
    //will use us for sprintf function
    char buffer[150];
    memset(buffer, '\0', 150);
    //will use us in order to check the return value of functions
    int closeRes;
    int writeRes;

    //creating the `results.csv` file
    fileFd = open("201509635.txt", O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
    //in case of failure
    if (fileFd < 0){
        perror("open faild");
        exit(-1);
    }
    //creating a unique key for the shm
    if((key = ftok("201509635.txt", 'k')) == -1){
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
    //creating a unique key for the semaphore
    if((semKey = ftok("201509635.txt", 'r')) == -1){
        perror("ftok failed");
        exit(-1);
    }
    //getting the id of the semaphore
    semid = semget(semKey, 2, IPC_CREAT | 0666);
    if (semid == -1){
        perror("semget failed");
        exit(-1);
    }
    //creating a mutex
    if (pthread_mutex_init(&lock, NULL) != 0){
        perror("mutex init failed");
        exit(-1);
    }
    //allocating memory for the array field of the union
    arg.array = (ushort*)malloc(2 * sizeof(ushort));
    //setting the array according to the semaphore`s values
    arg.array[0] = 0; //read value
    arg.array[1] = 1; //write value
    //setting the semaphores
    semctl(semid, 0, SETALL, arg);
    //creating thread pool
    for(i = 0; i < 5; i++){
        retVal =  pthread_create(&threadsArray[i], NULL, Operation, NULL);
        if(retVal != 0){
            printf("Failed in creating thread, error %d\n", retVal);
        }
    }
    //loopping over in order to read from the shm the current operation
    do{
        //setting the semaphore- locking the read from the shm
        sops.sem_num = 0;
        sops.sem_flg = 0;
        sops.sem_op = -1;
        //operating the read locking
        if(semop(semid ,&sops ,1) == -1){
            perror("semop failed");
            exit(-1);
        }
        //if the user entered operation 'g'
        if(*shmPtr == 'g' || *shmPtr == 'G'){
            //loopping over the threads in order to cancel them
            for(i = 0; i < 5; i++){
                retVal = pthread_cancel(threadsArray[i]);
                if(retVal != 0){
                    printf("Failed in canceling thread, error %d\n", retVal);
                }
            }
            //writing to the file
            tid = pthread_self();
            sprintf(buffer, "thread identifier is %lu and internal count is %d\n", tid, internal_count);
            writeRes = write(fileFd, buffer, strlen(buffer));
            if (writeRes < 0){
                perror("write faild");
                exit(-1);
            }
            break;
        }
        //if the user entered operation 'h'
        if(*shmPtr == 'h' || *shmPtr == 'H'){
            globalFlag = 0;
            //loopping over the threads in order to wait for their end
            for(i = 0; i < 5; i++){
                retVal = pthread_join(threadsArray[i], NULL);
                if(retVal != 0){
                    printf("Failed in canceling thread, error %d\n", retVal);
                }
            }
            //writing to the file
            tid = pthread_self();
            sprintf(buffer, "thread identifier is %lu and internal count is %d\n", tid, internal_count);
            writeRes = write(fileFd, buffer, strlen(buffer));
            if (writeRes < 0){
                perror("write faild");
                exit(-1);
            }
            break;
        }
        //push the operation in the shm to the queue
        Enqueue(*shmPtr);
        //setting the semaphore- openning the write to the shm
        sops.sem_num = 1;
        sops.sem_flg = 0;
        sops.sem_op = 1;
        //operating the write opening
        if(semop(semid ,&sops ,1) == -1){
            perror("semop failed");
            exit(-1);
        }
    }while(flag);
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
    //delete the semaphore
    if(semctl(semid, 0, IPC_RMID,NULL) == -1){
        perror("semctl failed");
        exit(-1);
    }
    //free the allocated memory
    free(arg.array);
    //destroy the mutex
    if(pthread_mutex_destroy(&lock) != 0){
        perror("mutex destroy failed");
        exit(-1);
    }
    //close the created file "201509635.txt"
    closeRes = close(fileFd);
    if (closeRes  < 0){
        perror("close faild");
        exit(-1);
    }
    return 0;
}

/**********************************************************
 * function name: Operation.
 * the input: void* arg.
 * the output: void*.
 * the function operation: the function handels the current operation in the queue
 * according to the instructions. the function runs by the threads.
 *********************************************************/
void* Operation(void* arg){
    //will use us in nanosleep
    struct timespec req;
    //will hold the return value of rand
    int randomWait;
    //will hold the return value of pthread_self
    pthread_t tid;
    //will use us for sprintf function
    char buffer[150];
    memset(buffer, '\0', 150);
    //will get the return value of write
    int writeRes;
    //choosing the write operation from the queue
    do{
        //locking all the other threads here
        pthread_mutex_lock(&lock);
        //checking if the queue is empty
        if(TopElement() != '\0') {
            //if the first element in the queue is 'f'
            if(TopElement() == 'f' || TopElement() == 'F'){
                Dequeue();
                //writing to the file
                tid = pthread_self();
                sprintf(buffer, "thread identifier is %lu and internal count is %d\n", tid, internal_count);
                writeRes = write(fileFd, buffer, strlen(buffer));
                if (writeRes < 0){
                    perror("write faild");
                    exit(-1);
                }
                //unlock in order to let the next thread to run
                pthread_mutex_unlock(&lock);
                continue;
            }
            randomWait = ((rand() % 90) + 10);
            req.tv_sec = 0;
            req.tv_nsec = randomWait;
            nanosleep(&req, NULL);
            //switching over the top element of the queue and choosing the operation
            switch (Dequeue()) {
                case 'A':
                case 'a':
                    internal_count += 1;
                    break;
                case 'B':
                case 'b':
                    internal_count += 2;
                    break;
                case 'C':
                case 'c':
                    internal_count += 3;
                    break;
                case 'D':
                case 'd':
                    internal_count += 4;
                    break;
                case 'E':
                case 'e':
                    internal_count += 5;
                    break;
            }
        }
        //unlock in order to let the next thread to run
        pthread_mutex_unlock(&lock);
    }while(globalFlag);
    pthread_mutex_lock(&lock);
    //writing to the file
    tid = pthread_self();
    sprintf(buffer, "thread identifier is %lu and internal count is %d\n", tid, internal_count);
    writeRes = write(fileFd, buffer, strlen(buffer));
    if (writeRes < 0){
        perror("write faild");
        exit(-1);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

/**********************************************************
 * function name: Enqueue.
 * the input: char data.
 * the output: none.
 * the function operation: the function inserts elements into the queue.
 *********************************************************/
void Enqueue(char data) {
    struct node *temp;
    temp = (struct node*)malloc(sizeof(struct node));
    //insert the parameter `data`
    temp->data = data;
    temp->link = NULL;
    if (rear  ==  NULL) {
        front = rear = temp;
    }
    else{
        rear->link = temp;
        rear = temp;
    }
}

/**********************************************************
 * function name: Dequeue.
 * the input: none.
 * the output: char.
 * the function operation: the function deletes elements from the queue.
 *********************************************************/
char Dequeue(){
    struct node *temp;
    temp = front;
    char retval;
    if (front == NULL){
        //queue is empty
        front = rear = NULL;
        return '\0';
    }
    else{
        //deleted element is front->data
        retval = front->data;
        if(front == rear){
            front = rear = NULL;
        }
        else{
            front = front->link;
        }
        free(temp);
        return retval;
    }
}

/**********************************************************
 * function name: TopElement.
 * the input: none.
 * the output: char.
 * the function operation: the function returns the first element of queue.
 *********************************************************/
char TopElement(){
    if (front == NULL){
        return '\0';
    }
    else{
        return front->data;
    }
}



