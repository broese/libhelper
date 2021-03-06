#include "lh_debug.h"
#include <stdio.h>
#include <stdint.h>

#define PARAALIGN(ptr) (const uint8_t *)(((size_t)(ptr))&(-1L<<4))

void hexdump(const void * data, ssize_t length) {
    const uint8_t * ptr = (const uint8_t *)data;
    const uint8_t * lptr;
    for(lptr=PARAALIGN(data); lptr<=PARAALIGN(data+length-1); lptr+=16) {
        printf("%8p  ",lptr);

        int lo;
        for(lo=0; lo<16; lo++) {
            if (lptr+lo < (const uint8_t *)data || lptr+lo >= (const uint8_t *)data+length)
                printf("   ");
            else
                printf("%02x ",lptr[lo]);
        }

        printf(" ");

        for(lo=0; lo<16; lo++) {
            if (lptr+lo < (const uint8_t *)data || lptr+lo >= (const uint8_t *)data+length)
                printf(" ");
            else
                printf("%c",(lptr[lo]>=' ' && lptr[lo]<0x80)?lptr[lo]:'.');
        }
        printf("\n");
    }
}

void hexprint(const void * data, ssize_t length) {
    int i;
    const uint8_t *ptr = (const uint8_t *)data;
    for(i=0; i<length; i++)
        printf("%02x%s",*ptr++,(length==i+1)?"\n":" ");
}

#define HEXDIGIT(h) \
    (h>='0' && h<='9') ? ( h-'0') :                 \
        ( (h>='a' && h<='f') ? ( h-'a'+10) :        \
          (h>='A' && h<='F') ? ( h-'A'+10) : -1)
                                                     
ssize_t hex_import(const char *hex, uint8_t *bin, ssize_t maxlen) {
    ssize_t i;
    for(i=0; i<maxlen; i++) {
        char h = HEXDIGIT(*hex); hex++;
        if (h<0) break;
        char l = HEXDIGIT(*hex); hex++;
        if (l<0) break;
        bin[i]=(h<<4)|l;
    }
    return i;
}
