#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_int.h"

/************************** Defining Constants for fs *******************/
#define LINESIZE 128
#define DISK_PARTITION 4000000
#define BLOCK_SIZE 5000
#define BLOCKS 4000000/5000
#define MAX_STRING_LENGTH 20
#define MAX_FILE_DATA_BLOCKS (BLOCK_SIZE-64*59) //Hard-coded as of now
#define MAX_SUBDIRECTORIES  (BLOCK_SIZE - 136)/MAX_STRING_LENGTH

int main (int argc, char *argv[]) {

    if (0 != (init_fresh_disk("disk.img", BLOCK_SIZE, BLOCKS))) {
        printf("ERROR: cannot create the file disk\n");
        exit(1);
    }

    close_disk();

    return 0;

}