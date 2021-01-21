# C Mole - low-level, POSIX compliant file indexer 🤖

C Mole is a POSIX compliant, low-level file indexer that can be used on UNIX systems, like Linux or Mac OS. This app was written entirely by me as a project for Operating Systems course in the Warsaw University of Technology. After running, program traverses all files in a given directory and its subdirectories and waits for user input. User can then display some information about files or perform
some other operations, which are explained later. 

# Usage 💽

Download main.c file and run terminal in a directory, where the file has been downloaded. Compile it using:

`gcc main.c -o main -lpthread`

Program can be run using:

`./main -d <path to a directory that will be traversed> -f <path to a file where index is stored> -t <time between rebuilds of index>`

First argument can be replaced by `$MOLE_DIR` environmental variable. Second argument is optional but can also be replaced by `$MOLE_INDEX_PATH` environmental variable. When neither variable or path is present program creates index in user's home directory. Third argument has to be in range [30, 7200]. When it's not provided automatic reindexing is disabled.

