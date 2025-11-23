#include "app.h"
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        INFO("Usage: wcl [FILE1] [FILE2] ...");
        return 0;
    }
    
    int total_lines = 0;
    char buf[BLOCK_SIZE];
    
    // Process each file argument
    for (int file_idx = 1; file_idx < argc; file_idx++) {
        char *filename = argv[file_idx];
        
        // Get the inode number for the file
        int file_ino = dir_lookup(workdir_ino, filename);
        if (file_ino < 0) {
            INFO("wcl: cannot open '%s': No such file", filename);
            continue;
        }
        
        // Track if file has any content and if last char was newline
        int file_has_content = 0;
        int last_char_was_newline = 1;  // Assume true initially
        
        // Read the entire file block by block
        int offset = 0;
        
        while (1) {
            int result = file_read(file_ino, offset, buf);
            
            if (result != 0) {
                break;  // End of file or error
            }
            
            // Process each character in this block
            for (int i = 0; i < BLOCK_SIZE && buf[i] != '\0'; i++) {
                file_has_content = 1;
                
                if (buf[i] == '\n') {
                    total_lines++;
                    last_char_was_newline = 1;
                } else {
                    last_char_was_newline = 0;
                }
            }
            
            // Check if we've reached the end of content in this block
            int has_null = 0;
            for (int i = 0; i < BLOCK_SIZE; i++) {
                if (buf[i] == '\0') {
                    has_null = 1;
                    break;
                }
            }
            
            // If we found a null terminator, this is the last block
            if (has_null) {
                break;
            }
            
            // Move to next block
            offset += BLOCK_SIZE;
        }
        
        // If file has content and doesn't end with newline, count the last line
        if (file_has_content && !last_char_was_newline) {
            total_lines++;
        }
    }
    
    // Print total line count
    printf("%d\n", total_lines);
    
    return 0;
}
