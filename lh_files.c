#include "lh_files.h"
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

