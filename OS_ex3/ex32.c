/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex32
 *********************************************************/
#include<signal.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define SHM_SIZE 1024

void ShmAttach(int sig);
void CheckPlayerNumber(int* player, int* enemy);
void InitializeBoard(int player, int enemy);
int IsThreatingSquare(int player, int enemy, int x, int y, int flag);
void PrintBoard();
int CheckForEndOFGame(int enemy, int player);
int CheckForWinner(int player, int enemy);
void WriteToShm(int player, int x, int y);
//will hold the pointer to the shm
char* globalShmPtr;
//will hold the board of the game
int gameBoard[8][8];

int main(){
    //file descriptor of the fifo
    int fdFifo;
    //will hold the pid of the client
    int pid;
    //will use us in order to check the return value of functions
    int writeRes;
    int closeRes;
    //will use us as condition for the main while loop
    int flag = 1;
    //will use us in order to scan the indexes of the board
    int x, y;
    //will hold the number of the player- 2 for black, 1 for white
    int player;
    //will hold the number of the enemy- 2 for black, 1 for white
    int enemy;
    //will get the return value of CheckForWinner function
    int checkForWinnerRes;
    //will use us for scanf function
    int scanfFlag = 0;
    //indexes
    int i, j;
    //in order to check potential move
    int potentialMoveFlag = 0;
    //signal setters
    struct sigaction userAction;
    sigset_t blockMask;

    sigfillset(&blockMask);
    userAction.sa_handler = ShmAttach;
    userAction.sa_mask = blockMask;
    userAction.sa_flags = 0;
    sigaction(SIGUSR1, &userAction, NULL);
    //checking for the process pid
    pid = getpid();
    /*open the fifo in order to write the pid to it, so that the server
     process will be able to read it*/
    fdFifo = open("fifo_clientTOserver", O_WRONLY);
    if (fdFifo < 0){
        perror("open failed");
        exit(-1);
    }
    writeRes = write(fdFifo, &pid, sizeof(int));
    if (writeRes < 0){
        perror("write failed");
        exit(-1);
    }
    closeRes = close(fdFifo);
    if (closeRes  < 0){
        perror("close failed");
        exit(-1);
    }
    //waiting for the server process to send a signal SIGUSR1
    pause();
    CheckPlayerNumber(&player, &enemy);
    //initialize the board according to the player`s number
    InitializeBoard(player, enemy);
    //scanning the indexes of the board
    printf("Please choose a square\n");
    scanf("[%d,%d]", &x, &y);
    //running until the end of the game
    while(flag){

        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++){
                if(gameBoard[i][j] == 0){
                    if(IsThreatingSquare(player, enemy, j, i, 0) == 1){
                        potentialMoveFlag = 1;
                    }
                }
            }
        }
        if(potentialMoveFlag == 0){
            //if there are no legal moves, check for the winner
            checkForWinnerRes = CheckForWinner(player, enemy);
            if(checkForWinnerRes == 2){
                printf("Winning player: Black\n");
            }
            if(checkForWinnerRes == 1){
                printf("Winning player: White\n");
            }
            if(checkForWinnerRes == 0){
                printf("No winning player\n");
            }
            flag = 0;
            /*writing to the shm in order to let the server know about the
             end of the game and the results*/
            *globalShmPtr = 'g';
            if(checkForWinnerRes == 2){
                *(globalShmPtr + 1) = 'b';
            }
            if(checkForWinnerRes == 1){
                *(globalShmPtr + 1) = 'w';
            }
            if(checkForWinnerRes == 0){
                *(globalShmPtr + 1) = 'e';
            }
            *(globalShmPtr + 2) = '\0';
            *(globalShmPtr + 3) = '\0';
            break;
        }

        //scanning the indexes of the board (from the second iteration and on)
        if(scanfFlag){
            printf("Please choose a square\n");
            scanf(" [%d,%d]", &x, &y);
        }
        scanfFlag = 1;
        //checking the validation of the indexes
        if((x > 7 || x < 0) || (y > 7 || y < 0)){
            printf("No such square\nPlease choose another square\n");
            continue;
        }
        if(IsThreatingSquare(player, enemy, x, y, 1) == 0){
            printf("This square is invalid\nPlease choose another square\n");
            continue;
        }
        PrintBoard();
        //checking for the end of the game
        if(CheckForEndOFGame(enemy, player)){
            //if the game ended, check for the winner
            checkForWinnerRes = CheckForWinner(player, enemy);
            if(checkForWinnerRes == 2){
                printf("Winning player: Black\n");
            }
            if(checkForWinnerRes == 1){
                printf("Winning player: White\n");
            }
            if(checkForWinnerRes == 0){
                printf("No winning player\n");
            }
            flag = 0;
            //write the move to the shm
            WriteToShm(player, x, y);
            break;
        }
        printf("Waiting for the other player to make a move\n");
        //write the move to the shm
        WriteToShm(player, x, y);
        //split the waiting phase according to the player`s number
        if(player == 2){
            //as long as the shm contains the move of the black player
            while(*globalShmPtr == 'b'){
                printf("Waiting for the other player to make a move\n");
                sleep(1);
            }
        }
        if(player == 1){
            //as long as the shm contains the move of the white player
            while(*globalShmPtr == 'w'){
                printf("Waiting for the other player to make a move\n");
                sleep(1);
            }
        }
        //refreshing the board after the enemy`s move
        x = *(globalShmPtr + 1) - '0';
        y = *(globalShmPtr + 2) - '0';
        IsThreatingSquare(enemy, player, x, y, 1);
        PrintBoard();
        //checking for the end of the game
        if(CheckForEndOFGame(enemy, player)){
            //if the game ended, check for the winner
            checkForWinnerRes = CheckForWinner(player, enemy);
            if(checkForWinnerRes == 2){
                printf("Winning player: Black\n");
            }
            if(checkForWinnerRes == 1){
                printf("Winning player: White\n");
            }
            if(checkForWinnerRes == 0){
                printf("No winning player\n");
            }
            flag = 0;
            /*writing to the shm in order to let the server know about the
             end of the game and the results*/
            *globalShmPtr = 'g';
            if(checkForWinnerRes == 2){
                *(globalShmPtr + 1) = 'b';
            }
            if(checkForWinnerRes == 1){
                *(globalShmPtr + 1) = 'w';
            }
            if(checkForWinnerRes == 0){
                *(globalShmPtr + 1) = 'e';
            }
            *(globalShmPtr + 2) = '\0';
            *(globalShmPtr + 3) = '\0';
            break;
        }
    }
    //detach the shm
    if(shmdt(globalShmPtr) == -1){
        perror("shmdt failed");
        exit(-1);
    }
    return 0;
}

/**********************************************************
 * function name: ShmAttach.
 * the input: int sig.
 * the output: none.
 * the function operation: the function handels the `SIGUSR1` signal
 * by attaching the shm to the process.
 *********************************************************/
void ShmAttach(int sig){
    //will get the return value of ftok function
    key_t key;
    //shared memory id
    int shmid;
    //will hold the pointer to the shared memory
    char* shmPtr;
    //creating a unique key for the shm
    if((key = ftok("ex31.c", 'k')) == -1){
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
    //assigning the pointer to the shared memory into the global pointer variable
    globalShmPtr = shmPtr;

}

/**********************************************************
 * function name: CheckPlayerNumber.
 * the input: int* player, int* enemy.
 * the output: none.
 * the function operation: the function determines the player`s number according
 * to the first cell of the shm.
 *********************************************************/
void CheckPlayerNumber(int* player, int* enemy){
    //this is the first turn of the game- the player is the black
    if(*globalShmPtr == '\0'){
        *player = 2;
        *enemy = 1;
    }
    else{
        //determine the player`s number according to the first cell of the shm
        if(*globalShmPtr == 'b'){
            *player = 1;
            *enemy = 2;
        }
        else{
            if(*globalShmPtr == 'w'){
                *player = 2;
                *enemy = 1;
            }
        }
    }
}

/**********************************************************
 * function name: InitializeBoard.
 * the input: int player, int enemy.
 * the output: none.
 * the function operation: the function initializes the board.
 *********************************************************/
void InitializeBoard(int player, int enemy){
    int i, j;
    int x, y;
    for(i = 0; i < 8; i++){
        for(j = 0; j < 8; j++){
            gameBoard[i][j] = 0;
        }
    }
    gameBoard[3][3] = 2;
    gameBoard[4][4] = 2;
    gameBoard[4][3] = 1;
    gameBoard[3][4] = 1;
    /*if the player is the white we must refresh the board according to the
     first move of the black player*/
    if(player == 1){
        x = *(globalShmPtr + 1) - '0';
        y = *(globalShmPtr + 2) - '0';
        IsThreatingSquare(enemy, player, x, y, 1);
    }
}

/**********************************************************
 * function name: IsThreatingSquare.
 * the input: int player, int enemy, int x, int y, int flag.
 * the output: 0 in case of no threat, 1 in case there is a threat and the
 * player made a mvoe.
 * the function operation: the function checks according to the game`s rules
 * if the chosen square of the player can threat other squares of the enemy.
 * if not, it returns 0. if it dose, the function makes the player`s move and
 * refreshes the board respectively.
 *********************************************************/
int IsThreatingSquare(int player, int enemy, int x, int y, int flag){
    int i, j;
    //the chosen square is not available
    if(gameBoard[y][x] != 0){
        return 0;
    }
    else{
        //up
        if((y - 1) >= 0 && gameBoard[y - 1][x] == enemy){
            i = 2;
            while((y - i) >= 0  && gameBoard[y - i][x] == enemy){
                i++;
            }
            if(y - i >= 0){
                if(gameBoard[y - i][x] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y - j][x] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of up
        //down
        if((y + 1) <= 7 && gameBoard[y + 1][x] == enemy){
            i = 2;
            while((y + i) <= 7 && gameBoard[y + i][x] == enemy){
                i++;
            }
            if(y + i <= 7){
                if(gameBoard[y + i][x] == player) {
                    if (flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y + j][x] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of down
        //right
        if((x + 1) <= 7 && gameBoard[y][x + 1] == enemy){
            i = 2;
            while((x + i) <= 7 && gameBoard[y][x + i] == enemy){
                i++;
            }
            if(x + i <= 7){
                if(gameBoard[y][x + i] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y][x + j] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of right
        //left
        if((x - 1) >= 0 && gameBoard[y][x - 1] == enemy){
            i = 2;
            while((x - i) >= 0 && gameBoard[y][x - i] == enemy){
                i++;
            }
            if(x - i >= 0){
                if(gameBoard[y][x - i] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y][x - j] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of left
        //up & right
        if((y - 1) >= 0 && (x + 1) <= 7 && gameBoard[y - 1][x + 1] == enemy){
            i = 2;
            while((y - i) >= 0 && (x + i) <= 7 && gameBoard[y - i][x + i] == enemy){
                i++;
            }
            if((y - i) >= 0 && (x + i) <= 7){
                if(gameBoard[y - i][x + i] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y - j][x + j] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of up & right
        //up & left
        if((y - 1) >= 0 && (x - 1) >= 0 && gameBoard[y - 1][x - 1] == enemy){
            i = 2;
            while((y - i) >= 0 && (x - i) >= 0 && gameBoard[y - i][x - i] == enemy){
                i++;
            }
            if((y - i) >= 0 && (x - i) >= 0){
                if(gameBoard[y - i][x - i] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y - j][x - j] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of up & left
        //down & right
        if((y + 1) <= 7 && (x + 1) <= 7 && gameBoard[y + 1][x + 1] == enemy){
            i = 2;
            while((y + i) <= 7 && (x + i) <= 7 && gameBoard[y + i][x + i] == enemy){
                i++;
            }
            if((y + i) <= 7 && (x + i) <= 7){
                if(gameBoard[y + i][x + i] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y + j][x + j] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of down & right
        //down & left
        if((y + 1) <= 7 && (x - 1) >= 0 && gameBoard[y + 1][x - 1] == enemy){
            i = 2;
            while((y + i) <= 7 && (x - i) >= 0 && gameBoard[y + i][x - i] == enemy){
                i++;
            }
            if((y + i) <= 7 && (x - i) >= 0){
                if(gameBoard[y + i][x - i] == player){
                    if(flag) {
                        for (j = 0; j <= i; j++) {
                            gameBoard[y + j][x - j] = player;
                            flag += 1;
                        }
                    }
                    else{
                        return 1;
                    }
                }
            }
        }//end of down & left
        if(flag == 0 || flag == 1){
            return 0;
        }
        if(flag > 1){
            return 1;
        }
    }
}

/**********************************************************
 * function name: PrintBoard.
 * the input: none.
 * the output: none.
 * the function operation: the function prints the board.
 *********************************************************/
void PrintBoard(){
    int i, j;
    printf("The board is:\n");
    for(i = 0; i < 8; i++){
        for(j = 0; j < 8; j++){
            printf("%d ", gameBoard[i][j]);
        }
        printf("\n");
    }
}

/**********************************************************
 * function name: CheckForEndOFGame.
 * the input: int enemy.
 * the output: 1 for end of game, 0 for continue.
 * the function operation: the function checks if the game ended by two ways:
 * one- in case that all cells are fill, two- if the enemy has no cells on the
 * board (so he cant do any move).
 *********************************************************/
int CheckForEndOFGame(int enemy, int player){
    int i, j;
    int zeroFlag = 0;
    int enemyFlag = 0;
    int playerFlag = 0;
    for(i = 0; i < 8; i++){
        for(j = 0; j < 8; j++){
            if(gameBoard[i][j] == 0){
                zeroFlag = 1;
            }
            if(gameBoard[i][j] == enemy){
                enemyFlag = 1;
            }
            if(gameBoard[i][j] == player){
                playerFlag = 1;
            }
        }
    }
    if(zeroFlag == 0 || enemyFlag == 0){
        return 1;
    }
    if(playerFlag == 0){
        return 1;
    }
    return 0;
}

/**********************************************************
 * function name: CheckForWinner.
 * the input: int player, int enemy.
 * the output: returns the number of the winning player.
 * the function operation: the function counts the cells of each player on
 * the board. the player with the bigger number of cells is the winner.
 *********************************************************/
int CheckForWinner(int player, int enemy){
    int i, j;
    int playerCounter = 0;
    int enemyCounter = 0;
    for(i = 0; i < 8; i++){
        for(j = 0; j < 8; j++){
            if(gameBoard[i][j] == player){
                playerCounter++;
            }
            if(gameBoard[i][j] == enemy){
                enemyCounter++;
            }
        }
    }
    //check who has the bigger number
    if(playerCounter > enemyCounter){
        return player;
    }
    if(playerCounter < enemyCounter){
        return enemy;
    }
    if(playerCounter = enemyCounter){
        return 0;
    }
}

/**********************************************************
 * function name: WriteToShm.
 * the input: int player, int x, int y.
 * the output: none.
 * the function operation: the function writes the player`s move to the
 * shm, so that the other client process will be able to update its
 * board according to its enemy`s move.
 *********************************************************/
void WriteToShm(int player, int x, int y){
    if(player == 1){
        *globalShmPtr = 'w';
        *(globalShmPtr + 1) = x + '0';
        *(globalShmPtr + 2) = y + '0';
        *(globalShmPtr + 3) = '\0';
    }
    if(player == 2){
        *globalShmPtr = 'b';
        *(globalShmPtr + 1) = x + '0';
        *(globalShmPtr + 2) = y + '0';
        *(globalShmPtr + 3) = '\0';
    }
}
