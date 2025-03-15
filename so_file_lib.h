#ifndef SO_FILE_H
#define SO_FILE_H

#include <stdio.h>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <fcntl.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
#endif

// Define FD_TYPE before it's used
#ifdef _WIN32
  #define FD_TYPE HANDLE
#else
  #define FD_TYPE int
#endif

typedef struct _so_file {
#ifdef _WIN32
    HANDLE file_handle;
#else
    int file_descriptor;
#endif
    int is_open; // Indicates whether the file is currently open
} SO_FILE;

// Function prototypes
SO_FILE *so_fopen(const char *pathname, const char *mode);
int so_fclose(SO_FILE *stream);
size_t so_fread(void *ptr, size_t size, size_t count, SO_FILE *stream);
size_t so_fwrite(const void *ptr, size_t size, size_t count, SO_FILE *stream);
int so_fseek(SO_FILE *stream, long offset, int whence);
long so_ftell(SO_FILE *stream);

// Auxiliary function for checker
FD_TYPE so_get_fd(SO_FILE* stream);

#endif // SO_FILE_H