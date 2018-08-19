/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex11
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define SIZE 1

int CheckFilesMatch(int fdFirst, int fdSecond);
int CmpFunc(char ch1, char ch2);

int main(int argc, char* argv[]){
    //these variables will get the return value from `open` function
    int fdFirst;
    int fdSecond;
    //will use us for `close` function
    int closeRes1;
    int closeRes2;
    //this variable will get the return value from `CheckFilesMatch` function
    int cmpresult;
    //open the first file
    fdFirst = open(argv[1], O_RDONLY);
    //in case of failure
    if (fdFirst < 0){
        perror("open faild");
        exit(-1);
    }
    //open the second file
    fdSecond = open(argv[2], O_RDONLY);
    //in case of failure
    if (fdSecond < 0){
        perror("open faild");
        exit(-1);
    }
    //comparing the two files using `CheckFilesMatch` function
    cmpresult = CheckFilesMatch(fdFirst, fdSecond);
    closeRes1 = close(fdFirst);
    if (closeRes1  < 0){
        perror("close faild");
        exit(-1);
    }
    closeRes2 = close(fdSecond);
    if (closeRes2  < 0){
        perror("close faild");
        exit(-1);
    }
    return cmpresult;
}//end of main

/**********************************************************
 * function name: CheckFilesMatch
 * the input: int fdFirst, int fdSecond (these are the return values from
 * the `open` function in main).
 * the output: returns 1 in case of match, 2 in case of
 * `similarity`, and 3 in case of differnt files.
 * the function operation: the function reads one char by one char
 * from the two files, compares them using `CmpFunc` function, and
 * according to its return value determines whether to read another char
 * from both files, or just from one file, or to end the reading from the
 * files.
 *********************************************************/
int CheckFilesMatch(int fdFirst, int fdSecond){
    //these variables uses us in order to read char by char from the two files
    char buffer1[SIZE];
    char buffer2[SIZE];
    //these variables will get the return values from the `read` function
    int charsRead1;
    int charsRead2;
    /*these variables will use us as flags in order to decide whether to read
      another char from each file or not*/
    int read1Flag = 1;
    int read2Flag = 1;
    //this variable will hold the final return value of the function
    int retval = 1;
    //loopping over the two files
    do {
        //reading from the first file
        if (read1Flag) {
            charsRead1 = read(fdFirst, buffer1, SIZE);
            //in case of failure
            if (charsRead1 < 0){
                perror("read faild");
                exit(-1);
            }
        }
        //reading from the second file
        if (read2Flag) {
            charsRead2 = read(fdSecond, buffer2, SIZE);
            //in case of failure
            if (charsRead2 < 0){
                perror("read faild");
                exit(-1);
            }
        }
        //in case we ended the reading from the first file before the second
        if (charsRead1 == 0 && charsRead2 != 0){
            //loopping over the rest of the second file
            do{
                //reading from the second file
                charsRead2 = read(fdSecond, buffer2, SIZE);
                //in case of failure
                if (charsRead2 < 0){
                    perror("read faild");
                    exit(-1);
                }
                //in case we read a char which is not a space
                if (!(isspace(buffer2[0]))){
                    retval = 3;
                    break;
                }
            }while(charsRead2 != 0);
            break;
        }
        //in case we ended the reading from the second file before the first
        if (charsRead2 == 0 && charsRead1 != 0){
            //loopping over the rest of the first file
            do{
                //reading from the first file
                charsRead1 = read(fdFirst, buffer1, SIZE);
                //in case of failure
                if (charsRead1 < 0){
                    perror("read faild");
                    exit(-1);
                }
                //in case we read a char which is not a space
                if (!(isspace(buffer1[0]))){
                    retval = 3;
                    break;
                }
            }while(charsRead1 != 0);
            break;
        }
        //in case of match between the two read chars
        if (CmpFunc(buffer1[0],buffer2[0]) == 0){
            read1Flag = 1;
            read2Flag = 1;
            continue;
        }
        //in case of `similarity` between the two read chars
        if (CmpFunc(buffer1[0],buffer2[0]) == 1){
            read1Flag = 1;
            read2Flag = 1;
            retval = 2;
            continue;
        }
        //in case of difference between the two read chars
        else{
            //in case that the first read char is space and the second is not
            if (isspace(buffer1[0]) && !(isspace(buffer2[0]))){
                //we would like to read another char only from the first file
                read1Flag = 1;
                read2Flag = 0;
                retval = 2;
                continue;
            }
            //in case that the second read char is space and the first is not
            if  (isspace(buffer2[0]) && !(isspace(buffer1[0]))){
                //we would like to read another char only from the second file
                read1Flag = 0;
                read2Flag = 1;
                retval = 2;
                continue;
            }
            //means that we found difference between the read chars
            else{
                retval = 3;
                break;
            }
        }
    }while(!(charsRead1 == 0 && charsRead2 == 0));
    return retval;
}//end of CheckFilesMatch

/**********************************************************
 * function name: CmpFunc
 * the input: char ch1, char ch2
 * the output: returns 0 in case of match, 1 in case of `similarity` and -1
 * in case of difference.
 * the function operation: the function compares between the two passed chars.
 * in case of identity it returns 0, in case of `similarity` (case insensitive
 * or white spaces) it returns 1, else it returns -1.
 *********************************************************/
int CmpFunc(char ch1, char ch2){
    if (ch1 == ch2){
        return 0;
    }
    if (isalpha(ch1) && isalpha(ch2) && abs(ch1 - ch2) == 32){
        return 1;
    }
    if (isspace(ch1) && isspace(ch2)){
        return 1;
    }
    return -1;
}//end of CmpFunc
