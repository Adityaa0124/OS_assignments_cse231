#include "app.h"
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        INFO("Usage: grep [PATTERN] [FILE]");
        return 0;
    }

    char *pattern = argv[1];
    char *filename = argv[2];
    char buf[BLOCK_SIZE];
    char line[BLOCK_SIZE];
    int line_idx = 0;

    int file_ino = dir_lookup(workdir_ino, filename);
    if (file_ino < 0)
    {
        INFO("grep: %s: No such file", filename);
        return 1;
    }

    int offset = 0;

    while (1)
    {
        int result = file_read(file_ino, offset, buf);

        if (result != 0)
        {
            if (line_idx > 0)
            {
                line[line_idx] = '\0';
                if (strstr(line, pattern) != NULL)
                {
                    printf("%s\n", line);
                }
            }
            break;
        }

        int has_null = 0;
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            if (buf[i] == '\0')
            {
                has_null = 1;
                break;
            }
        }

        for (int i = 0; i < BLOCK_SIZE && buf[i] != '\0'; i++)
        {
            char c = buf[i];

            if (c == '\n')
            {
                line[line_idx] = '\0';

                if (strstr(line, pattern) != NULL)
                {
                    printf("%s\n", line);
                }

                line_idx = 0;
            }
            else
            {
                if (line_idx < BLOCK_SIZE - 1)
                {
                    line[line_idx++] = c;
                }
            }
        }

        if (has_null)
        {
            if (line_idx > 0)
            {
                line[line_idx] = '\0';
                if (strstr(line, pattern) != NULL)
                {
                    printf("%s\n", line);
                }
            }
            break;
        }
        offset += BLOCK_SIZE;
    }
    return 0;
}
