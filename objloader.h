#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef OBJLOADER_H
#define OBJLOADER_H
#define DEBUG 1
#define MAX_LINE_SIZE 500
#define LT_VEX 0
#define LT_FACE 1
#define LT_TRI 2
typedef struct
{
    float x,y,z;
    uint32_t color;
} vertex_t;
typedef struct
{
    int p1,p2,p3;
    uint32_t color;
} face_t;
typedef struct
{
    vertex_t p1,p2,p3;
    uint32_t color;
} triangle_t;
typedef union
{
    vertex_t vet;
    face_t face;
} ld_type;
typedef struct
{
    void * next;
    ld_type data;
} node_t;

typedef struct
{
    void * next;
    triangle_t data;
} node_tri_t;

typedef struct
{
    int type;
    size_t size;
    void * head;
    uint32_t index;
    void * p_now;
} ld_list;
uint64_t OL_Read(const char * filename,uint8_t * buff,size_t buffsize);
int OL_Load(uint8_t * mod,float size,bool resize,int modsize,ld_list * vet,ld_list * face);
int OL_GetTriangle(ld_list* vex,ld_list * face,ld_list *tri);
void OL_Seek(ld_list* list,uint32_t index);

#endif // OBJLOADER_H
/**
#include "objloader.h"
#include <stdio.h>
int main()
{
    int objsize;
    ld_list vet;
    ld_list face;
    ld_list tri;
    puts("Obj loader");
    uint8_t * ldbuff=malloc(1000000000);
    if((objsize=OL_Read("test.obj",ldbuff,1000000000))==-1)
    {
        puts("read failed!");
        return -1;
    }
    OL_Load(ldbuff,100.0,1,objsize,&vet,&face);
    OL_GetTriangle(&vet,&face,&tri);
    getchar();
    free(ldbuff);
    return 0;
}
*/
