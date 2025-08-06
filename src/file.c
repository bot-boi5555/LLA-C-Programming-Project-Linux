#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"

#define rdwr_rd_rd 0644


int create_db_file(char *filename) {
    
    int fd;

    // check if the file with the name <filename> already exists.
    fd = open(filename, O_RDONLY);

    // if the open did not fail (i.e db file exists)
    if (fd != -1) {
        close(fd);
        printf("cannot create new db with the name %s. Db file already exists.\n", filename);

        return STATUS_ERROR;
    }

    // if the file does not exist

    fd = open(filename, O_RDWR | O_CREAT, rdwr_rd_rd);

    if (fd == -1) {
        perror("open: create_db_file");

        return STATUS_ERROR;
    }


    return fd;
}

int open_db_file(char *filename) {

    int fd = open(filename, O_RDWR);

    if (fd == -1) {
        perror("open: open_db_file");
        return STATUS_ERROR;
    }

    return fd;
}


