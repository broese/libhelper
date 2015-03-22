#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lh_buffers.h"

#ifndef LH_BUFPRINTF_GRAN
#define LH_BUFPRINTF_GRAN 256
#endif

static inline ssize_t lh_bufprintf_g(uint8_t **bufp, ssize_t *lenp, int gran, const char *fmt,...) {
    // remaining space in the allocated buffer
    ssize_t remsize = lh_align(*lenp,gran)-*lenp;

    // allocate buffer if it's not allocated at all
    // so the snprintf won't bail out due to a NULL pointer
    if (!*bufp || *lenp==0) {
        lh_resize(*bufp,gran);
        *lenp = 0;
        remsize = gran;
    }
    else {
        // otherwise ensure granularity
        lh_setgran(*bufp, *lenp, gran);
    }

    uint8_t *pos = *bufp+*lenp;

    va_list args;

    va_start(args,fmt);
    int plen = vsnprintf((char *)pos, remsize, fmt, args);
    va_end(args);

    if (plen < 0) return plen; // error occured

    if (plen >= remsize) {
        // The space was not sufficient for the written string
        // (including terminator). Resize and repeat
        lh_resize(*bufp, lh_align(plen+1+*lenp, gran));
        pos = *bufp+*lenp;

        va_start(args,fmt);
        plen = vsnprintf((char *)pos, plen+1, fmt, args);
        va_end(args);

        if (plen < 0) return plen; // error occured
    }

    *lenp += plen;
    return plen;
}

#define _lh_bufprintf(ptr,cnt,fmt,...)                                  \
    lh_bufprintf_g(&ptr,&cnt,LH_BUFPRINTF_GRAN,fmt,##__VA_ARGS__)

#define lh_bufprintf(...) _lh_bufprintf(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define bprintf                         lh_bufprintf

#endif
