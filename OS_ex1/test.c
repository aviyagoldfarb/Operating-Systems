//
// Created by aviya on 10/05/17.
//

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    pid_t pid;
    int stat;
    int fdin;
    int fdout;
    fdout = open("resFile.txt", O_CREAT | O_RDONLY, S_IRWXU|S_IRWXG|S_IRWXO);
        execlp("gcc", "gcc","/home/aviya/ClionProjects/co mission 2/ex2.c",NULL);

    printf("%d",fdout);
    return 0;
}