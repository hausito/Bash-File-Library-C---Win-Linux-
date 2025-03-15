#include "so_file_lib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_usage() {
    printf("Usage:\n");
    printf("  open <filename> <mode>\n");
    printf("  close\n");
    printf("  read <size>\n");
    printf("  write <text>\n");
    printf("  seek <offset> <whence>\n");
    printf("  exit\n");
}

int main() {
    SO_FILE* file = NULL;
    char command[256];
    char buffer[1024];

    while (1) {
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }

        // Remove newline
        command[strcspn(command, "\n")] = 0;

        // Parse command
        char* cmd = strtok(command, " ");
        if (!cmd) continue;

        if (strcmp(cmd, "open") == 0) {
            if (file) {
                printf("File already open. Close it first.\n");
                continue;
            }

            char* filename = strtok(NULL, " ");
            char* mode = strtok(NULL, " ");
            if (!filename || !mode) {
                print_usage();
                continue;
            }

            file = so_fopen(filename, mode);
            if (file) {
                printf("File opened successfully.\n");
            } else {
                printf("Failed to open file.\n");
            }
        }
        else if (strcmp(cmd, "close") == 0) {
            if (!file) {
                printf("No file is currently open.\n");
                continue;
            }

            if (so_fclose(file) == 0) {
                printf("File closed successfully.\n");
                file = NULL;
            } else {
                printf("Failed to close file.\n");
            }
        }
        else if (strcmp(cmd, "read") == 0) {
            if (!file) {
                printf("No file is currently open.\n");
                continue;
            }

            char* size_str = strtok(NULL, " ");
            if (!size_str) {
                print_usage();
                continue;
            }

            size_t size = atoi(size_str);
            if (size >= sizeof(buffer)) {
                printf("Read size too large.\n");
                continue;
            }

            size_t read_count = so_fread(buffer, 1, size, file);
            if (read_count > 0) {
                buffer[read_count] = '\0';
                printf("Read: %s\n", buffer);
            } else {
                printf("Failed to read from file.\n");
            }
        }
        else if (strcmp(cmd, "write") == 0) {
            if (!file) {
                printf("No file is currently open.\n");
                continue;
            }

            char* text = strtok(NULL, "\n");
            if (!text) {
                print_usage();
                continue;
            }

            size_t len = strlen(text);
            size_t written = so_fwrite(text, 1, len, file);
            if (written == len) {
                printf("Write successful.\n");
            } else {
                printf("Write failed.\n");
            }
        }
        else if (strcmp(cmd, "seek") == 0) {
            if (!file) {
                printf("No file is currently open.\n");
                continue;
            }

            char* offset_str = strtok(NULL, " ");
            char* whence_str = strtok(NULL, " ");
            if (!offset_str || !whence_str) {
                print_usage();
                continue;
            }

            long offset = atol(offset_str);
            int whence = atoi(whence_str);

            if (so_fseek(file, offset, whence) == 0) {
                printf("Seek successful.\n");
            } else {
                printf("Seek failed.\n");
            }
        }
        else if (strcmp(cmd, "exit") == 0) {
            if (file) {
                so_fclose(file);
            }
            break;
        }
        else {
            print_usage();
        }
    }

    return 0;
}