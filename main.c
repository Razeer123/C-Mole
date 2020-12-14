#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>

#define ERR(source) (perror(source), \
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
        exit(EXIT_FAILURE))
#define MAGIC_LENGTH 4
#define MAX_DEPTH 100
#define MAX_FILES 50
#define MAX_SIZE 100
#define MAX_PATH 100
#define BUFFER_SIZE 10

typedef struct indexData {

    char name[MAX_SIZE];
    char path[MAX_SIZE];
    long size;
    int uid;
    char type[MAX_SIZE];

} indexData_t;

typedef struct neededVariables {
    char dirPath[MAX_SIZE];
    char filePath[MAX_SIZE];
    pthread_t threadID;

} neededVariables_t;

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f);
void readArguments(int argc, char ** argv, char ** dirPath, char ** filePath, int * time);
void * threadWork(void * arguments);
void printFile(char * filePath);
void reindexFiles(neededVariables_t variablesStructure, int time);

indexData_t indexArray[MAX_FILES];
int start = 0;

int main(int argc, char ** argv) {

    // dirPath -> path in which we start indexing files
    // filePath -> path for index file
    // time -> time between rebuilds of index | optional

    char *dirPath = NULL;
    char *filePath = NULL;
    int time = 0;
    int reindex;
    int file;

    readArguments(argc, argv, &dirPath, &filePath, &time);
    // reindex -> stores boolean information about if reindexing will be happening
    reindex = time ? 1 : 0;

    neededVariables_t variablesStructure;
    strcpy(variablesStructure.dirPath, dirPath);

    // It's the beginning of the program, checks whether index the file for the first time or print the content

    if ((file = open(filePath, O_RDONLY, 0777)) < 0) {
        errno = 0;
/*        if (filePath != NULL) {
            strcpy(variablesStructure.filePath, filePath);
            if (((pthread_create(&variablesStructure.threadID, NULL, threadWork, &variablesStructure))) != 0) {
                ERR("Error in pthread_create.");
            }
        } else {*/
        char *functionPath = malloc(sizeof(char) * MAX_PATH);
        strcat(functionPath, dirPath);
        strcat(functionPath, "/file.mole_index");
        strcpy(variablesStructure.filePath, functionPath);
        memset(functionPath, 0, MAX_PATH);

        if (((pthread_create(&variablesStructure.threadID, NULL, threadWork, &variablesStructure))) != 0) {
            ERR("Error in pthread_create.");
        }
    } else {
        strcpy(variablesStructure.filePath, filePath);
    }

    // Waits for threads to finish their job
    // TODO: Add handling of this thing

    int err = pthread_join(variablesStructure.threadID, NULL);
    printFile(variablesStructure.filePath);

    // REMEMBER THAT FILE IS STILL OPENED

    if (reindex) {
        reindexFiles(variablesStructure, time);
    }

    // TODO: Wait for user input here

    return EXIT_SUCCESS;

}

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f) {

    int file;
    int * signature = malloc(MAGIC_LENGTH);
    if (signature == NULL) {
        ERR("Error when allocating memory.");
    }
    char * signatureHex = malloc(MAGIC_LENGTH);
    if (signatureHex == NULL) {
        ERR("Error when allocating memory.");
    }

    if ((file = open(fileName, O_RDONLY, 0777)) < 0) {
        ERR("Error when opening a file!");
    }

    // This should give us file signature that we'll compare with known values

    read(file, signature, MAGIC_LENGTH);
    sprintf(signatureHex, "%x", signature[0]);


    if (fileType == FTW_D || strstr(signatureHex, "ffd8") != NULL || (strstr(signatureHex, "474e5089") != NULL || strstr(signatureHex, "8088b1f") != NULL
    || strstr(signatureHex, "4034b50") != NULL)) {

        char * temp = strrchr(fileName, '/');
        strcpy(indexArray[start].name, temp + 1);
        indexArray[start].size = s->st_size;
        indexArray[start].uid = s->st_uid;
        strcpy(indexArray[start].path, fileName);

        if (strstr(signatureHex, "ffd8") != NULL) {
            strcpy(indexArray[start].type, "jpeg");
        } else if (strstr(signatureHex, "474e5089") != NULL) {
            strcpy(indexArray[start].type, "png");
        } else if (strstr(signatureHex, "8088b1f") != NULL) {
            strcpy(indexArray[start].type, "gzip");
        } else if (strstr(signatureHex, "4034b50") != NULL) {
            strcpy(indexArray[start].type, "zip");
        } else {
            strcpy(indexArray[start].type, "folder");
        }

        start++;

    }

    if (close(file) < 0) {
        ERR("Error when closing the file!");
    }

    return 0;

}

void readArguments(int argc, char ** argv, char ** dirPath, char ** filePath, int * time) {

    int c;
    char * dirEnv = getenv("MOLE_DIR");
    char * fileEnv = getenv("MOLE_INDEX_PATH");

    while((c = getopt(argc, argv, "d:f:t:")) != -1) {

        switch (c) {

            case 'd':
                *dirPath = optarg;
                break;
            case 'f':
                *filePath = optarg;
                break;
            case 't':
                * time = atoi(optarg);
                if (* time < 30 || * time > 7200) {
                    ERR("Time is not in a correct interval!");
                }
                break;
            default:
                ERR("Invalid argument!");

        }
    }

    if (*dirPath == NULL && dirEnv) {
        *dirPath = dirEnv;
    } else if (dirPath == NULL) {
        ERR("Program must have provide direction argument!");
    }

    if (*filePath == NULL && fileEnv) {
        *filePath = fileEnv;
    }
}

void * threadWork(void * arguments) {

    neededVariables_t * variables = arguments;
    char dirArray[MAX_PATH];
    char fileArray[MAX_PATH];
    strcpy(dirArray, variables->dirPath);
    strcpy(fileArray, variables->filePath);
    nftw(dirArray, recursiveWalk, MAX_DEPTH, FTW_PHYS);
    int file;

    // Opening file, creating if doesn't exist

    if ((file = open(fileArray, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666)) < 0) {
        ERR("Error when opening / creating file.");
    }

    // Writing data to index file

    for (int i = 0; i < start; i++) {

        write(file, "Name: ", 6);
        write(file, indexArray[i].name, strlen(indexArray[i].name));
        write(file, "\n", 1);
        write(file, "Path: ", 6);
        write(file, indexArray[i].path, strlen(indexArray[i].path));
        write(file, "\n", 1);

        char buffer[BUFFER_SIZE];
        sprintf(buffer, "%lu", indexArray[i].size);
        write(file, "Size: ", 6);
        write(file, buffer, strlen(buffer));
        write(file, "\n", 1);

        sprintf(buffer, "%d", indexArray[i].uid);
        write(file, "UID: ", 5);
        write(file, buffer, strlen(buffer));
        write(file, "\n", 1);

        write(file, "Type: ", 6);
        write(file, &indexArray[i].type, strlen(indexArray[i].type));
        write(file, "\n\n", 2);

    }

    fprintf(stdout, "Indexing finished.\n");
    close(file);

    return NULL;

}

void printFile(char * filePath) {

    int file;

    if ((file = open(filePath, O_RDONLY, 0666)) < 0) {
        ERR("Error when opening / creating file.");
    }

    char buffer[MAX_SIZE * 100];
    while (read(file, buffer, sizeof(buffer)) > 0) {
        fprintf(stdout, "%s", buffer);
    }

    memset(buffer, 0, sizeof(buffer));

    close(file);

}

// TODO: Somehow safeguard the index file when indexing

void reindexFiles(neededVariables_t variablesStructure, int time) {

    while (1) {

        sleep(time);

        if (((pthread_create(&variablesStructure.threadID, NULL, threadWork, &variablesStructure))) != 0) {
            ERR("Error in pthread_create.");
        }

        printFile(variablesStructure.filePath);

    }
}