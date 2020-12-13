#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERR(source) (perror(source), \
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
        exit(EXIT_FAILURE))
#define MAGIC_LENGTH 4
#define MAX_DEPTH 100
#define MAX_FILES 50
#define MAX_SIZE 100

// TODO: Verify that the function that opens a file works
// FIXME: Fix recursiveWalk so that it indexes files properly

int openFile(FILE ** FILE, char * filePath);
int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f);
void readArguments(int argc, char ** argv, char ** dirPath, char ** filePath, int * time);

// TODO: Fill this struct with that that we have to index
// TODO: Create an array of structs to keep all indexed data

typedef struct indexData {

    char name[MAX_SIZE];
    char path[MAX_SIZE];
    long size;
    int uid;
    char type[MAX_SIZE];

} indexData_t;
indexData_t indexArray[MAX_FILES];
int start = 0;

int main(int argc, char ** argv) {

    // dirPath -> path in which we start indexing files
    // filePath -> path for index file
    // time -> time between rebuilds of index | optional

    char * dirPath = NULL;
    char * filePath = NULL;
    int time = 0;
    int reindex;
    FILE * file;

    readArguments(argc, argv, &dirPath, &filePath, &time);
    // reindex -> stores boolean information about if reindexing will be happening
    reindex = time ? 1 : 0;

    // TODO: Write some code so that function really works
    if (openFile(&file, filePath) == 0) {
        // function that should recursively traverse given location
        nftw(dirPath, recursiveWalk, MAX_DEPTH, FTW_PHYS);
    } else {
        // read index from the file
    }

    for (int i = 0; i < start; i++) {
        printf("%s\n", indexArray[i].name);
        printf("%s\n", indexArray[i].type);
        printf("%s\n", indexArray[i].path);
        printf("%d\n", indexArray[i].uid);
        printf("%lu\n\n"
               "", indexArray[i].size);
    }

    // TODO: Wait for user input here

    return EXIT_SUCCESS;

}

//TODO: Add a thread after implementing the algorithm, later.

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f) {

    // TODO: Open each file (open function gets binary input). Compare few bytes with signature to get file type.
    // TODO: Fill structure with given data. If it's not a correct file, discard it.

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

int openFile(FILE ** file, char * filePath) {

    // Opens a file and checks correctness of the operation

    if ((*file = fopen(filePath, "w+")) == NULL) {
        return 0;
    }

    return 1;
}