# C Mole - low-level, POSIX compliant file indexer 🤖

C Mole is a POSIX compliant, low-level file indexer that can be used on UNIX systems, like Linux or macOS. This app was written entirely by me as a project for Operating Systems course in the Warsaw University of Technology. After running, the program traverses all files in a given directory and its subdirectories and waits for user input. User can then display some information about files or perform some other operations, which are explained later. 

# Usage 💽

Download main.c file and run terminal in a directory, where the file has been downloaded. Compile it using:

`gcc main.c -o main -lpthread`

Program can be run using:

`./main -d <path to a directory that will be traversed> -f <path to a file where index is stored> -t <time between rebuilds of index>`

First argument can be replaced by `$MOLE_DIR` environmental variable. Second argument is optional but can also be replaced by `$MOLE_INDEX_PATH` environmental variable. When neither variable nor path is present, the program creates an index in user's home directory. The Third argument has to be in range [30, 7200]. When it's not provided automatic reindexing is disabled.

# Functions 🔧

At the start C Mole starts the process of indexing files. It supports the following formats:
- directories
- JPEG images
- PNG images
- gzip compressed files
- zip compressed files (including any files based on zip format like docx, odt etc.).

Index stores such information about each file or directory:
- file name
- an absolute path to a file
- size
- owner's uid
- type.

User can use a few commands to perform some operations:
- `exit` - program waits till indexing finishes and exits
- `exit!` - program terminates indexing, saves data and exits
- `index` - stars the reindexing procedure; if something changed, C Mole will notice it 🕵️
- `count` - program counts the number of each file type and prints it to the standard output
- `largerthan x` - displays information about files larger than some value
- `namepart x` - displays information about files which contain a given substring
- `owner uid` - displays files owned by a given user

It's worth mentioning that three last commands support pagination and can be displayed in programs like less. In order to do that you have to set an environmental variable `$PAGER` to the name of the wanted program. Otherwise, everything is displayed as standard output. 
