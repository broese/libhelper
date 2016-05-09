/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>

#include "lh_dir.h"

#define LH_DIR_ALLOCGRAN 256

// object representing a single file (or general: a directory entry)
// a directory object maintains a list of these objects
typedef struct lh_dwfile {
    struct stat * st;           // stat data
    char * name;                // file name (w/o path)
} lh_dwfile;

// object representing a single directory being processed
// dirwalker object maintains a stack of these objects
typedef struct lh_dwdir {
    char          * path;       // full path of the directory
    //struct stat   * st;

    lh_dwfile     * files;      // list of files
    ssize_t         nfiles;     // number of files

    int             init;       // if 0: the structure has not been initialized
                                // with the list of files, only path is valid

    int             nextfile;   // next file to process

    struct lh_dwdir * parent;   // parent directory (NULL for base directory)
} lh_dwdir;

// dirwalker object
struct lh_dirwalk {
    ssize_t         name_max;   // max size of file name, obtained from pathconf
    ssize_t         path_max;   // max size of path, obtained from pathconf

    int             level;      // current subdirectory level
    int             flags;      // flags supplied to lh_dirwalk_create

    lh_dwdir      * current;    // currently processed directory
};

////////////////////////////////////////////////////////////////////////////////

lh_dirwalk * lh_dirwalk_create(const char * basepath, int flags) {
    // initialize the walker instance
    lh_create_obj(lh_dirwalk, dw);

    dw->name_max = pathconf(basepath, _PC_NAME_MAX);
    if (dw->name_max < 0) dw->name_max = NAME_MAX;
    dw->path_max = pathconf(basepath, _PC_PATH_MAX);
    if (dw->path_max < 0) dw->path_max = PATH_MAX;

    dw->flags = flags;
    dw->level = 0;

    // initialize the current directory
    lh_create_obj(lh_dwdir, ds);
    dw->current = ds;

    //NOTE: ds->path is NULL for the top dwdir object

    // allocate just one object in our file list - our basepath
    lh_dwfile * df = lh_arr_new(ds->files, ds->nfiles, LH_DIR_ALLOCGRAN);

#ifdef HAVE_STRNLEN
    ssize_t nlen = strnlen(basepath, dw->path_max);
#else
    ssize_t nlen = strlen(basepath);
    if (nlen > dw->path_max)
        LH_ERROR(NULL,"Path too long: %s\n",basepath);
#endif

    lh_alloc_buf(df->name, nlen+1);
    memcpy(df->name, basepath, nlen);
    do {
        df->name[nlen--] = 0;
    } while(nlen>0 && df->name[nlen] == '/');

    lh_alloc_obj(df->st);
    int res;
    if (flags & (LH_DW_BASE_SYMLINK|LH_DW_FOLLOW_SYMLINK))
        res = stat(basepath, df->st);
    else
        res = lstat(basepath, df->st);

    if (res < 0) {
        lh_dirwalk_destroy(dw);
        LH_ERROR(NULL,"Cannot stat %s\n",basepath);
    }

    ds->init = 1;

    return dw;
}

void lh_dirwalk_destroy(lh_dirwalk * dw) {
    if (!dw) return;

    while (dw->current) {
        // pop the next dwdir object from the stack
        lh_dwdir *ds = dw->current;
        dw->current = ds->parent;

        // free the dwdir object
        if (ds->path) free(ds->path);
        if (ds->files) {
            int i;
            for(i=0; i<ds->nfiles; i++) {
                if (ds->files[i].st) free(ds->files[i].st);
                if (ds->files[i].name) free(ds->files[i].name);
            }
            free(ds->files);
        }
        free(ds);
    }
    free(dw);
}

void lh_dirwalk_dump(lh_dirwalk * dw) {
    if (!dw) return;

    lh_dwdir *ds;
    for(ds=dw->current; ds; ds=ds->parent) {
        printf("Path: %s\n",ds->path);
        if (ds->files) {
            int i;
            for(i=0; i<ds->nfiles; i++)
                printf("%3d %s\n",i,ds->files[i].name);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_QSORT_R

static int lh_dirwalk_cmp(const void *pa, const void *pb, void *priv) {
    lh_dwfile *a =(lh_dwfile *)pa;
    lh_dwfile *b =(lh_dwfile *)pb;
    int flags = (intptr_t)priv;

    if (flags&LH_DW_SORT_DIRFIRST) {
        // sort directories first
        if ( S_ISDIR(a->st->st_mode) > S_ISDIR(b->st->st_mode) ) return -1;
        if ( S_ISDIR(a->st->st_mode) < S_ISDIR(b->st->st_mode) ) return 1;
    }

    // sort files alphabetically
    assert(a->name);
    assert(b->name);
    if (flags&LH_DW_SORT_IGNORECASE)
        return strcasecmp(a->name, b->name);
    else
        return strcmp(a->name, b->name);
}

#else

static int lh_dirwalk_cmp(const void *pa, const void *pb) {
    lh_dwfile *a =(lh_dwfile *)pa;
    lh_dwfile *b =(lh_dwfile *)pb;

    // sort directories first
    if ( S_ISDIR(a->st->st_mode) > S_ISDIR(b->st->st_mode) ) return -1;
    if ( S_ISDIR(a->st->st_mode) < S_ISDIR(b->st->st_mode) ) return 1;

    // sort files alphabetically
    assert(a->name);
    assert(b->name);
    return strcmp(a->name, b->name);
}

#endif

static int lh_dirwalk_readdir(lh_dirwalk *dw) {
    lh_dwdir *ds = dw->current;

    assert(ds);
    assert(ds->path);

    DIR * dir = opendir(ds->path);
    if (!dir) LH_ERROR(-1, "Failed to open directory %s",ds->path);

    // dirent pointers for the readdir_r result
    struct dirent *dep, *de;
    de = malloc(offsetof(struct dirent, d_name)+dw->name_max+1);
    lh_clear_ptr(de);

    char *path = malloc(dw->path_max+1);
    ssize_t npos = sprintf(path, "%s/", ds->path);

    while (1) {
        // read next dirent
        if (readdir_r(dir, de, &dep))
            LH_ERROR(-1, "readdir_r failed for directory %s",ds->path);
        if (!dep) break; // end of directory reached

        const char *name = de->d_name;

        // keep the '.' and '..' entries if the flag requires
        if (!(dw->flags&LH_DW_REPORT_DOTDIR))
            if (name[0]=='.' && ( name[1]==0 || (name[1]=='.' && name[2]==0) ) )
                continue;

        // allocate a new entry in the file list
        lh_dwfile * newfile = lh_arr_new(ds->files, ds->nfiles, LH_DIR_ALLOCGRAN);

        newfile->name = strdup(name);

        // stat it
        assert(npos+strlen(name) <= dw->path_max);
        sprintf(path+npos, "%s", name);

        int res;
        lh_alloc_obj(newfile->st);
        if (dw->flags&LH_DW_FOLLOW_SYMLINK)
            res = stat(path, newfile->st);
        else
            res = lstat(path, newfile->st);
    }

    // sort the files if necessary
    if (ds->files && dw->flags&LH_DW_SORT) {
#ifdef HAVE_QSORT_R
        qsort_r(ds->files, ds->nfiles, sizeof(*ds->files),
                lh_dirwalk_cmp, (void *)(intptr_t)dw->flags);
#else
        qsort(ds->files, ds->nfiles, sizeof(*ds->files), lh_dirwalk_cmp);
#endif
    }

    // the directory is now initialized
    ds->init = 1;
    closedir(dir);
    free(path);
    free(de);

    return 0;
}

int lh_dirwalk_next_p(lh_dirwalk *dw, lh_dwres_p *dr) {
    if (!dw || !dr) return LH_DW_ILLEGAL;
    lh_clear_ptr(dr);

    while(1) {
        lh_dwdir *ds = dw->current;

        if (ds->nextfile >= ds->nfiles) {
            // no more files in the list, this directory is processed to the end

            if (!ds->parent) {
                //LH_DEBUG("End of walk");
                // we are in the base list
                // all entries are processed, walk finished
                dr->type = LH_DW_FINISH;
                return LH_DW_FINISH;
            }

            // pop the dirstate from the stack and destroy it
            dw->current = ds->parent;
            dw->level--;

            if (ds->path) free(ds->path);
            if (ds->files) {
                int i;
                for(i=0; i<ds->nfiles; i++) {
                    if (ds->files[i].st) free(ds->files[i].st);
                    if (ds->files[i].name) free(ds->files[i].name);
                }
                free(ds->files);
            }
            free(ds);
            ds = dw->current;

            if (dw->flags & LH_DW_REPORT_DIREND) {
                // report a DIREND event to the user
                dr->type = LH_DW_DIREND;

                // level+1, since we already popped the dirstate from the stack
                dr->level = dw->level+1;

                lh_dwfile * df = ds->files+ds->nextfile-1;
                dr->path = ds->path;
                dr->name = df->name;
                dr->st = df->st;
                return LH_DW_DIREND;
            }
            continue;
        }

        // take next file from the current directory list
        lh_dwfile * df = &ds->files[ds->nextfile++];

        //TODO: filter

        if (S_ISDIR(df->st->st_mode)) {
            // next file in list is a directory - we will enter it

            // allocate new dirstate on top of stack
            lh_alloc_obj(dw->current);
            dw->current->parent = ds;

            lh_alloc_buf(dw->current->path,dw->path_max);
            if (ds->path) {
                if (ds->path[0]=='/' && ds->path[1]==0)
                    sprintf(dw->current->path, "/%s",df->name);
                else
                    sprintf(dw->current->path, "%s/%s",ds->path,df->name);
            }
            else {
                // there is no path for the top dwdir, since it's
                // an explicitly defined file list
                sprintf(dw->current->path, "%s",df->name);
            }

            dw->level++;
            // everything else stays zeroed - it will be initialized
            // in the next round

            if (lh_dirwalk_readdir(dw)<0)
                return LH_DW_ERROR;

            if (dw->flags & LH_DW_REPORT_DIR) {
                dr->type = LH_DW_DIR;
                // level-1 to get the level of the now-parent directory
                dr->level = dw->level-1;
                dr->path = ds->path;
                dr->name = df->name;
                dr->st   = df->st;
                return LH_DW_DIR;
            }

            continue;
        }

        if (S_ISREG(df->st->st_mode)) {
            // regular file
            if (dw->flags & LH_DW_REPORT_FILE) {
                dr->type = LH_DW_FILE;
                dr->level = dw->level;
                dr->path = ds->path;
                dr->name = df->name;
                dr->st   = df->st;
                return LH_DW_FILE;
            }
            continue;
        }

        // it's a special file (symlink, socket, pipe, device, etc.)
        if (dw->flags & LH_DW_REPORT_SPECIAL) {
            dr->type = LH_DW_SPECIAL;
            dr->level = dw->level;
            dr->path = ds->path;
            dr->name = df->name;
            dr->st   = df->st;
            return LH_DW_SPECIAL;
        }
    }

    return dr->type;
}

int lh_dirwalk_next(lh_dirwalk *dw, lh_dwres *dr) {
    assert(dw->name_max <= NAME_MAX);
    assert(dw->path_max <= PATH_MAX);

    lh_clear_ptr(dr);

    lh_dwres_p drp;
    int res = lh_dirwalk_next_p(dw, &drp);

    dr->type = drp.type;
    if (res>0) {
        dr->level = drp.level;
        if (drp.path)
            strncpy(dr->path, drp.path, sizeof(dr->path));
        else
            dr->path[0] = 0;
        strncpy(dr->name, drp.name, sizeof(dr->name));
        memcpy(&dr->st, drp.st, sizeof(dr->st));
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////

int lh_create_dir(const char *path, int mode) {
    struct stat st;
    if (!stat(path, &st)) {
        if (S_ISDIR(st.st_mode))
            return 0; // directory exists, success
        else
            return ENOTDIR; // path exists, but is not a directory
    }

    if (errno != ENOENT) {
        printf("stat on path %s failed: %d %s\n", path, errno, strerror(errno));
        return errno;
    }

    const char * rslash = rindex(path, '/');
    if (rslash) {
        // get the parent directory name
        ssize_t plen = rslash-path;
        char buf[PATH_MAX];
        memmove(buf, path, plen);
        buf[plen] = 0;

        // create parent directory
        int res = lh_create_dir(buf, mode);
        if (res) return res;
    }

    int res = mkdir(path, mode);
    if (res)
        printf("Failed to create directory %s : %s\n", path, strerror(errno));
    return res;
}









