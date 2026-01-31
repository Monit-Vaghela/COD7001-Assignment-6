#include <stdio.h>
#include <errno.h>
#include <sys/ptrace.h>
#include "debugger/memory.h"
#include <sys/types.h>

static int read_word(unsigned long addr, pid_t pid, long *result) {
    /*
    Inputs :
        1) pid -> pid of the child 
        2) addr -> address we want to start the inspection from
        3) count -> number of word we want to inspect from
    
    Output :
        0  : Success
        -1 : Kernel Error
    */

    errno = 0;
    long word = ptrace(PTRACE_PEEKDATA, pid, (void *)addr, NULL);
    if(word == -1 && errno != 0){
        return -1;
    }
    *result = word;
    
    return 0;
}

void inspect_memory(unsigned long addr, pid_t pid, int count)
{
    int i = 0;                  
    unsigned long cursor = addr;     

    while (i < count) {
        long result;
        if (read_word(cursor, pid, &result) == -1) {
            perror("PTRACE_PEEKDATA");
            return;
        }

        // Result has 8 Bytes of data
        // User need byte by byte data
        // hence we convert our long to an char array
        // which is easier to print in hex values
        unsigned char *bytes = (unsigned char *)&result;

        printf("0x%016lx: ", cursor);

        /*
        To make sure we dont go above given count, .i.e
        user may give count 7 but out result has 8 bytes we
        do this neat little trick
        */
        int line_bytes;
        if (count - i >= 8) {
            line_bytes = 8;
        } else {
            line_bytes = count - i;
        }

        // print hex value of one byte
        for (int j = 0; j < line_bytes; j++) {
            printf("%02x ", bytes[j]);
        }

        // now we print a seperator
        for (int j = line_bytes; j < 8; j++)
            printf("   ");

        printf("|");

        // Now lets print the ASCII value of each hex
        for (int j = 0; j < line_bytes; j++) {
            unsigned char c = bytes[j];
            // we will only print between these values other wise we wont
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }

        printf("|\n");

        i += line_bytes;
        cursor += 8;
    }
}
