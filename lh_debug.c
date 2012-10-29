#include "lh_debug.h"
#include <stdio.h>

//FIXME: size_t -> ptrdiff_t ?
#define PARAALIGN(ptr) (unsigned char *)(((size_t)(ptr))&(-1L<<4))

void hexdump(const unsigned char * data, ssize_t length) {
    //FIXME: verify if this procedure works on 64 bit

    unsigned char * lptr;
    for(lptr=PARAALIGN(data); lptr<=PARAALIGN(data+length-1); lptr+=16) {
        printf("%8p  ",lptr);

        int lo;
        for(lo=0; lo<16; lo++) {
            if (lptr+lo < data || lptr+lo >= data+length)
                printf("   ");
            else
                printf("%02x ",lptr[lo]);
        }

        printf(" ");

        for(lo=0; lo<16; lo++) {
            if (lptr+lo < data || lptr+lo >= data+length)
                printf(" ");
            else
                printf("%c",(lptr[lo]>=' ' && lptr[lo]<0x80)?lptr[lo]:'.');
        }
        printf("\n");
    }
}
