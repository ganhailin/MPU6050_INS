#include <time.h>
#include <math.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "3D.h"
#include "draw.h"
#include "touch.h"
#include "objloader.h"
#include <windows.h>
#include <inttypes.h>
#include "rs232.h"
int fps=0,fpss;
bool done = false;
bool drawdone = false;
pointR3D zero= {.rx=0,.ry=0,.rz=0};
extern char sharebuff[100];
SDL_GameController *controller = NULL;
bool haspad=false;
#define mutisize 1
char buff[1000];
char* getlinecomp(int c)
{
    int count=0;
    char * fp=buff;
    while(*(fp+count-1)!='\n')
    {
        if(RS232_PollComport(c,(unsigned char*)(fp+count),1))
            count++;
    }
    *(fp+count)=0;

    return buff;
}
int16_t buff_i[6];
float buff_f[6];
point3D p3d= {.x=0,.y=0,.z=20};
pointR3D r3d= {.rx=0,.ry=0,.rz=0};
point3D box[8];

void * pth1(void* args)///---------------------------此乃刷屏线程
{
//   char str[30];
//    char str2[30];
    while(!done)
    {
        usleep(200);
        if(done)
            break;
        clrscreen();
        clrdeepbuff();

        //sprintf(str,"FPS:%d",fpss);
        //sprintf(str2,"Angle of cube:%f",R2D(tet.getangle()));
        //sprintf(sharebuff,"Number of Finger(s):%d",getfingernum());
        //drawstring(0,0,(unsigned char*)str,0xffffff);
        //drawstring(0,10,(unsigned char*)str2,0xffffff);
        drawstring(0,20,(unsigned char*)sharebuff,0xffffff);
        // drawstring(0,30,(unsigned char*)tet.getmsg(),0xffffff);
        //Drawbox(0,0,0,10,r3d,0xfffff);
        //Drawbox(p3d.x,p3d.y,p3d.z,3,zero,0xfffff);
        Drawbox(box[0].x,box[0].y,box[0].z,3,zero,0xfffff);
        for(int i=1; i<6; i++)
            Drawbox(box[i].x,box[i].y,box[i].z,3,zero,0xfff);
        //sprintf(sharebuff,"%3.5f,%3.5f,%3.5f",r3d.rx,r3d.ry,r3d.rz);
        /* for(int i=0;i<4;i++)
         {
         sprintf(str2,"Key%d:%d",i,axis[i]);

         drawstring(0,40+i*10,(unsigned char*)str2,0xffffff);

         }*/

        updatescreen();
        fps++;
    }
    drawdone=true;
    return NULL;
}

void * pth2(void* args)///-----------------------------------这是fps计数线程
{
    while(1)
    {

        fps=0;
        for(int i=0; i<10; i++)
            usleep(100000);
        fpss=fps;
    }
    return NULL;
}

void * pth3(void* args)
{
    while(1)
    {
        usleep(10000);
        uint32_t key=getkey();
        if(key&0x01)
            movecam(0,0,1);
        if(key&0x02)
            movecam(0,0,-1);
        if(key&0x04)
            movecam(1,0,0);
        if(key&0x08)
            movecam(-1,0,0);
        if(key&0x10)
            movecam(0,-1,0);
        if(key&0x20)
            movecam(0,1,0);
    }

    return NULL;
}




pointR3D R3D_Axis(pointR3D& input,pointR3D& axis,float angle)
{
    pointR3D temp;
   /* temp.rx=(axis.rx*axis.rx+(1-axis.rx*axis.rx)*cos_)*input.rx+\
            (axis.rx*axis.ry*(1-cos_)+axis.rz*sinf(angle))*input.ry+\
            (axis.rx*axis.rz*(1-cos_)-axis.ry*sinf(angle))*input.rz;
    temp.ry=(axis.rx*axis.ry*(1-cos_)-axis.rz*sinf(angle))*input.rx+\
            (axis.ry*axis.ry+(1-axis.ry*axis.ry)*cos_)*input.ry+\
            (axis.rz*axis.ry*(1-cos_)+axis.rx*sinf(angle))*input.rz;
    temp.rz=(axis.rx*axis.rz*(1-cos_)+axis.ry*sinf(angle))*input.rx+\
            (axis.rz*axis.ry*(1-cos_)-axis.rx*sinf(angle))*input.ry+\
            (axis.rz*axis.rz+(1-axis.rz*axis.rz)*cos_)*input.rz;*/
    float sin_=sinf(angle),cos_=cosf(angle);
    temp.rx=(axis.rx*axis.rx+(1-axis.rx*axis.rx)*cos_)*input.rx+\
            (axis.rx*axis.ry*(1-cos_)-axis.rz*sin_)*input.ry+\
            (axis.rx*axis.rz*(1-cos_)+axis.ry*sin_)*input.rz;
    temp.ry=(axis.rx*axis.ry*(1-cos_)+axis.rz*sin_)*input.rx+\
            (axis.ry*axis.ry+(1-axis.ry*axis.ry)*cos_)*input.ry+\
            (axis.rz*axis.ry*(1-cos_)-axis.rx*sin_)*input.rz;
    temp.rz=(axis.rx*axis.rz*(1-cos_)-axis.ry*sin_)*input.rx+\
            (axis.rz*axis.ry*(1-cos_)+axis.rx*sin_)*input.ry+\
            (axis.rz*axis.rz+(1-axis.rz*axis.rz)*cos_)*input.rz;
    return temp;
}

void * pth4(void* args)///-----------------------------模拟一个计时器中断，100毫秒执行一次。
{
   /* pointR3D test1,test2,test3;
    test1={0,1,0};
    test2={1,0,0};
    test3=R3D_Axis(test1,test2,D2R(45));
    point3D test4={0,1,0,0};
    pointR3D test5={D2R(45),0,0};
    R3D(&test4,&test5);
    sprintf(sharebuff,"%+f   %+f   %+f",test3.rx,test3.ry,test3.rz);*/
    pointR3D axis[3];
    pointR3D axis_buff[3];
    point3D speed={0,0,0,0};
    axis[0]={1.0,0,0};
    axis[1]={0,1.0,0};
    axis[2]={0,0,1.0};
    pointR3D cali_g= {0,0,0};
    point3D Rest= {0,0,0};
    for(int i=0; i<100; i++)
    {
        while(sscanf(getlinecomp(14),"%hd/%hd/%hd,%hd/%hd/%hd",&buff_i[0],&buff_i[1],&buff_i[2],&buff_i[3],&buff_i[4],&buff_i[5])!=6);
        buff_f[3]=D2R(buff_i[3]/32768.0f);///drx
        buff_f[4]=D2R(buff_i[4]/32768.0f);///dry
        buff_f[5]=D2R(buff_i[5]/32768.0f);///drz
        buff_f[0]=buff_i[0]/32768.0f*16.0;
        buff_f[1]=buff_i[1]/32768.0f*16.0;
        buff_f[2]=buff_i[2]/32768.0f*16.0;
        Rest.x-=buff_f[1];
        Rest.y+=buff_f[2];
        Rest.z+=buff_f[0];
        cali_g.rz+=buff_f[3];
        cali_g.rx+=buff_f[4];
        cali_g.ry+=buff_f[5];
    }
    Rest.x/=100.0;
    Rest.y/=100.0;
    Rest.z/=100.0;
    box[0].x=Rest.x*15;
    box[0].y=Rest.y*15;
    box[0].z=Rest.z*15;
   /* for(int i=1; i<6; i++)
        box[i]=box[0];
    pointR3D trmp= {0,0,PI/2};
    R3D(&box[1],&trmp);
    trmp= {0,0,-PI/2};
    R3D(&box[2],&trmp);
    trmp= {PI/2,0,0};
    R3D(&box[3],&trmp);
    trmp= {-PI/2,0,0};
    R3D(&box[4],&trmp);
    trmp= {PI,0,0};
    R3D(&box[5],&trmp);*/
    cali_g.rx/=100.0;
    cali_g.rx+=0.000056;
    cali_g.ry/=100.0;
    cali_g.ry+=0.000010;
    cali_g.rz/=100.0;
    cali_g.rz-=0.000065;
    box[4].x=Rest.x;
    box[4].y=Rest.y;
    box[4].z=Rest.z;
   auto get = [](pointR3D cali_g)
    {
        if(sscanf(getlinecomp(14),"%hd/%hd/%hd,%hd/%hd/%hd",&buff_i[0],&buff_i[1],&buff_i[2],&buff_i[3],&buff_i[4],&buff_i[5])==6)
        {
            buff_f[0]=buff_i[0]/32768.0f*16.0;
            buff_f[1]=buff_i[1]/32768.0f*16.0;
            buff_f[2]=buff_i[2]/32768.0f*16.0;
            buff_f[3]=D2R(buff_i[3]/32768.0f)-cali_g.rx;///drx
            buff_f[4]=D2R(buff_i[4]/32768.0f)-cali_g.ry;///dry
            buff_f[5]=D2R(buff_i[5]/32768.0f)-cali_g.rz;///drz
        }
    };
    while(!done)
    {
            get(cali_g);
            pointR3D Rgyro={buff_f[4],-buff_f[5],-buff_f[3]};
            float Rgyroabs = sqrt(Rgyro.rx*Rgyro.rx +Rgyro.ry*Rgyro.ry +Rgyro.rz*Rgyro.rz );
            for(int i=0;i<3;i++)
            {
                axis_buff[i]=axis[i];
            }
            axis_buff[1]=R3D_Axis(axis_buff[1],axis[0],Rgyro.rx);
            axis_buff[2]=R3D_Axis(axis_buff[2],axis[0],Rgyro.rx);
            axis_buff[0]=R3D_Axis(axis_buff[0],axis[1],Rgyro.ry);
            axis_buff[2]=R3D_Axis(axis_buff[2],axis[1],Rgyro.ry);
            axis_buff[0]=R3D_Axis(axis_buff[0],axis[2],Rgyro.rz);
            axis_buff[1]=R3D_Axis(axis_buff[1],axis[2],Rgyro.rz);
            for(int i=0;i<3;i++)
            {
                axis[i]=axis_buff[i];
            }
            for(int i=0;i<3;i++)
            {
                box[i].x=axis[i].rx*15;
                box[i].y=axis[i].ry*15;
                box[i].z=axis[i].rz*15;
            }
            pointR3D acc={0,0,0};
            acc.rx+=buff_f[0]*axis[2].rx;
            acc.ry+=buff_f[0]*axis[2].ry;
            acc.rz+=buff_f[0]*axis[2].rz;
            acc.rx-=buff_f[1]*axis[0].rx;
            acc.ry-=buff_f[1]*axis[0].ry;
            acc.rz-=buff_f[1]*axis[0].rz;
            acc.rx+=buff_f[2]*axis[1].rx;
            acc.ry+=buff_f[2]*axis[1].ry;
            acc.rz+=buff_f[2]*axis[1].rz;
            box[3].x=acc.rx;
            box[3].y=acc.ry;
            box[3].z=acc.rz;
            box[3].x-=box[4].x;
            box[3].y-=box[4].y;
            box[3].z-=box[4].z;

            speed.x+=box[3].x/1000;
            speed.y+=box[3].y/1000;
            speed.z+=box[3].z/1000;
            box[5].x+=speed.x;
            box[5].y+=speed.y;
            box[5].z+=speed.z;

            sprintf(sharebuff,"%+f   %+f   %+f",box[3].x,box[3].y,box[3].z);
            /*
            /*r3d.rz=buff_f[3];
            r3d.rx=buff_f[4];
            r3d.ry=buff_f[5];
            for(int i=0; i<6; i++)
                R3D(box+i,&r3d);
            // R3D(&p3d,&r3d);
             r3d.rz+=buff_f[3];
             r3d.rx+=buff_f[4];
             r3d.ry+=buff_f[5];*/
          //  sprintf(sharebuff,"%+f   %+f   %+f",buff_f[0],buff_f[1],buff_f[2]);

    }
    return NULL;
}

void * pth5(void* args)
{
    while(!done)
    {
        usleep(5000);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if(RS232_OpenComport(14,115200))
        sprintf(sharebuff,"hhhhhh");
    pthread_t thread,thread2,thread3,thread4;//,thread5;
    setdisplay(640,480);
    initSDL();
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
        if (SDL_IsGameController(i))
        {
            controller = SDL_GameControllerOpen(i);
            if (controller)
            {
                haspad=true;
                break;

            }
        }

    setcam(setpoint(0,0,-200,0),setpointR(0,0,0));
    seteye(setpoint(0,0,-1000,0));
    pthread_create(&thread,NULL,pth1,NULL);
    pthread_create(&thread2,NULL,pth2,NULL);
    pthread_create(&thread3,NULL,pth3,NULL);
    pthread_create(&thread4,NULL,pth4,NULL);
    //pthread_create(&thread5,NULL,pth5,NULL);
    SDLloop(&drawdone,&done);
    exit(0);
}






