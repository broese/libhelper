#include "lh_files.h"
#include "lh_buffers.h"
#include "lh_debug.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

////////////////////////////////////////////////////////////////////////////////
// stat-related functions

off_t filesize(const char * path) {
    struct stat st;
    if (stat(path, &st)) LH_ERROR(-1,"Failed to stat file %s",path);
    return st.st_size;
}

off_t filesize_fp(FILE * fd) {
    struct stat st;
    int fn = fileno(fd);
    if (fn<0) LH_ERROR(-1,"Incorrect file descriptor","");
    if (fstat(fn, &st)) LH_ERROR(-1,"Failed to stat file descriptor %d",fn);
    return st.st_size;
}

////////////////////////////////////////////////////////////////////////////////
// file opening

static FILE *open_file(const char *path, const char *mode, const char *smode, off_t *size) {
    FILE * fd = fopen(path, mode);
    if (!fd) LH_ERROR(NULL,"Failed to open %s for %s",path,smode);

    if (size) {
        *size = filesize_fp(fd);
        if (*size<0) { fclose(fd); return NULL; }
    }
    return fd;
}

FILE *open_file_r(const char *path, off_t *size) {
    return open_file(path,"r","reading",size);
}

FILE *open_file_w(const char *path) {
    off_t size;
    return open_file(path,"w","writing",&size);
}

FILE *open_file_u(const char *path, off_t *size) {
    return open_file(path,"r+","updating",size);
}

FILE *open_file_a(const char *path, off_t *size) {
    return open_file(path,"a","appending",size);
}

////////////////////////////////////////////////////////////////////////////////
// reading files

uint8_t * read_fp(FILE *fp, ssize_t size) {
    ALLOCB(buffer, size);                                      
    if (!buffer)                                               
        LH_ERROR(NULL, "Failed to allocate %zd bytes\n", size);

    if (read_fp_to(fp, buffer, size)<0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

uint8_t * read_fp_at(FILE *fp, off_t pos, ssize_t size) {
    if (fseeko(fp, pos, SEEK_SET)<0)                                    
        LH_ERROR(NULL, "Failed to seek to offset %jd", (intmax_t) pos);
    return read_fp(fp, size);
}

uint8_t * read_fp_whole(FILE *fp, ssize_t *size) {
    // get file size
    off_t fsize = filesize_fp(fp);
    if (fsize<0) return NULL;

    // check if the file size theoretically fits into memory
    // FIXME: limited to 2GiB on 32-bit platforms
    if (sizeof(off_t)>sizeof(size_t) && fsize >= SSIZE_MAX)
        LH_ERROR(NULL, "File size (%jd) exceeds limits of ssize_t (%d bits)", (intmax_t)fsize, sizeof(ssize_t)/8 );
    *size = (ssize_t)fsize;

    return read_fp_at(fp, 0, *size);
}

uint8_t * read_file_whole(const char * path, ssize_t *size) {
    FILE * fd = open_file_r(path, NULL);
    if (!fd) return NULL;

    uint8_t *buf = read_fp_whole(fd, size);
    fclose(fd);
    return buf;
}

uint8_t * read_file_at(const char * path, off_t pos, ssize_t size) {
    FILE * fd = open_file_r(path, NULL);
    if (!fd) return NULL;

    uint8_t *buf = read_fp_at(fd, pos, size);
    fclose(fd);
    return buf;
}

int read_fp_to(FILE *fp, uint8_t * buffer, ssize_t size) {
    ssize_t rbytes = fread(buffer,1,size,fp);                           
    if (rbytes!=size)
        LH_ERROR(-1,"Failed to read %zd bytes from file (only read %zd)",size,rbytes);
    return 0;
}

int read_fp_at_to(FILE *fp, uint8_t * buffer, off_t pos, ssize_t size) {
    if (fseeko(fp, pos, SEEK_SET)<0)                                    
        LH_ERROR(-1, "Failed to seek to offset %jd", (intmax_t) pos);
    return read_fp_to(fp, buffer, size);
}

int read_file_at_to(const char * path, uint8_t * buffer, off_t pos, ssize_t size) {
    FILE * fd = open_file_r(path, NULL);
    if (!fd) return -1;

    int res = read_fp_at_to(fd, buffer, pos, size);
    fclose(fd);

    return res;
}








#if 0


unsigned char * read_file_f(FILE *fd, ssize_t *size) {
    rewind(fd);
    off_t fsize;
    fsize = filesize_fp(fd);
    if (fsize<0) return NULL;
    if (sizeof(*size)==4 && fsize>=(1<<31))
        LH_ERROR(NULL,"File size exceeds %d-bit representation", sizeof(*size)*8);

    ALLOCB(buffer,*size);
    if (!buffer) LH_ERROR(NULL, "Failed to allocate buffer for %zd bytes",*size);
    
    ssize_t rbytes = fread(buffer,1,*size,fd);
    if (rbytes!=*size) {
        free(buffer);
        LH_ERROR(NULL,"Failed to read %zd bytes from file (only read %zd)",*size,rbytes);
    }

    return buffer;
}

////////////////////////////////////////////////////////////////////////////////
// reading and writing file fragments

int write_to(FILE *fd, off_t offset, const unsigned char *data, ssize_t size) {
    if (fseek(fd, offset,SEEK_SET))
        LH_ERROR(-1,"Failed to seek to position %jd\n",(intmax_t)offset);
    return write_file_f(fd, data, size);
}

int read_from(FILE *fd, off_t offset, unsigned char *buffer, ssize_t size) {
    if (fseek(fd, offset,SEEK_SET))
        LH_ERROR(-1,"Failed to seek to position %jd\n",(intmax_t)offset);

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

#endif

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
    off_t oldsize;
    FILE * fd = open_file_u(path, &oldsize);
    if (!fd) return -1;
    int result = write_file_f(fd,data,size);
    fclose(fd);
    return result;
}

