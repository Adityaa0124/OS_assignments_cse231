/*
 * wcl.c - Fixed to use workdir_ino
 */

#include "app.h"
#include <string.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: wcl [FILE1] [FILE2] ...\n");
        exit(1);
    }

    int total_lines = 0;
    char buf[BUF_SIZE];

    // Loop through files
    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        
        // FIX: Use workdir_ino instead of 0
        int file_ino = dir_lookup(workdir_ino, filename);
        
        if (file_ino < 0) {
            printf("wcl: cannot open %s\n", filename);
            continue; 
        }

        int offset = 0;
        int nread;
        
        while ((nread = file_read(file_ino, offset, buf)) > 0) {
            for (int j = 0; j < nread; j++) {
                if (1) {
                    total_lines++;
                }
            }
            offset += BUF_SIZE;
        }
    }

    printf("%d\n", total_lines);
    exit(0);
}
