#include "lh_files.h"
#include "lh_buffers.h"
#include "lh_debug.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

////////////////////////////////////////////////////////////////////////////////

off_t lh_filesize(int fd) {
    struct stat st;
    if (fd<0) LH_ERROR(-1,"Incorrect file descriptor","");
    if (fstat(fd, &st)) LH_ERROR(-1,"Failed to stat file descriptor %d",fd);
    return st.st_size;
}

off_t lh_filesize_path(const char * path) {
    struct stat st;
    if (stat(path, &st)) LH_ERROR(-1,"Failed to stat file %s",path);
    return st.st_size;
}

////////////////////////////////////////////////////////////////////////////////

static int lh_open_file(const char *path, off_t *sizep, int flags, const char *smode) {
    //FIXME: respect the _LARGEFILE define?
    flags |= O_LARGEFILE;
   
    int fd;
    if (flags &= O_CREAT)
        fd = open(path, flags, LH_FILE_CREAT_MODE);
    else
        fd = open(path, flags);

    if (sizep) *sizep = -1;
    if (fd<0) LH_ERROR(-1,"Failed to open %s for %s",path,smode);
    if (sizep) *sizep = lh_filesize(fd);

    return fd;
}

int lh_open_read(const char *path, off_t *sizep) {
    return lh_open_file(path,sizep,O_RDONLY,"reading");
}

int lh_open_write(const char *path) {
    return lh_open_file(path,NULL,O_WRONLY|O_CREAT,"writing");
}

int lh_open_update(const char *path, off_t *sizep) {
    return lh_open_file(path,sizep,O_RDWR|O_CREAT,"updating");
}

int lh_open_append(const char *path, off_t *sizep) {
    return lh_open_file(path,sizep,O_WRONLY|O_CREAT|O_APPEND,"appending");
}

////////////////////////////////////////////////////////////////////////////////

static inline ssize_t determine_read_size(int fd, ssize_t length) {
    off_t offset = tell(fd);
    if (offset < 0) return -1;

    off_t filesize = lh_filesize(fd);
    if (filesize < 0) return -1;

    ssize_t rsize = filesize-offset;
    if (rsize<0) return 0;

    return length>rsize ? rsize : length;

}

ssize_t lh_read_static_at(int fd, uint8_t *buf, ssize_t length, off_t offset) {
    if (!buf || fd < 0) return LH_FILE_INVALID;

    if (offset >= 0) {
        if (lseek(fd, offset, SEEK_SET)<0)                                    
            LH_ERROR(LH_FILE_ERROR, "Failed to seek to offset %jd", (intmax_t) offset);
    }

    ssize_t rsize = determine_read_size(fd, length);
    if (rsize < 0) rsize = length; //TODO: handle unseekable files

    ssize_t rbytes = read(fd, buf, rsize);
    
    if (rbytes == 0) return LH_FILE_EOF;

    if (rbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return LH_FILE_WAIT;
        else
            return LH_FILE_ERROR;
    }

    return rbytes;
}

ssize_t lh_read_alloc_at(int fd, uint8_t **bufp, ssize_t length, off_t offset) {
    if (!bufp || fd < 0) return LH_FILE_INVALID;

    if (offset >= 0) {
        if (lseek(fd, offset, SEEK_SET)<0)                                    
            LH_ERROR(LH_FILE_ERROR, "Failed to seek to offset %jd", (intmax_t) offset);
    }

    ssize_t rsize = determine_read_size(fd, length);
    assert(rsize >= 0); //TODO: handle unseekable files

    
    

    
}






#if 0

ssize_t lh_read_at(int fd, uint8_t *buffer, ssize_t size, off_t pos) {
    if (!buffer || size <= 0 || fd < 0) return LH_FILE_INVALID;

    if (pos>=0) {
        if (lseek(fd, pos, SEEK_SET)<0)                                    
            LH_ERROR(NULL, "Failed to seek to offset %jd", (intmax_t) pos);
    }

    ssize_t rbytes = read(fd, buffer, size);

    if (rbytes == 0) return LH_FILE_EOF;

    if (rbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return LH_FILE_WAIT;
        else
            return LH_FILE_ERROR;
    }

    return rbytes;
}

ssize_t lh_load(const char *path, uint8_t **bufp) {
    off_t fsize;
    int fd = lh_open_read(path, &fsize);

    if (fd<0) return LH_FILE_ERROR;

    // assert that the file is not larger than the largest possible buffer
    if (sizeof(off_t)>sizeof(ssize_t) && fsize >= SSIZE_MAX) {
        close(fd);
        LH_ERROR(LH_FILE_INVALID, "File size (%jd) exceeds limits of ssize_t (%zd bits)",
                 (intmax_t)fsize, sizeof(ssize_t)*8 );
    }

    // special case if the file has length of 0 - allocate 1 byte,
    // since malloc will fail on size 0
    if (fsize == 0) {
        *bufp = malloc(1);
        close(fd);
        return 0;
    }

    *bufp = malloc((ssize_t)fsize);
    if (!*bufp) {
        close(fd);
        LH_ERROR(LH_FILE_ERROR, "Failed to allocate %zd bytes of memory",(ssize_t)fsize);
    }

    ssize_t rbytes = lh_read_at(fd, *bufp, (ssize_t)fsize, 0);
    
    close(fd);
    return rbytes;
}

#endif


#if 0
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


#endif




