# This is a readme file


Notes: 

I am writing this on a windows device. In order to continue with the project, i made some modifactions to the make file and fudged in a few
libraries that are already included in minGW.

i. makefile

1. changing rm to del in order to remove the old files
2. adding -lws2_32 after gcc -o $@ $? so that the linker can find where the code for winsock2 can be found

ii. code:

1. os.h in order to use open, close, and read
2. winsock2.h instead of arpa/inet.h for ntohs, ntohl, htons, and htonl
