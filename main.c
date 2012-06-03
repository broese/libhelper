#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lh_buffers.h"

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float tx, ty;
} vertex;

typedef struct {
    int a,b,c;
} face;

typedef struct {
    vertex * v;
    int      nv;
    face   * f;
    int      nf;
} model;

#define ALLOC_GRAN 4
#define BUF_GRAN   64

int main(int ac, char **av) {
#if 0
    ALLOC(model,m);
    ARRAY_ALLOCG(vertex,m->v,m->nv,100,ALLOC_GRAN);
    ARRAY_ALLOCG(face,m->f,m->nf,100,ALLOC_GRAN);
    ARRAY_ADDG(vertex,m->v,m->nv,20,ALLOC_GRAN);
#endif

#if 0
    int i;
    for(i=0; i<20; i++)
        printf("%3d %3d %3d\n",i,GRANREST(i,ALLOC_GRAN),GRANSIZE(i,ALLOC_GRAN));
#endif

#if 0
    int *a, n;
    //ARRAY_ALLOCG(int,a,n,0,ALLOC_GRAN);

    while(1) {
        int input;
        printf("Enter next number: ");
        if (scanf("%u",&input)==1) {
            ARRAY_ADDG(int,a,n,1,ALLOC_GRAN);
            a[n-1] = input;
        }
        else {
            printf("Input finished, the resulting array contains %d elements\n",n);
            int i;
            for(i=0; i<n; i++)
                printf(" %3d : %d\n",i,a[i]);
            break;
        }
    }
#endif

#if 0

    unsigned char *str, buffer[4096];
    int n;

    BUFFER_ALLOCG(str,n,1,BUF_GRAN);
    str[0] = 0;

    while(1) {
        printf("Enter next string: ");
        fgets(buffer,sizeof(buffer),stdin);
        if (!strncmp(buffer,"END",3)) break;

        int pos = n-1;
        int len = strlen(buffer);
        BUFFER_ADDG(str,n,len,BUF_GRAN);
        memcpy(str+pos,buffer,len);
        str[n-1]=0;
    }

    printf("Total length: %d\n%s\n",n-1,str);
#endif

    return 0;
}

