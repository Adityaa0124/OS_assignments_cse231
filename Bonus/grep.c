/*
 * grep.c - Fixed to use workdir_ino
 */

#include "app.h"
#include <string.h>

#define BUF_SIZE 4096   
#define MAX_LINE 1024   

int main(int argc, char *argv[]) {
    // Check arguments
    if (argc != 3) {
        printf("Usage: grep [PATTERN] [FILE]\n");
        exit(1);
    }

    char *pattern = argv[1];
    char *filename = argv[2];

    // FIX: Use workdir_ino instead of 0
    int file_ino = dir_lookup(workdir_ino, filename);

    if (file_ino < 0) {
        printf("grep: cannot open %s\n", filename);
        exit(1);
    }

    char buf[BUF_SIZE];      
    char line[MAX_LINE];     
    int line_pos = 0;        
    int offset = 0;          
    int nread;

    // Read loop
    while ((nread = file_read(file_ino, offset, buf)) > 0) {
        for (int i = 0; i < nread; i++) {
            char c = buf[i];

            if (line_pos < MAX_LINE - 1) {
                line[line_pos++] = c;
            }

            if (c == '\n') {
                line[line_pos] = '\0'; 
                if (strstr(line, pattern) != NULL) {
                    printf("%s", line); 
                }
                line_pos = 0;
            }
        }
        offset += BUF_SIZE;
    }

    // Handle last line if no newline at EOF
    if (line_pos > 0) {
        line[line_pos] = '\0';
        if (strstr(line, pattern) != NULL) {
            printf("%s\n", line); 
        }
    }

    exit(0);
}

