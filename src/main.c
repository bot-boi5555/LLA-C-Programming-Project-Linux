#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv) {
    
    printf("Usage: %s -n -f <database file>\n", argv);
    printf("\t -f : (required) path to database file\n");
    printf("\t -n : create new database file\n");
    printf("\t -a : append a new employee to the specified database\n");
    printf("\t -l : print a list of the current employees in the specified database\n");
}


int main(int argc, char *argv[]) { 
	
    short opt;

    char* filePath = NULL;
    char* addString = NULL;
    bool newFile = false;
    bool listEmp = false;

    int dbfd = -1; // database file discreptor

    struct dbheader_t* header = NULL;
    struct employee_t* employees = NULL;


    while((opt = getopt(argc, argv, "nf:a:l")) != -1) {

        switch (opt) {

            case 'n':
                newFile = true;
                break;

            case 'f':
                filePath = optarg;
                break;

            case 'a':
                addString = optarg;
                break;

            case 'l':
                listEmp = true;
                break;

            default:
                print_usage(argv[0]);
                return 1;

        }
    }

    if (filePath == NULL) {
        printf("File path is a required argument\n");
        print_usage(argv[0]);
        return 1;
    }

    if (newFile) {
        dbfd = create_db_file(filePath);

        if (dbfd == STATUS_ERROR) {
            printf("Unable to create new database file\n");

            return -1;
        }
        
        if (create_db_header(dbfd, &header) == STATUS_ERROR) {

            printf("Failed to create database header\n");
            return -1;
        }


    }
    else {
        dbfd = open_db_file(filePath);

        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file.\n");

            return -1;
        }

        if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Header validation has failed\n");

            return -1;
        }

    }

    if (read_employees(dbfd, header, &employees) != STATUS_SUCCESS) {
        printf("Failed to read employees\n");

        return -1;
    }
    // add new employee
    if (addString != NULL) {
        header->count++;

        employees = realloc(employees, header->count * sizeof(struct employee_t));

        add_employee(header, employees, addString);
    }

    if (listEmp) {
        list_employees(header, employees);
    }

    output_file(dbfd, header, employees);


    return 0;
}
