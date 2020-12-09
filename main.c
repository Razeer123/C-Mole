#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>

#define ERR(source) (perror(source), \
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
        exit(EXIT_FAILURE))

// TODO: Add function that will open a file

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f);
void readArguments(int argc, char ** argv, char ** dirPath, char ** filePath, int * time);

int main(int argc, char ** argv) {

    // TODO: Add correct behaviours to when certain arguments are missing

    // dirPath -> path in which we start indexing files
    // filePath -> path for index file
    // time -> time between rebuilds of index | optional

    char * dirPath;
    char * filePath;
    int time = -1;

    readArguments(argc, argv, &dirPath, &filePath, &time);

    return EXIT_SUCCESS;

}

//TODO: Implement file recognition based on signature. It has to be created in a NEW THREAD!

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f) {

    if (fileType == FTW_F) {

    }

    return 0;

}

void readArguments(int argc, char ** argv, char ** dirPath, char ** filePath, int * time) {

    int c;

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
}
