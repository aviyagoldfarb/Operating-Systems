/**********************************************************
 * student name: aviya goldfarb
 * student id: 201509635
 * course exercize group: 06
 * exercize name: ex12
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
//#include <ctype.h>
#include <string.h>
#include <sys/fcntl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
//#include <malloc.h>

#define SIZE 160

char* pathBuilder(char* fatherPath, char* childName);
char* FindCFileFunc(char* fatherPath, int* pDepth, int* pSubDirsCounter, char* usersGradesBuffer);
int CompAndExecFunc(char* cFilePath, char* input, char* usersGradesBuffer, int* fdResFile);
int CompareFunc(char* correctOutputFile, int* fdResFile);
void GradeCalculator(int* depth, int compResult, char* usersGradesBuffer);
void ResultsFileCreator(int fd, char* usersGradesBuffer);

int main(int argc, char* argv[]){
    //this variable will get the return value from `open` function
    int fdconfig;
    //this variable will get the return values from the `read` function
    int charsRead;
    //this variable will hold the text of the configuration file
    char buffer[SIZE*3];
    memset(buffer, '\0', SIZE*3);
    //each line variable will hold the match line of the configuration file
    char line1[SIZE];
    char line2[SIZE];
    char line3[SIZE];
    memset(line1, '\0', SIZE);
    memset(line2, '\0', SIZE);
    memset(line3, '\0', SIZE);
    //will use us for the function `strtok`
    char* token;
    //will hold the path to the current student`s directory
    char* studentDirPath;
    //will get the return value of `opendir` function
    DIR* fatherDir;
    //will get the return value of `readdir` function
    struct dirent* pdirent;
    //will hold the depth of the current subdirectory
    int depth = 0;
    int* pDepth = &depth;
    //will hold the number of directories in the current level
    int subDirsCounter = 0;
    int* pSubDirsCounter = &subDirsCounter;
    //will hold the path of the current found c file
    char* cFilePath;
    //will get thr return value of `CompareFunc`
    int compResult;
    //will hold the return value of `open` function for the `results` file
    int resultsFd;
    //will hold the grade and notes of the current student
    char usersGradesBuffer[SIZE*3];
    //will hold the return value of `CompAndExecFunc`
    int compAndExecRes;
    //will use for success checking of close()
    int closeFather;
    //will hold the fd of `resFile.txt`
    int fdResFile;
    //will use for success checking of close()
    int closeRes;
    //open the configuration file
    fdconfig = open(argv[1], O_RDONLY);
    //in case of failure
    if (fdconfig < 0){
        perror("open faild");
        exit(-1);
    }
    //read from the configuration file to `buffer`
    charsRead = read(fdconfig, buffer, SIZE*3);
    //in case of failure
    if (charsRead < 0){
        perror("read faild");
        exit(-1);
    }
    //split the text of the configuration file by lines
    token = strtok(buffer, "\n");
    strcpy(line1, token);
    token = strtok(NULL, "\n");
    strcpy(line2, token);
    token = strtok(NULL, "\n");
    strcpy(line3, token);
    //the `fatherDir`s path is in first line
    fatherDir = opendir(line1);
    //in case of failure
    if (fatherDir == NULL){
        perror("open faild");
        exit(-1);
    }
    //creating the `results.csv` file
    resultsFd = open("results.csv", O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
    //in case of failure
    if (resultsFd < 0){
        perror("open faild");
        exit(-1);
    }
    //running over the directories- each directory represents a student
    do{
        //the current level is 0
        *pDepth = 0;
        //setting the `usersGradesBuffer` to '\0'
        memset(usersGradesBuffer, '\0', SIZE*3);
        //running over the directories- each directory represents a student
        pdirent = readdir(fatherDir);
        if (pdirent == NULL){
            break;
        }
        //taking care of '.' and '..' directories
        if (pdirent->d_name[0] == '.' && pdirent->d_name[1] == '.'){
            continue;
        }
        if (pdirent->d_name[0] == '.'){
            continue;
        }
        //copy the current student`s name to the `usersGradesBuffer`
        strcpy(usersGradesBuffer, pdirent->d_name);
        //calling `pathBuilder` function
        studentDirPath = pathBuilder(line1, pdirent->d_name);
        //searching for c file in the student`s directory using `FindCFileFunc`
        cFilePath = FindCFileFunc(studentDirPath, pDepth, pSubDirsCounter, usersGradesBuffer);
        //in case we found a c file
        if (cFilePath != NULL){
            //compile and execute the found c file using CompAndExecFunc
            compAndExecRes = CompAndExecFunc(cFilePath, line2, usersGradesBuffer, &fdResFile);
            //in case of failure in CompAndExecFunc
            if (compAndExecRes == -1){
                ResultsFileCreator(resultsFd, usersGradesBuffer);
                free(cFilePath);
                continue;
            }// end of `case of failure in CompAndExecFunc`
            /*in case that CompAndExecFunc worked, compare the result with the
             file in line3 path*/
            compResult = CompareFunc(line3, &fdResFile);
            //calculate the grade and notes according to the relevant data
            GradeCalculator(pDepth, compResult, usersGradesBuffer);
        }// end of `case we found a c file`
        //write the text in `usersGradesBuffer` into the file `results.csv`
        ResultsFileCreator(resultsFd, usersGradesBuffer);
        //free allocated memory
        free(cFilePath);
    //keep loopping as long as there are more directories (more students)
    } while(pdirent != NULL);
    closeRes = close(fdconfig);
    if (closeRes  < 0){
        perror("close faild");
        exit(-1);
    }
    //closing the father directory
    closeFather = closedir(fatherDir);
    //in case of failure
    if (closeFather < 0){
        perror("close faild");
        exit(-1);
    }
    unlink("resFile.txt");
    unlink("a.out");
    return 0;
}// end of main

/**********************************************************
 * function name: pathBuilder.
 * the input: char* fatherPath, char* childName.
 * the output: returns the child dir path.
 * the function operation: the function allocates memory for the childDirPath,
 * copy the fatherPath to the allocated memory, concats "/" and then concats
 * the childName.
 *********************************************************/
char* pathBuilder(char* fatherPath, char* childName){
    char* childDirPath = (char*)malloc(SIZE * sizeof(char));

    memset(childDirPath, '\0', SIZE);
    strcpy(childDirPath, fatherPath);
    strcat(childDirPath, "/");
    strcat(childDirPath, childName);
    return childDirPath;
}// end of pathBuilder

/**********************************************************
 * function name: FindCFileFunc.
 * the input: char* fatherPath, int* pDepth, int* pSubDirsCounter,
 * char* usersGradesBuffer.
 * the output: in case of finding c file- returns its path, else returns
 * NULL.
 * the function operation: the function runs over the subdirectories of the
 * current student, looking for ".c" postfix in the dir`s name. it allso
 * updates the variables `pDepth`, `pSubDirsCounter` and `usersGradesBuffer`
 * accordind to the instructions.
 *********************************************************/
char* FindCFileFunc(char* fatherPath, int* pDepth, int* pSubDirsCounter, char* usersGradesBuffer){
    //will hold the return value of `opendir` function
    DIR* childDir;
    //will get the return value of `readdir` function
    struct dirent* pdirent;
    //will get the return value of `strrchr` function
    char* searchRes;
    //will use us for `stat` function
    struct stat pStat;
    //will hold the return value of `pathBuilder`
    char* newPath;
    //will hold the path to the first directory in the current student`s subdir
    char* subChildDirPaths[1];
    //in order to check the success of `close` function
    int closeRes;
    //the current number of subdirectories is 0
    *pSubDirsCounter = 0;
    //open the current directory or subdirectory
    childDir = opendir(fatherPath);
    //in case of failure
    if (childDir == NULL){
        exit(-1);
    }
    do{
        //breadth first search for c file
        pdirent = readdir(childDir);
        //in case we ended reading from the current dir
        if (pdirent == NULL){
            break;
        }
        //searching for a c file
        searchRes = strrchr(pdirent->d_name, '.');
        //checking if we found a 'c' file
        if (searchRes != NULL && (strcmp(searchRes, ".c") == 0)){
            //build the path to the c file
            newPath = pathBuilder(fatherPath, pdirent->d_name);
            //closing the directory
            closeRes = closedir(childDir);
            //in case of failure
            if (closeRes < 0){
                perror("close faild");
                exit(-1);
            }
            free(fatherPath);
            return newPath;
        }
        //taking care of '.' and '..' directories
        if (pdirent->d_name[0] == '.' && pdirent->d_name[1] == '.'){
            continue;
        }
        if (pdirent->d_name[0] == '.') {
            continue;
        }
        else{
            //the current file is not a 'c' file
            newPath = pathBuilder(fatherPath, pdirent->d_name);
            if(stat(newPath, &pStat) == -1){
                perror("stat faild");
                exit(-1);
            }
            //checking if the current file is a directory
            if (S_ISDIR(pStat.st_mode)){
                //in case that this is the first subdir
                if ((*pSubDirsCounter) == 0){
                    //saving the path to the subdir
                    subChildDirPaths[0] = newPath;
                    (*pSubDirsCounter) += 1;
                }
                else{
                    //in case that this is not the first dir
                    free(newPath);
                    (*pSubDirsCounter) += 1;
                }
            }
        }
    }while(pdirent != NULL);
    //c file not found yet
    *pDepth += 1;
    //checking the number of subdirs
    if ((*pSubDirsCounter) == 0 || (*pSubDirsCounter) > 1){
        if ((*pSubDirsCounter) > 1){
            strcat(usersGradesBuffer, ",0,MULTIPLE_DIRECTORIES");
        }
        if ((*pSubDirsCounter) == 0){
            strcat(usersGradesBuffer, ",0,NO_C_FILE");
        }

        //closing the directory
        closeRes = closedir(childDir);
        //in case of failure
        if (closeRes < 0){
            perror("close faild");
            exit(-1);
        }

        free(fatherPath);
        return NULL;
    }
    else{
        //closing the directory
        closeRes = closedir(childDir);
        //in case of failure
        if (closeRes < 0){
            perror("close faild");
            exit(-1);
        }
        free(fatherPath);
        //the student has only one sub directory, depth first search
        FindCFileFunc(subChildDirPaths[0], pDepth, pSubDirsCounter, usersGradesBuffer);
    }
}//end of FindCFileFunc

/**********************************************************
 * function name: CompAndExecFunc.
 * the input: char* cFilePath, char* input, char* usersGradesBuffer,
 *  int* fdResFile.
 * the output: returns -1 in case of compilation error or in case of
 * timeout, returns 0 in case of success.
 * the function operation: the function creates new file called `resFile.txt`
 * compiles the passed c file, runs it with the inputs from `input`, and
 * writes the result to the new created file `resFile.txt`.
 *********************************************************/
int CompAndExecFunc(char* cFilePath, char* input, char* usersGradesBuffer, int* fdResFile){
    //will get the return value from fork() function
    pid_t pid;
    //will use us for wait() function
    int stat;
    //will get the return value of `open` on `input`
    int fdin;
    //will use us in order to check timeout
    pid_t timeOut;
    //will use us to check the return value of wait function
    int waitRetVal;
    //will use for success checking of close()
    int closeRes;
    //creating a new file that will hold the c file execution result
    *fdResFile = open("resFile.txt", O_CREAT|O_RDWR|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    //in case of failure
    if (*fdResFile < 0){
        perror("open faild");
        exit(-1);
    }
    //open the inupt file
    fdin = open(input, O_RDONLY);
    //in case of failure
    if (fdin < 0) {
        perror("open faild");
        exit(-1);
    }
    //in order to compile the c file we must use fork()
    if ((pid = fork()) == 0){//child process
        //compiling the c file
        execlp("gcc", "gcc", cFilePath, "-o", "a.out", NULL);
    }
    else{//father process
        waitRetVal = wait(&stat);
        if (waitRetVal < 0){
            perror("wait faild");
            exit(-1);
        }
        if (!WIFEXITED(stat)){
            printf("child exit failed\n");
        }
        else {
            //checking the exit status of the child (if the compilation worked)
            if (WEXITSTATUS(stat) != 0){
                strcat(usersGradesBuffer, ",0,COMPILATION_ERROR");
                return -1;
            }
            //creating another process in order to execute the a.out file
            if ((pid = fork()) == 0) {//child process
                //???
                //`fdin` gets the `job` of `0` fd - stdin
                dup2(fdin, 0);
                //`*fdResFile` gets the `job` of `1` fd - stdout
                dup2(*fdResFile, 1);
                //execute the a.out file
                execlp("./a.out", "./a.out", NULL);
            } else {//father process
                //checking the running time of the child process
                sleep(5);
                timeOut = waitpid(pid, &stat, WNOHANG);
                if (timeOut == 0){
                    strcat(usersGradesBuffer, ",0,TIMEOUT");
                    kill(pid, SIGSTOP);
                    return -1;
                }
                closeRes = close(fdin);
                if (closeRes  < 0){
                    perror("close faild");
                    exit(-1);
                }
                //means that compilation and execution went fine
                return 0;
            }
        }
    }
}//end of CompAndExecFunc

/**********************************************************
 * function name: CompareFunc.
 * the input: char* correctOutputFile, int* fdResFile.
 * the output: returns the result of `comp.out` execution.
 * the function operation: the function executes comp.out file with
 * `resFile.txt` and `correctOutputFile` as parameters, and returns the
 * result of the execution.
 *********************************************************/
int CompareFunc(char* correctOutputFile, int* fdResFile){
    //will get the return value from fork() function
    pid_t pid;
    //will use us for wait() function
    int stat;
    //will get the return value of comp.out execution
    int retVal;
    //will use us to check the return value of wait function
    int waitRetVal;
    //in order to check the success of `close` function
    int closeRes;
    //creating another process in order to execute the comp.out file
    if ((pid = fork()) == 0){//child process
        execlp("./comp.out", "./comp.out", "resFile.txt", correctOutputFile, NULL);
    }
    else{//father process
        waitRetVal = wait(&stat);
        if (waitRetVal < 0){
            perror("wait faild");
            exit(-1);
        }
        //checking the exit status of the child (if the compilation worked)
        if (!WIFEXITED(stat)){
            printf("child exit failed\n");
        }
        else{
            //checking the exit status of the child- the return value of comp.out
            retVal = WEXITSTATUS(stat);
            //closing the directory
            closeRes = close(*fdResFile);
            //in case of failure
            if (closeRes < 0){
                perror("close faild");
                exit(-1);
            }
            return retVal;
        }
    }
}//end of CompareFunc

/**********************************************************
 * function name: GradeCalculator.
 * the input: int* depth, int compResult, char* usersGradesBuffer.
 * the output: none. updates the text in `usersGradesBuffer`.
 * the function operation: the function `calculates` the right text
 * according to `depth` and `compResult` parameters.
 *********************************************************/
void GradeCalculator(int* depth, int compResult, char* usersGradesBuffer){
    //will use us in order to calculate the grade
    int gradeNum;
    //will use us in order to convert the integer grade value to string
    char gradeStr[3];
    memset(gradeStr, '\0', 3);
    //separate cases according to `depth` and `compResult` parameters
    if (compResult == 3 && *depth != 0){
        strcat(usersGradesBuffer, ",0,WRONG_DIRECTORY,BAD_OUTPUT");
        return;
    }
    if (compResult == 3 && *depth == 0){
        strcat(usersGradesBuffer, ",0,BAD_OUTPUT");
        return;
    }
    if (compResult == 2 && *depth != 0){
        gradeNum = (70 - 10*(*depth));
        sprintf(gradeStr, "%d", gradeNum);
        strcat(usersGradesBuffer, ",");
        strcat(usersGradesBuffer, gradeStr);
        strcat(usersGradesBuffer, ",WRONG_DIRECTORY,SIMILLAR_OUTPUT");
        return;
    }
    if (compResult == 2 && *depth == 0){
        strcat(usersGradesBuffer, ",70,SIMILLAR_OUTPUT");
        return;
    }
    if (compResult == 1 && *depth != 0){
        gradeNum = (100 - 10*(*depth));
        sprintf(gradeStr, "%d", gradeNum);
        strcat(usersGradesBuffer, ",");
        strcat(usersGradesBuffer, gradeStr);
        strcat(usersGradesBuffer, ",WRONG_DIRECTORY,GREAT_JOB");
        return;
    }
    if (compResult == 1 && *depth == 0){
        strcat(usersGradesBuffer, ",100,GREAT_JOB");
        return;
    }
}//end of GradeCalculator

/**********************************************************
 * function name: ResultsFileCreator.
 * the input: int fd, char* usersGradesBuffer.
 * the output: none.
 * the function operation: the function writes the content of
 * `usersGradesBuffer` into the file `results.csv`.
 *********************************************************/
void ResultsFileCreator(int fd, char* usersGradesBuffer){
    int writeRes;
    writeRes = write(fd, usersGradesBuffer, strlen(usersGradesBuffer));
    if (writeRes < 0){
        perror("write faild");
        exit(-1);
    }
    writeRes = write(fd, "\n", 1);
    if (writeRes < 0){
        perror("write faild");
        exit(-1);
    }
}//end of ResultsFileCreator