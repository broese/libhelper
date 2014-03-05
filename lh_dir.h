#pragma once

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#include "lh_buffers.h"
#include "lh_debug.h"

/* Note: FTS-based implementation was dropped due to lack of 
 * large file (_FILE_OFFSET_BITS=64) support.
 * FTW-based implementation is considered, but it would have
 * major limitations - no sorting, no dirend, etc.
 */

#define LH_DW_REPORT_FILE      (1<<0)  /* report files */
#define LH_DW_REPORT_DIR       (1<<1)  /* report directories when entering */
#define LH_DW_REPORT_DIREND    (1<<2)  /* report when exiting a directory */
#define LH_DW_REPORT_DOTDIR    (1<<3)  /* report . and .. directories */
#define LH_DW_REPORT_SPECIAL   (1<<4)  /* report all other files (symlinks, pipes, etc) */
#define LH_DW_FOLLOW_SYMLINK   (1<<5)  /* when disabled, symlinks are not resolved but reported as symlinks */
#define LH_DW_BASE_SYMLINK     (1<<6)  /* allow base directory to be a symlink */
#define LH_DW_KEEP_FS          (1<<7)  /* do not change file system - not implemented */
#define LH_DW_SORT             (1<<8)  /* sort files alphabetically */
#define LH_DW_SORT_DIRFIRST    (1<<9)  /* sort directories first */
#define LH_DW_SORT_IGNORECASE  (1<<10) /* ignore case when sorting */

#define LH_DW_DEFAULTS                                          \
    (LH_DW_REPORT_FILE|LH_DW_REPORT_DIR|LH_DW_REPORT_DIREND|    \
     LH_DW_SORT|LH_DW_SORT_DIRFIRST)

#define LH_DW_ERROR           -2    /* OS reports an error (unreadable etc.) */
#define LH_DW_ILLEGAL         -1    /* incorrect arguments */
#define LH_DW_FINISH           0    /* all items browsed, end of dir walk */
#define LH_DW_FILE             1    /* a regular file */
#define LH_DW_DIR              2    /* a directory */
#define LH_DW_DIREND           3    /* leaving a directory */
#define LH_DW_SPECIAL          4    /* a non-regular file */

////////////////////////////////////////////////////////////////////////////////

typedef struct lh_dirwalk lh_dirwalk;

typedef struct {
    const char * path;
    const char * name;
    struct stat * st;
    int level;
    int type;
} lh_dwres_p;

typedef struct {
    char path[PATH_MAX+1];
    char name[NAME_MAX+1];
    struct stat st;
    int level;
    int type;
} lh_dwres;

////////////////////////////////////////////////////////////////////////////////

lh_dirwalk * lh_dirwalk_create(const char * basepath, int flags);
void lh_dirwalk_destroy(lh_dirwalk * dw);
void lh_dirwalk_dump(lh_dirwalk * dw);

int lh_dirwalk_next_p(lh_dirwalk *dw, lh_dwres_p *dr);
int lh_dirwalk_next(lh_dirwalk *dw, lh_dwres *dr);

//TODO: init with a list of files
//TODO: skip to next directory
//TODO: ignore hidden files flag
//TODO/CLARIFY: handle files without read access
//TODO: external filter

void lh_dirwalk_test(const char * basedir);
