#include "lh_files.h"
#include "lh_buffers.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

off_t get_file_length(const char * path) {
    struct stat st;
    if (stat(path, &st)) LH_ERROR(-1,"Failed to stat file %s",path);
    return st.st_size;
}

off_t get_file_length_f(FILE * fd) {
    struct stat st;
    int fn = fileno(fd);
    if (fn<0) LH_ERROR(-1,"Incorrect file descriptor","");
    if (fstat(fn, &st)) LH_ERROR(-1,"Failed to stat file descriptor %d",fn);
    return st.st_size;
}

unsigned char * read_file(const char * path, ssize_t *size) {
    FILE * fd = fopen(path, "r");
    if (!fd) LH_ERROR(NULL,"Failed to open %s for reading",path);
    unsigned char *buf = read_file_f(fd,size);
    fclose(fd);
    return buf;
}

unsigned char * read_file_f(FILE *fd, ssize_t *size) {
    rewind(fd);
    *size = get_file_length_f(fd);
    if (*size<0) return NULL;

    ALLOCB(buffer,*size);
    if (!buffer) LH_ERROR(NULL, "Failed to allocate buffer for %zd bytes",*size);
    
    ssize_t rbytes = fread(buffer,1,*size,fd);
    if (rbytes!=*size) {
        free(buffer);
        LH_ERROR(NULL,"Failed to read %zd bytes from file (only read %zd)",*size,rbytes);
    }

    return buffer;
}

int write_file(const char *path, const unsigned char *data, ssize_t size) {
    FILE * fd = open_file_w(path);
    if (!fd) return -1;
    ssize_t wbytes = fwrite(data, 1, size, fd);
    if (wbytes != size)
        LH_ERROR(-1,"Failed to write %zd bytes to %s (only wrote %zd)",*size,path,wbytes);
    fclose(fd);
    return 0;
}

FILE *open_file_r(const char *path, ssize_t *size) {
    FILE * fd = fopen(path, "r");
    if (!fd) LH_ERROR(NULL,"Failed to open %s for reading",path);
    *size = get_file_length_f(fd);
    if (*size<0) return NULL;
    return fd;
}

FILE *open_file_w(const char *path) {
    FILE * fd = fopen(path, "w");
    if (!fd) LH_ERROR(NULL,"Failed to open %s for reading",path);
    return fd;
}
