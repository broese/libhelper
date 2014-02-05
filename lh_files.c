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

off_t lh_filesize_path(const char * path) {
    struct stat st;
    if (stat(path, &st)) LH_ERROR(-1,"Failed to stat file %s",path);
    return st.st_size;
}

off_t lh_filesize_fp(FILE * fd) {
    struct stat st;
    int fn = fileno(fd);
    if (fn<0) LH_ERROR(-1,"Incorrect file descriptor","");
    if (fstat(fn, &st)) LH_ERROR(-1,"Failed to stat file descriptor %d",fn);
    return st.st_size;
}

////////////////////////////////////////////////////////////////////////////////
// file opening

static FILE *open_file(const char *path, const char *mode, const char *smode, off_t *sizep) {
    FILE * fp = fopen(path, mode);
    if (!fp) LH_ERROR(NULL,"Failed to open %s for %s",path,smode);

    if (sizep) {
        *sizep = lh_filesize_fp(fp);
        if (*sizep<0) { fclose(fp); return NULL; }
    }
    return fp;
}

FILE *lh_open_file_read(const char *path, off_t *sizep) {
    return open_file(path,"r","reading",sizep);
}

FILE *lh_open_file_write(const char *path) {
    return open_file(path,"w","writing",NULL);
}

FILE *lh_open_file_update(const char *path, off_t *sizep) {
    return open_file(path,"r+","updating",sizep);
}

FILE *lh_open_file_append(const char *path, off_t *sizep) {
    return open_file(path,"a","appending",sizep);
}

////////////////////////////////////////////////////////////////////////////////
// reading files

uint8_t * lh_read_fp(FILE *fp, ssize_t size, uint8_t * buffer) {
    if (!buffer)
        lh_alloc_buf(buffer, size);

    if (!buffer)                                               
        LH_ERROR(NULL, "Failed to allocate %zd bytes\n", size);

    ssize_t rbytes = fread(buffer,1,size,fp);                           
    if (rbytes!=size)
        LH_ERROR(NULL,"Failed to read %zd bytes from file (only read %zd)",size,rbytes);

    return buffer;
}
   
uint8_t * lh_read_fp_at(FILE *fp, off_t pos, ssize_t size, uint8_t * buffer) {
    if (fseeko(fp, pos, SEEK_SET)<0)                                    
        LH_ERROR(NULL, "Failed to seek to offset %jd", (intmax_t) pos);

    return lh_read_fp(fp, size, buffer);
}

uint8_t * lh_load_fp(FILE *fp, ssize_t *sizep) {
    off_t fsize = lh_filesize_fp(fp);
    // assert that the file is not larger than the largest possible buffer
    if (sizeof(off_t)>sizeof(size_t) && fsize >= SSIZE_MAX)
        LH_ERROR(NULL, "File size (%jd) exceeds limits of ssize_t (%zd bits)",
                 (intmax_t)fsize, sizeof(ssize_t)*8 );
    *sizep = (ssize_t)fsize;

    return lh_read_fp_at(fp, 0, *sizep, NULL);
}

uint8_t * lh_read_file(const char *path, ssize_t size, uint8_t * buffer) {
    FILE *fp = lh_open_file_read(path,NULL);
    if (!fp) return NULL;

    uint8_t * buf = lh_read_fp(fp, size, buffer);
    fclose(fp);
    return buf;
}

uint8_t * lh_read_file_at(const char *path, off_t pos, ssize_t size, uint8_t * buffer) {
    FILE *fp = lh_open_file_read(path,NULL);
    if (!fp) return NULL;

    uint8_t * buf = lh_read_fp_at(fp, pos, size, buffer);
    fclose(fp);
    return buf;
}

uint8_t * lh_load_file(const char *path, ssize_t *sizep) {
    off_t fsize = lh_filesize_path(path);
    // assert that the file is not larger than the largest possible buffer
    if (sizeof(off_t)>sizeof(size_t) && fsize >= SSIZE_MAX)
        LH_ERROR(NULL, "File size (%jd) exceeds limits of ssize_t (%zd bits)",
                 (intmax_t)fsize, sizeof(ssize_t)*8 );
    *sizep = (ssize_t)fsize;

    return lh_read_file_at(path, 0, *sizep, NULL);
}


int lh_write_file(const char *path, const uint8_t *data, ssize_t size) {
    FILE * fp = lh_open_file_write(path);
    if (!fp) return -1;

    int res = lh_write_fp(fp, data, size);
    fclose(fp);

    return res;
}

int lh_write_file_at(const char *path, off_t pos, const uint8_t *data, ssize_t size) {
    FILE * fp = lh_open_file_update(path, NULL);
    if (!fp) return -1;

    int res = lh_write_fp_at(fp, pos, data, size);
    fclose(fp);

    return res;
}

int lh_write_fp(FILE *fp, const uint8_t *data, ssize_t size) {
    size_t wbytes = fwrite(data, 1, size, fp);
    if (wbytes != size)
        LH_ERROR(-1, "Failed to write %zd bytes to file, only wrote %zd\n", size, wbytes);
    return 0;
}

int lh_write_fp_at(FILE *fp, off_t pos, const uint8_t *data, ssize_t size) {
    if (fseeko(fp, pos, SEEK_SET)<0)                                    
        LH_ERROR(-1, "Failed to seek to offset %jd", (intmax_t) pos);
    return lh_write_fp(fp, data, size);
}

int lh_append_file(const char *path, const uint8_t *data, ssize_t size) {
    FILE * fp = lh_open_file_append(path, NULL);
    if (!fp) return -1;

    int res = lh_write_fp(fp, data, size);
    fclose(fp);

    return res;
}

