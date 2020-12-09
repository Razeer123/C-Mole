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

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f);

int main() {

    return EXIT_SUCCESS;

}

// Function that will be counting folders and types

int recursiveWalk(const char * fileName, const struct stat * s, int fileType, struct FTW * f) {

    if (fileType == FTW_F) {

    }

}
