#include "app.h"
#include <string.h>

int main(int argc, char **argv){
    if (argc < 2){
        INFO("Usage: wcl [FILE1] [FILE2] ...");
        return 0;
    }
    int total_lines = 0;
    char buf[BLOCK_SIZE];
    for (int file_idx = 1; file_idx < argc; file_idx++){
        char *filename = argv[file_idx];
        int file_ino = dir_lookup(workdir_ino, filename);
        if (file_ino < 0){
            INFO("wcl: cannot open '%s': No such file", filename);
            continue; }
        int file_has_content = 0;
        int last_char_was_newline = 1; // Assume true initially
        int offset = 0;
        while (1){
            int result = file_read(file_ino, offset, buf);
            if (result != 0){
                break;
            }
            for (int i = 0; i < BLOCK_SIZE && buf[i] != '\0'; i++){
                file_has_content = 1;
                if (buf[i] == '\n'){
                    total_lines++;
                    last_char_was_newline = 1; }
                else{
                    last_char_was_newline = 0;
                }
            }
            int has_null = 0;
            for (int i = 0; i < BLOCK_SIZE; i++){
                if (buf[i] == '\0'){
                    has_null = 1;
                    break;
                }
            }
            if (has_null){
                break;
            }
            offset += BLOCK_SIZE;
        }
        if (file_has_content && !last_char_was_newline){
            total_lines++;
        }
    }
    printf("%d\n", total_lines);
    return 0;
}
