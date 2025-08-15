#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

    for (int i = 0; i < dbhdr->count; ++i) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours: %d\n", employees[i].hours);
    }

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    char* name = NULL;
    char* address = NULL;
    unsigned int hours = 0;

    unsigned short lastIndex = dbhdr->count - 1;

    //printf("%s\n", addstring);

    // "" because the delimiter is a string
    name = strtok(addstring, ",");
    address = strtok(NULL, ",");
    hours = atoi(strtok(NULL,","));


    strncpy(employees[lastIndex].name, name, sizeof(employees[lastIndex].name));
    strncpy(employees[lastIndex].address, address, sizeof(employees[lastIndex].address));

    employees[lastIndex].hours = hours; 

    //printf("%s %s %d\n", employees[lastIndex].name, employees[lastIndex].address, employees[lastIndex].hours);



    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    
    unsigned short count;
    struct employee_t* employees;

    // checks if fd is valid
    if (fd < 0) {
        printf("bad FD: output_file\n");
        
        return STATUS_ERROR;
    }

    count = dbhdr->count;

    employees = calloc(count, sizeof(struct employee_t));

    if (employees == NULL) {
        printf("Failed to calloc employees array: read_employees\n");

        return STATUS_ERROR;
    }

    // read count number of employees
    read(fd, employees, count*sizeof(struct employee_t));

    // change hours from network to host endian
    for (int i = 0; i < count; ++i) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {

    unsigned short count = dbhdr->count;
    unsigned int fileSize = sizeof(struct dbheader_t) + count * sizeof(struct employee_t); // calculating the total file size

    // checks if fd is valid
    if (fd < 0) {
        printf("bad FD: output_file\n");
        
        return STATUS_ERROR;
    }

    // converts from host endien to network endien

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->filesize = htonl(fileSize);

    // sets the file offset back to the top at byte 0.
    lseek(fd, 0, SEEK_SET);

    // write header
    write(fd, dbhdr, sizeof(struct dbheader_t));

    // write employee data
    for (int i = 0; i < count; ++i) {
        employees[i].hours = htonl(employees[i].hours);

        write(fd, &(employees[i]), sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {

    struct dbheader_t* header;
    struct stat dbstat = {0};

    int readSize;
    
    // checks if fd is valid
    if (fd < 0) {
        printf("bad FD: validate_db_header\n");
        
        return STATUS_ERROR;
    }


    header = calloc(1, sizeof(struct dbheader_t));

    if (header == NULL) {
        printf("Failed to calloc db header: validate_db_header\n");

        return STATUS_ERROR;
    }

    readSize = read(fd, header, sizeof(struct dbheader_t));

    if (readSize < sizeof(struct dbheader_t)) {
        perror("read");
        free(header);

        return STATUS_ERROR;
    }

    // ntohs and ntohl are used to change the value from network endien (n) to host endien (h). s = short, l = long

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    // cheaking if the file that we are dealing with is valid

    if (header->magic != HEADER_MAGIC) {

        printf("Improper header magic\n");

        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        printf("Improper header version\n");

        free(header);
        return STATUS_ERROR;
    }

    fstat(fd, &dbstat);

    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database: header file size does not match actual file size\n");

        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
	// we put header in the heap so we can access it in main
    struct dbheader_t* header = calloc(1, sizeof(struct dbheader_t));

    if (header == NULL) {
        printf("Failed to calloc db header: create_db_header\n");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}



/*

DESCRIPTION
       lseek() repositions the file offset of the open file description
       associated with the file descriptor fd to the argument offset
       according to the directive whence as follows:

       SEEK_SET
              The file offset is set to offset bytes.

       SEEK_CUR
              The file offset is set to its current location plus offset
              bytes.

       SEEK_END
              The file offset is set to the size of the file plus offset
              bytes.

       lseek() allows the file offset to be set beyond the end of the
       file (but this does not change the size of the file).  If data is
       later written at this point, subsequent reads of the data in the
       gap (a "hole") return null bytes ('\0') until data is actually
       written into the gap.
*/
