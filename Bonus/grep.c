#include "app.h"
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        INFO("Usage: grep [PATTERN] [FILE]");
        return 0;
    }
    
    char *pattern = argv[1];
    char *filename = argv[2];
    char buf[BLOCK_SIZE];
    char line[BLOCK_SIZE];
    int line_idx = 0;
    
    // Get the inode number for the file
    int file_ino = dir_lookup(workdir_ino, filename);
    if (file_ino < 0) {
        INFO("grep: %s: No such file", filename);
        return 1;
    }
    
    // Read the entire file block by block
    int offset = 0;
    
    while (1) {
        int result = file_read(file_ino, offset, buf);
        
        if (result != 0) {
            // Check if there's a final line without newline
            if (line_idx > 0) {
                line[line_idx] = '\0';
                if (strstr(line, pattern) != NULL) {
                    printf("%s\n", line);
                }
            }
            break;  // End of file or error
        }
        
        // Check if we should stop after this block
        int has_null = 0;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            if (buf[i] == '\0') {
                has_null = 1;
                break;
            }
        }
        
        // Process each character in the block
        for (int i = 0; i < BLOCK_SIZE && buf[i] != '\0'; i++) {
            char c = buf[i];
            
            if (c == '\n') {
                // End of line - check if pattern matches
                line[line_idx] = '\0';
                
                // Search for pattern in the line
                if (strstr(line, pattern) != NULL) {
                    printf("%s\n", line);
                }
                
                // Reset line buffer
                line_idx = 0;
            } else {
                // Add character to current line
                if (line_idx < BLOCK_SIZE - 1) {
                    line[line_idx++] = c;
                }
            }
        }
        
        // If we found a null terminator, check final line and stop
        if (has_null) {
            if (line_idx > 0) {
                line[line_idx] = '\0';
                if (strstr(line, pattern) != NULL) {
                    printf("%s\n", line);
                }
            }
            break;
        }
        
        // Move to next block
        offset += BLOCK_SIZE;
    }
    
    return 0;
}
