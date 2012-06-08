#include "lh_files.h"
#include "lh_buffers.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

////////////////////////////////////////////////////////////////////////////////
// stat-related functions

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

////////////////////////////////////////////////////////////////////////////////
// file opening

static FILE *open_file(const char *path, const char *mode, const char *smode, ssize_t *size) {
    FILE * fd = fopen(path, mode);
    if (!fd) LH_ERROR(NULL,"Failed to open %s for %s",path,smode);
    *size = get_file_length_f(fd);
    if (*size<0) { fclose(fd); return NULL; }
    return fd;
}

FILE *open_file_r(const char *path, ssize_t *size) {
    return open_file(path,"r","reading",size);
}

FILE *open_file_w(const char *path) {
    ssize_t size;
    return open_file(path,"w","writing",&size);
}

FILE *open_file_u(const char *path, ssize_t *size) {
    return open_file(path,"r+","updating",size);
}

FILE *open_file_a(const char *path, ssize_t *size) {
    return open_file(path,"a","appending",size);
}

////////////////////////////////////////////////////////////////////////////////
// reading and writing entire files

unsigned char * read_file(const char * path, ssize_t *size) {
    FILE * fd = open_file_r(path, size);
    if (!fd) return NULL;
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

int write_file_f(FILE *fd, const unsigned char *data, ssize_t size) {
    ssize_t wbytes = fwrite(data, 1, size, fd);
    if (wbytes != size)
        LH_ERROR(-1,"Failed to write %zd bytes to file (only wrote %zd)",size,wbytes);
    return 0;
}

int write_file(const char *path, const unsigned char *data, ssize_t size) {
    FILE * fd = open_file_w(path);
    if (!fd) return -1;
    int result = write_file_f(fd,data,size);
    fclose(fd);
    return result;
}

int append_file(const char *path, const unsigned char *data, ssize_t size) {
    ssize_t oldsize;
    FILE * fd = open_file_u(path, &oldsize);
    if (!fd) return -1;
    int result = write_file_f(fd,data,size);
    fclose(fd);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// reading and writing file fragments

int write_to(FILE *fd, off_t offset, const unsigned char *data, ssize_t size) {
    if (fseek(fd, offset,SEEK_SET))
        LH_ERROR(-1,"Failed to seek to position %lu\n",offset);
    return write_file_f(fd, data, size);
}

int read_from(FILE *fd, off_t offset, unsigned char *buffer, ssize_t size) {
    if (fseek(fd, offset,SEEK_SET))
        LH_ERROR(-1,"Failed to seek to position %lu\n",offset);

    ssize_t rbytes = fread(buffer,1,size,fd);
    if (rbytes!=size)
        LH_ERROR(-1,"Failed to read %zd bytes from file (only read %zd)",size,rbytes);

    return 0;
}

unsigned char * read_froma(FILE *fd, off_t offset, ssize_t size) {
    ALLOCB(buffer,size);
    if (!buffer) LH_ERROR(NULL, "Failed to allocate buffer for %zd bytes",size);

    if (read_from(fd, offset, buffer, size)) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

