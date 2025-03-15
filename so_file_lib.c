// so_file_lib.c
#include "so_file_lib.h"
#include <stdlib.h> // For malloc and free
#include <string.h> // For memset

// Helper function to parse mode
static int parse_mode(const char *mode,
#ifdef _WIN32
                      DWORD *dwDesiredAccess, DWORD *dwCreationDisposition
#else
                      int *oflags
#endif
) {
    if (strcmp(mode, "r") == 0) {
        #ifdef _WIN32
        *dwDesiredAccess = GENERIC_READ;
        *dwCreationDisposition = OPEN_EXISTING;
        #else
        *oflags = O_RDONLY;
        #endif
    }
    else if (strcmp(mode, "r+") == 0) {
        #ifdef _WIN32
        *dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        *dwCreationDisposition = OPEN_EXISTING;
        #else
        *oflags = O_RDWR;
        #endif
    }
    else if (strcmp(mode, "w") == 0) {
        #ifdef _WIN32
        *dwDesiredAccess = GENERIC_WRITE;
        *dwCreationDisposition = CREATE_ALWAYS;
        #else
        *oflags = O_WRONLY | O_CREAT | O_TRUNC;
        #endif
    }
    else if (strcmp(mode, "w+") == 0) {
        #ifdef _WIN32
        *dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        *dwCreationDisposition = CREATE_ALWAYS;
        #else
        *oflags = O_RDWR | O_CREAT | O_TRUNC;
        #endif
    }
    else if (strcmp(mode, "a") == 0) {
        #ifdef _WIN32
        *dwDesiredAccess = FILE_APPEND_DATA;
        *dwCreationDisposition = OPEN_ALWAYS;
        #else
        *oflags = O_WRONLY | O_CREAT | O_APPEND;
        #endif
    }
    else if (strcmp(mode, "a+") == 0) {
        #ifdef _WIN32
        *dwDesiredAccess = FILE_APPEND_DATA | GENERIC_READ;
        *dwCreationDisposition = OPEN_ALWAYS;
        #else
        *oflags = O_RDWR | O_CREAT | O_APPEND;
        #endif
    }
    else {
        return -1; // Unsupported mode
    }
    return 0;
}

SO_FILE *so_fopen(const char *pathname, const char *mode) {
    if (!pathname || !mode) {
        return NULL;
    }

    SO_FILE *file = (SO_FILE *)malloc(sizeof(SO_FILE));
    if (!file) {
        return NULL;
    }

    memset(file, 0, sizeof(SO_FILE));

    #ifdef _WIN32
    DWORD dwDesiredAccess = 0;
    DWORD dwCreationDisposition = 0;

    if (parse_mode(mode, &dwDesiredAccess, &dwCreationDisposition) != 0) {
        free(file);
        return NULL;
    }

    HANDLE handle = CreateFileA(
        pathname,
        dwDesiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        free(file);
        return NULL;
    }

    file->file_handle = handle;
    file->is_open = 1;

    #else
    int oflags = 0;

    if (parse_mode(mode, &oflags) != 0) {
        free(file);
        return NULL;
    }

    int fd = open(pathname, oflags, 0666); // rw-rw-rw- permissions if file is created 
    if (fd == -1) {
        free(file);
        return NULL;
    }

    file->file_descriptor = fd;
    file->is_open = 1;

    #endif

    return file;
}

int so_fclose(SO_FILE *stream) {
    if (!stream || !stream->is_open) {
        return -1;
    }

    int result = 0;

    #ifdef _WIN32
    if (!CloseHandle(stream->file_handle)) {
        result = -1;
    }
    #else
    if (close(stream->file_descriptor) == -1) {
        result = -1;
    }
    #endif

    free(stream);
    return result;
}

size_t so_fread(void *ptr, size_t size, size_t count, SO_FILE *stream) {
    if (!ptr || !stream || !stream->is_open || size == 0 || count == 0) {
        return 0;
    }

    size_t bytes_to_read = size * count;
    size_t bytes_read = 0;

    #ifdef _WIN32
    DWORD dwBytesRead = 0;
    BOOL success = ReadFile(
        stream->file_handle,
        ptr,
        (DWORD)bytes_to_read,
        &dwBytesRead,
        NULL
    );

    if (!success) {
        return 0;
    }

    bytes_read = dwBytesRead;
    #else
    bytes_read = read(stream->file_descriptor, ptr, bytes_to_read);
    if (bytes_read == (size_t)-1) {
        return 0;
    }
    #endif

    return bytes_read / size; // Return the number of objects read
}

size_t so_fwrite(const void *ptr, size_t size, size_t count, SO_FILE *stream) {
    if (!ptr || !stream || !stream->is_open || size == 0 || count == 0) {
        return 0;
    }

    size_t bytes_to_write = size * count;
    size_t bytes_written = 0;

    #ifdef _WIN32
    DWORD dwBytesWritten = 0;
    BOOL success = WriteFile(
        stream->file_handle,
        ptr,
        (DWORD)bytes_to_write,
        &dwBytesWritten,
        NULL
    );

    if (!success) {
        return 0;
    }

    bytes_written = dwBytesWritten;
    #else
    bytes_written = write(stream->file_descriptor, ptr, bytes_to_write);
    if (bytes_written == (size_t)-1) {
        return 0;
    }
    #endif

    return bytes_written / size; // Return the number of objects written
}

int so_fseek(SO_FILE *stream, long offset, int whence) {
    if (!stream || !stream->is_open) {
        return -1;
    }

    #ifdef _WIN32
    DWORD moveMethod;
    switch (whence) {
        case SEEK_SET:
            moveMethod = FILE_BEGIN;
            break;
        case SEEK_CUR:
            moveMethod = FILE_CURRENT;
            break;
        case SEEK_END:
            moveMethod = FILE_END;
            break;
        default:
            return -1;
    }

    LONG distanceLow = (LONG)offset;
    LONG distanceHigh = 0; // Assuming offset fits in LONG

    DWORD newPos = SetFilePointer(
        stream->file_handle,
        distanceLow,
        &distanceHigh,
        moveMethod
    );

    if (newPos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        return -1;
    }

    return 0;
    #else
    off_t result = lseek(stream->file_descriptor, offset, whence);
    if (result == (off_t)-1) {
        return -1;
    }
    return 0;
    #endif
}

long so_ftell(SO_FILE *stream) {
    if (!stream || !stream->is_open) {
        return -1L;
    }

    #ifdef _WIN32
    LARGE_INTEGER distance;
    distance.QuadPart = 0;
    BOOL success = SetFilePointerEx(
        stream->file_handle,
        distance,
        &distance,
        FILE_CURRENT
    );

    if (!success) {
        return -1L;
    }

    return (long)distance.QuadPart;
    #else
    off_t pos = lseek(stream->file_descriptor, 0, SEEK_CUR);
    if (pos == (off_t)-1) {
        return -1L;
    }
    return (long)pos;
    #endif
}

FD_TYPE so_get_fd(SO_FILE* stream) {
    if (!stream)
        return (FD_TYPE)-1;

    #ifdef _WIN32
        return stream->file_handle;
    #else
        return stream->file_descriptor;
    #endif
}