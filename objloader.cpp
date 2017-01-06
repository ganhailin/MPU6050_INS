#include "objloader.h"
#include <stdio.h>
#include <memory.h>
#include <float.h>
const int SIZES[3]= {sizeof(vertex_t),sizeof(face_t),sizeof(triangle_t)};

uint64_t OL_Read(const char * filename,uint8_t * buff,size_t buffsize)
{
    FILE * fp=0;
    uint64_t count=0;
    int i;
    fp=fopen(filename,"r");
    if(fp)
    {
        if(DEBUG)
            puts("Open succeed");
        while(((i=fgetc(fp))!=EOF)&&(count<=buffsize))
        {
            *(buff++)=i;
            count++;
        }
        if(i==-1)
            return count;
        else
            return -1;
    }
    else return -1;
}

void OL_Seek(ld_list* list,uint32_t index)
{
    if(index<list->index)
    {
        list->p_now=list->head;
        list->index=1;
    }
    while(list->index!=index)
    {
        list->index++;
        if(list->type!=LT_TRI)
            list->p_now=(void*)(((node_t*)(list->p_now))->next);
        else
            list->p_now=(void*)(((node_tri_t*)(list->p_now))->next);
    }
}

int OL_AddNode(ld_list* list,void *node)
{
    void * pt=NULL;
    if(list->size==0)
    {
        if(list->type!=LT_TRI)
            pt=malloc(sizeof(node_t));
        else
            pt=malloc(sizeof(node_tri_t));
        if(pt==NULL)return -1;
        list->head=list->p_now=pt;
        list->size++;
        list->index++;
        if(list->type!=LT_TRI)
            memcpy(&((node_t*)(list->p_now))->data,node,SIZES[list->type]);
        else
            memcpy(&((node_tri_t*)(list->p_now))->data,node,SIZES[list->type]);

        return 0;
    }

    if(list->index!=list->size)
        OL_Seek(list,list->size);
    if(list->type!=LT_TRI)
    {
        pt=malloc(sizeof(node_t));
        if(pt==NULL)return -1;

        ((node_t*)(list->p_now))->next=pt;
        list->size++;
        list->p_now=((node_t*)list->p_now)->next;
        list->index++;
        memcpy(&((node_t*)(list->p_now))->data,node,SIZES[list->type]);
    }
    else
    {
        pt=malloc(sizeof(node_tri_t));
        if(pt==NULL)return -1;

        ((node_tri_t*)(list->p_now))->next=pt;
        list->size++;
        list->p_now=((node_tri_t*)list->p_now)->next;
        list->index++;
        memcpy(&((node_tri_t*)(list->p_now))->data,node,SIZES[list->type]);

    }
    return 0;
}
int OL_Load(uint8_t * mod,float size,bool resize,int modsize,ld_list * vet,ld_list * face)
{

    uint8_t buff[MAX_LINE_SIZE];
    int index=0;
    int line_index;
    int temp2;
    ld_type temp;
    vet->type=LT_VEX;
    vet->size=0;
    vet->index=0;
    face->type=LT_FACE;
    face->size=0;
    face->index=0;
    while(index<modsize)
    {
        line_index=0;
        while(*(mod+index)!='\r'&&*(mod+index)!='\n'&&index<modsize)
        {
            *(buff+line_index)=*(mod+index);
            index++;
            line_index++;
        }
        index++;
        *(buff+line_index)=0;
        if(DEBUG)puts((char*)buff);
        ///----------------读取每一行（并且支持各种换行符号QvQ）
        switch(*buff)
        {
        case 'v':
            if(*(buff+1)!=' ')break;
            if(DEBUG)puts("get vertex");
            sscanf((char*)buff+1,"%f\n%f\n%f",&temp.vet.x,&temp.vet.y,&temp.vet.z);
            if(DEBUG)printf("%f\n%f\n%f\n",temp.vet.x,temp.vet.y,temp.vet.z);
            OL_AddNode(vet,&temp);
            if(DEBUG)printf("%f\n",((node_t*)(vet->p_now))->data.vet.x);
            break;
        case 'o':
            if(DEBUG)puts("get object");
            break;
        case 'f':
            if(DEBUG)puts("get face");
            if(strchr((char*)buff+1,'/'))
            {
                int num=sscanf((char*)buff+1,"%d/%*d\n%d/%*d\n%d/%*d\n%d/%*d",&temp.face.p1,&temp.face.p2,&temp.face.p3,&temp2);
                if(num==4)
                {
                    OL_AddNode(face,&temp);
                    temp.face.p2=temp2;
                    OL_AddNode(face,&temp);
                }
                if(num==3)
                    OL_AddNode(face,&temp);
                if(num==1)
                {
                    num=sscanf((char*)buff+1,"%d/%*d/%*d\n%d/%*d/%*d\n%d/%*d/%*d\n%d/%*d/%*d",&temp.face.p1,&temp.face.p2,&temp.face.p3,&temp2);
                    if(num==4)
                    {
                        OL_AddNode(face,&temp);
                        temp.face.p2=temp2;
                        OL_AddNode(face,&temp);
                    }
                    if(num==3)
                        OL_AddNode(face,&temp);

                }
            }
            else
            {
                if(sscanf((char*)buff+1,"%d\n%d\n%d\n%d",&temp.face.p1,&temp.face.p2,&temp.face.p3,&temp2)==4)
                {
                    OL_AddNode(face,&temp);
                    temp.face.p2=temp2;
                    OL_AddNode(face,&temp);
                }
                else
                    OL_AddNode(face,&temp);
            }
            if(DEBUG)printf("%d\n%d\n%d\n",temp.face.p1,temp.face.p2,temp.face.p3);

            break;
        default:
            break;
        }
    }
    if(resize)
    {
        size/=2;
        float maxs[6]= {FLT_MIN,FLT_MAX,FLT_MIN,FLT_MAX,FLT_MIN,FLT_MAX};
        for(uint32_t i=1; i<=vet->size; i++)
        {
            OL_Seek(vet,i);
            if(((node_t*)vet->p_now)->data.vet.x>maxs[0])
                maxs[0]=((node_t*)vet->p_now)->data.vet.x;
            if(((node_t*)vet->p_now)->data.vet.x<maxs[1])
                maxs[1]=((node_t*)vet->p_now)->data.vet.x;
            if(((node_t*)vet->p_now)->data.vet.x>maxs[2])
                maxs[2]=((node_t*)vet->p_now)->data.vet.x;
            if(((node_t*)vet->p_now)->data.vet.x<maxs[3])
                maxs[3]=((node_t*)vet->p_now)->data.vet.x;
            if(((node_t*)vet->p_now)->data.vet.x>maxs[4])
                maxs[4]=((node_t*)vet->p_now)->data.vet.x;
            if(((node_t*)vet->p_now)->data.vet.x<maxs[5])
                maxs[5]=((node_t*)vet->p_now)->data.vet.x;
        }
        float mid[3];
        mid[0]=(maxs[0]+maxs[1])/2;
        mid[1]=(maxs[2]+maxs[3])/2;
        mid[2]=(maxs[4]+maxs[5])/2;
        maxs[0]=size/(maxs[0]-maxs[1]);
        maxs[1]=size/(maxs[2]-maxs[3]);
        maxs[2]=size/(maxs[4]-maxs[5]);
        for(uint32_t i=1; i<=vet->size; i++)
        {
            OL_Seek(vet,i);
            ((node_t*)vet->p_now)->data.vet.x-=mid[0];
            ((node_t*)vet->p_now)->data.vet.y-=mid[1];
            ((node_t*)vet->p_now)->data.vet.z-=mid[2];
            ((node_t*)vet->p_now)->data.vet.x*=maxs[0];
            ((node_t*)vet->p_now)->data.vet.y*=maxs[1];
            ((node_t*)vet->p_now)->data.vet.z*=maxs[2];
        }

    }
    return 0;
}
int OL_GetTriangle(ld_list* vex,ld_list * face,ld_list *tri)
{
    triangle_t temp;
    uint16_t i;
    tri->type=LT_TRI;
    tri->size=0;
    tri->index=0;
    for(i=1; i<=face->size; i++)
    {
        OL_Seek(face,i);
        OL_Seek(vex,((node_t*)face->p_now)->data.face.p1);
        temp.p1.x=((node_t*)vex->p_now)->data.vet.x;
        temp.p1.y=((node_t*)vex->p_now)->data.vet.y;
        temp.p1.z=((node_t*)vex->p_now)->data.vet.z;
        OL_Seek(vex,((node_t*)face->p_now)->data.face.p2);
        temp.p2.x=((node_t*)vex->p_now)->data.vet.x;
        temp.p2.y=((node_t*)vex->p_now)->data.vet.y;
        temp.p2.z=((node_t*)vex->p_now)->data.vet.z;
        OL_Seek(vex,((node_t*)face->p_now)->data.face.p3);
        temp.p3.x=((node_t*)vex->p_now)->data.vet.x;
        temp.p3.y=((node_t*)vex->p_now)->data.vet.y;
        temp.p3.z=((node_t*)vex->p_now)->data.vet.z;
        if(DEBUG)printf("%f %f %f\n",temp.p1.x,temp.p1.y,temp.p1.z);
        OL_AddNode(tri,&temp);
    }
    return 0;
}
