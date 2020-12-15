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
    int time;
} neededVariables_t;

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f);
void readArguments(int argc, char ** argv, char ** dirPath, char ** filePath, int * time);
void * threadWork(void * arguments);
void printFile(char * filePath);
void * reindexFiles(void * arguments);
int safelyExitProgram(int file);
int forceExitProgram(int file, pthread_t pid);
void countEachFile();
void findLargerFiles(long size);
void writeToStructure(char * filePath);
void * forceIndexFiles(void * arguments);
void findByNamePart(char * namePart);
void findByID(int uid);

indexData_t indexArray[MAX_FILES];
int start = 0;
int indexingInProgress = 0;
int savingInProgress = 0;

int main(int argc, char ** argv) {

    // dirPath -> path in which we start indexing files
    // filePath -> path for index file
    // time -> time between rebuilds of index | optional

    char *dirPath = NULL;
    char *filePath = NULL;
    int time = 0;
    int reindex;
    int file;
    int err;

    readArguments(argc, argv, &dirPath, &filePath, &time);
    // reindex -> stores boolean information about if reindexing will be happening
    reindex = time ? 1 : 0;

    neededVariables_t variablesStructure;
    strcpy(variablesStructure.dirPath, dirPath);
    variablesStructure.time = time;

    // It's the beginning of the program, checks whether index the file for the first time or print the content

    if ((file = open(filePath, O_RDONLY, 0777)) < 0) {
        errno = 0;
        char *functionPath = malloc(sizeof(char) * MAX_PATH);
        strcat(functionPath, dirPath);
        strcat(functionPath, "/file.mole_index");
        strcpy(variablesStructure.filePath, functionPath);
        memset(functionPath, 0, MAX_PATH);

        if (((pthread_create(&variablesStructure.threadID, NULL, threadWork, &variablesStructure))) != 0) {
            ERR("Error in pthread_create.");
        }

        if (err = pthread_join(variablesStructure.threadID, NULL) != 0) {
            ERR("Error in pthread_join.");
        }

    } else {
        strcpy(variablesStructure.filePath, filePath);
        writeToStructure(variablesStructure.filePath);
    }

    // Waits for threads to finish their job

    printFile(variablesStructure.filePath);

    // REMEMBER THAT FILE IS STILL OPENED

    if (reindex) {
        if (((pthread_create(&variablesStructure.threadID, NULL, reindexFiles, &variablesStructure))) != 0) {
            ERR("Error in pthread_create.");
        }
    }

    // TODO: Wait for user input here

    // Main menu

    char input[MAX_SIZE];

    while (1) {

        scanf("%[^\n]%*c", input);

        if ((strcmp(input, "exit")) == 0) {
            safelyExitProgram(file);
        } else if (strcmp(input, "exit!") == 0) {
            forceExitProgram(file, variablesStructure.threadID);
        } else if (strcmp(input, "index") == 0) {
            if (indexingInProgress == 1) {
                fprintf(stderr, "Indexing is already in progress!\n");
                continue;
            } else {
                do {
                    sleep(1);
                } while (savingInProgress == 1);
                if (((pthread_create(&variablesStructure.threadID, NULL, forceIndexFiles, &variablesStructure))) != 0) {
                    ERR("Error in pthread_create.");
                }
            }
        } else if (strcmp(input, "count") == 0) {
            countEachFile();
        } else if (strstr(input, "largerthan") != NULL) {
            char temp[MAX_SIZE];
            memcpy(temp, &input[11], strlen(input));
            long number = atoi(temp);
            findLargerFiles(number);
        } else if (strstr(input, "namepart") != NULL) {
            char temp[MAX_SIZE];
            memcpy(temp, &input[9], strlen(input));
            findByNamePart(temp);
        } else if (strstr(input, "owner uid") != NULL) {
            char temp[MAX_SIZE];
            memcpy(temp, &input[10], strlen(input));
            int number = atoi(temp);
            findByID(number);
        } else {
            fprintf(stderr, "Incorrect command! Try again.\n");
        }
    }

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

    savingInProgress = 1;

    for (int i = 0; i < start; i++) {

        //write(file, "Name: ", 6);
        write(file, indexArray[i].name, strlen(indexArray[i].name));
        write(file, "\n", 1);
        //write(file, "Path: ", 6);
        write(file, indexArray[i].path, strlen(indexArray[i].path));
        write(file, "\n", 1);

        char buffer[BUFFER_SIZE];
        sprintf(buffer, "%lu", indexArray[i].size);
        //write(file, "Size: ", 6);
        write(file, buffer, strlen(buffer));
        write(file, "\n", 1);

        sprintf(buffer, "%d", indexArray[i].uid);
        //write(file, "UID: ", 5);
        write(file, buffer, strlen(buffer));
        write(file, "\n", 1);

        //write(file, "Type: ", 6);
        write(file, &indexArray[i].type, strlen(indexArray[i].type));
        write(file, "\n", 1);

    }

    write(file, "END\n\0", 5);

    savingInProgress = 0;
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

void * reindexFiles(void * arguments) {

    neededVariables_t * variables = arguments;
    start = 0;
    memset(indexArray, 0, sizeof indexArray);

    while (1) {

        sleep(variables->time);
        indexingInProgress = 1;
        threadWork(variables);
        indexingInProgress = 0;

    }
}

int forceExitProgram(int file, pthread_t pid) {
    do {
        sleep(1);
    } while (savingInProgress == 1);
    pthread_cancel(pid);
    close(file);
    exit(EXIT_SUCCESS);
}

int safelyExitProgram(int file) {
    do {
        sleep(1);
    } while (indexingInProgress == 1);
    close(file);
    exit(EXIT_SUCCESS);
}

void writeToStructure(char * filePath) {

    int file;

    if ((file = open(filePath, O_RDONLY, 0666)) < 0) {
        ERR("Error when opening / creating file.");
    }

    char buffer[MAX_SIZE * 100];
    char * token;
    while (read(file, buffer, sizeof(buffer)) > 0);
    int i = 0;
    int j = 0;

    token = strtok(buffer, "\n");
    strcpy(indexArray[j].name, token);
    i++;

    while (token != NULL) {
        token = strtok(NULL, "\n");
        if (strcmp(token, "END") == 0) {
            break;
        }
        if (i == 0) {
            strcpy(indexArray[j].name, token);
            i++;
        } else if (i == 1) {
            strcpy(indexArray[j].path, token);
            i++;
        } else if (i == 2) {
            indexArray[j].size = atoi(token);
            i++;
        } else if (i == 3) {
            indexArray[j].uid = atoi(token);
            i++;
        } else {
            strcpy(indexArray[j].type, token);
            i++;
        }
        if (i == 5) {
            i = 0;
            j++;
            start++;
        }
    }

    memset(buffer, 0, sizeof(buffer));
    close(file);

}

void countEachFile() {

    int directories = 0;
    int jpeg = 0;
    int png = 0;
    int gzip = 0;
    int zip = 0;

    for (int i = 0; i < start; i++) {
        if ((strcmp(indexArray[i].type, "folder")) == 0) {
            directories++;
        } else if ((strcmp(indexArray[i].type, "jpeg")) == 0) {
            jpeg++;
        } else if ((strcmp(indexArray[i].type, "png")) == 0) {
            png++;
        } else if ((strcmp(indexArray[i].type, "gzip")) == 0) {
            gzip++;
        } else {
            zip++;
        }
    }

    printf("In index there are %d directories, %d jpeg files, %d png files, %d gzip files and %d zip files.\n", directories, jpeg, png, gzip, zip);

}

void findLargerFiles(long size) {

    for (int i = 0; i < start; i++) {
        if (indexArray[i].size > size) {
            printf("%s\n", indexArray[i].path);
            printf("%lu\n", indexArray[i].size);
            printf("%s\n\n", indexArray[i].type);
        }
    }
}

void * forceIndexFiles(void * arguments) {

    neededVariables_t * variables = arguments;
    start = 0;
    memset(indexArray, 0, sizeof indexArray);

    if (!indexingInProgress) {
        indexingInProgress = 1;
        threadWork(variables);
        indexingInProgress = 0;
    }

    return NULL;

}

void findByNamePart(char * namePart) {

    for (int i = 0; i < start; i++) {
        if (strstr(indexArray[i].name, namePart) != NULL) {
            printf("%s\n", indexArray[i].path);
            printf("%lu\n", indexArray[i].size);
            printf("%s\n\n", indexArray[i].type);
        }
    }
}

void findByID(int uid) {

    for (int i = 0; i < start; i++) {
        if (indexArray[i].uid == uid) {
            printf("%s\n", indexArray[i].path);
            printf("%lu\n", indexArray[i].size);
            printf("%s\n\n", indexArray[i].type);
        }
    }
}