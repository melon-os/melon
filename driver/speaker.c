#include <drv/speaker.h>

void Beep_on(ULONG32 time)
{
    while (time--)
    {
        GPIO_WRITE(GPIOA, BEEP_CTRL,!GPIO_GET_BIT(GPIOA, BEEP_CTRL));
        Sys_delay_cpu_opr(9000);
    }
}


#ifdef PLAY_MUSIC
#define TONE_1_L1   0x60
#define TONE_1_N0   0x30
#define TONE_1_H1   0x18
#define TONE_1_H2   0x0C

#define TONE_2_L1   0x55
#define TONE_2_N0   0x2B
#define TONE_2_H1   0x15

#define TONE_3_L1   0x4C
#define TONE_3_N0   0x26
#define TONE_3_H1   0x13

#define TONE_4_L1   0x48
#define TONE_4_N0   0x24
#define TONE_4_H1   0x12

#define TONE_5_L1   0x40
#define TONE_5_N0   0x20
#define TONE_5_H1   0x10

#define TONE_6_L1   0x39
#define TONE_6_N0   0x1C
#define TONE_6_H1   0x0E

#define TONE_7_L1   0x33
#define TONE_7_N0   0x19
#define TONE_7_H1   0x0D

#if 0
#define GRADE_1_4 0x30
#define GRADE_1_8 0x18
#else
#define GRADE_1_2 0x60
#define GRADE_1_4 0x30
#define GRADE_1_8 0x18
#define GRADE_1_16 0x0C
#endif

#if 0
UCHAR8 code sound[]={0xff,
    0xff,0x60,0xff,0x60,
    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_2_H1,GRADE_1_8,/*2h,1/8*/
    TONE_2_H1,GRADE_1_4 + GRADE_1_8,/*2h,1/4.*/
    TONE_1_H1,GRADE_1_4 + GRADE_1_4,/*1h,1/4-*/    
    TONE_6_N0,GRADE_1_8,/*6,1/8*/
    TONE_6_N0,GRADE_1_4,/*6,1/4*/
    TONE_3_N0,GRADE_1_4,/*3,1/4*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_2_H1,GRADE_1_8,/*2h,1/8*/
    TONE_2_H1,GRADE_1_4 + GRADE_1_8,/*2h,1/4.*/
    TONE_5_H1,GRADE_1_4 + GRADE_1_4,/*5h,1/4-*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_3_H1,GRADE_1_4 + GRADE_1_4,/*3h,1/4-*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_2_H1,GRADE_1_8,/*2h,1/8*/
    TONE_2_H1,GRADE_1_4 + GRADE_1_8,/*2h,1/4.*/
    TONE_1_H1,GRADE_1_4 + GRADE_1_4,/*1h,1/4-*/

    TONE_6_N0,GRADE_1_8,/*6,1/8*/
    TONE_6_N0,GRADE_1_8,/*6,1/8*/
    TONE_3_N0,GRADE_1_8,/*3,1/8*/
    TONE_3_N0,GRADE_1_4,/*3,1/4*/

    TONE_2_H1,GRADE_1_8,/*2h,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_3_N0,GRADE_1_4,/*3,1/4*/

    TONE_2_N0,GRADE_1_8,/*2n,1/8*/
    TONE_2_N0,GRADE_1_4 + GRADE_1_4,/*2n,1/4-*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_1_H1,GRADE_1_8,/*1h,1/8*/
    TONE_5_H1,GRADE_1_8,/*5h,1/8*/
    TONE_3_H1,GRADE_1_4 + GRADE_1_8,/*3h,1/4.*/

    TONE_1_H1,GRADE_1_8,/*1h,1/8*/
    TONE_2_H1,GRADE_1_4,/*2h,1/4*/
    TONE_5_H1,GRADE_1_4,/*5h,1/4*/
    TONE_3_H1,GRADE_1_4 + GRADE_1_4,/*3h,1/4-*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_1_H1,GRADE_1_8,/*1h,1/8*/
    TONE_5_H1,GRADE_1_8,/*5h,1/8*/
    TONE_1_H2,GRADE_1_4,/*1hh,1/4*/
    
    TONE_7_H1,GRADE_1_8,/*7h,1/8*/
    TONE_6_H1,GRADE_1_8,/*6h,1/8*/
    TONE_5_H1,GRADE_1_4,/*5h,1/4*/

    TONE_3_H1,GRADE_1_4,/*3h,1/4*/
    TONE_2_H1,GRADE_1_4 + GRADE_1_4,/*2h,1/4-*/

    TONE_6_N0,GRADE_1_4 + GRADE_1_4,/*6n,1/4-*/
    TONE_5_N0,GRADE_1_4 *6,/*5n,1/4-----*/

    0x00,
};
UCHAR8 code yaolanqu[]={0xff,
    TONE_5_L1,GRADE_1_8,
    TONE_6_L1,GRADE_1_8,
    TONE_1_N0,GRADE_1_4,
    TONE_1_N0,GRADE_1_8,
    TONE_2_N0,GRADE_1_8,
    TONE_3_N0,GRADE_1_4,    
    TONE_3_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_8,

    TONE_6_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_8,
    TONE_1_N0,GRADE_1_8,
    TONE_2_N0,GRADE_1_8,
    TONE_3_N0,GRADE_1_4,

    TONE_2_N0,GRADE_1_8,
    TONE_1_N0,GRADE_1_8,

    TONE_6_L1,GRADE_1_4,
    TONE_5_L1,GRADE_1_8,
    TONE_3_L1,GRADE_1_8,
    TONE_5_L1,GRADE_1_4,
    TONE_6_L1,GRADE_1_8,
    TONE_7_L1,GRADE_1_8,

    TONE_1_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_8+GRADE_1_16,
    TONE_2_N0,GRADE_1_16,
    TONE_2_N0,GRADE_1_4,
    TONE_5_L1,GRADE_1_8,
    TONE_6_L1,GRADE_1_8,
    
    TONE_1_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_8+GRADE_1_16,
    TONE_2_N0,GRADE_1_16,
    TONE_2_N0,GRADE_1_4,

    0xff, GRADE_1_4,
    0xff, GRADE_1_4,

    TONE_3_N0,GRADE_1_8,
    TONE_3_N0,GRADE_1_16,
    TONE_5_N0,GRADE_1_16,
    TONE_5_N0,GRADE_1_8,
    TONE_1_N0,GRADE_1_8,
    TONE_2_N0,GRADE_1_4,

    TONE_6_N0,GRADE_1_8+GRADE_1_16,
    TONE_5_N0,GRADE_1_16,
    TONE_3_N0,GRADE_1_8,
    TONE_6_N0,GRADE_1_16,
    TONE_5_N0,GRADE_1_16,
    TONE_5_N0,GRADE_1_8,
    TONE_3_N0,GRADE_1_8,
    0xff, GRADE_1_4,

    TONE_6_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_16,
    TONE_6_N0,GRADE_1_16 + GRADE_1_8,
    TONE_3_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_8,
    TONE_3_N0,GRADE_1_16,
    TONE_2_N0,GRADE_1_16,
    TONE_1_N0,GRADE_1_8,
    TONE_6_L1,GRADE_1_16,
    TONE_5_L1,GRADE_1_16,

    TONE_6_L1,GRADE_1_8,
    TONE_5_L1,GRADE_1_16,
    TONE_6_L1,GRADE_1_16*2,
    TONE_3_N0,GRADE_1_8 + GRADE_1_16,
    TONE_2_N0,GRADE_1_4*2,
    0x00,
};

#else
UCHAR8 code lianzheduoxihuan[]={0xff,
    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_2_H1,GRADE_1_4*2,/*2h,1/4.*/
    TONE_1_H1,GRADE_1_4 + GRADE_1_4,/*1h,1/4-*/    
    TONE_6_N0,GRADE_1_8 + GRADE_1_4,/*6,1/8*/
    TONE_3_N0,GRADE_1_4,/*3,1/4*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_2_H1,GRADE_1_4*2,/*2h,1/8*/
    TONE_5_H1,GRADE_1_4 + GRADE_1_4,/*5h,1/4-*/
    TONE_3_H1,GRADE_1_8*5,/*3h,1/8*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_2_H1,GRADE_1_4*2,/*2h,1/8*/
    TONE_1_H1,GRADE_1_4 + GRADE_1_4,/*1h,1/4-*/

    TONE_6_N0,GRADE_1_4,/*6,1/8*/
    TONE_3_N0,GRADE_1_8,/*3,1/8*/
    TONE_3_N0,GRADE_1_4,/*3,1/4*/

    TONE_2_H1,GRADE_1_8,/*2h,1/8*/
    TONE_3_H1,GRADE_1_8,/*3h,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_5_N0,GRADE_1_4,/*5n,1/8*/
    TONE_3_N0,GRADE_1_4,/*3,1/4*/

    TONE_2_N0,GRADE_1_8,/*2n,1/8*/
    TONE_2_N0,GRADE_1_4 + GRADE_1_4,/*2n,1/4-*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_1_H1,GRADE_1_8,/*1h,1/8*/
    TONE_5_H1,GRADE_1_8,/*5h,1/8*/
    TONE_3_H1,GRADE_1_4 + GRADE_1_8,/*3h,1/4.*/

    TONE_1_H1,GRADE_1_8,/*1h,1/8*/
    TONE_2_H1,GRADE_1_4,/*2h,1/4*/
    TONE_5_H1,GRADE_1_4,/*5h,1/4*/
    TONE_3_H1,GRADE_1_4 + GRADE_1_4,/*3h,1/4-*/

    TONE_5_N0,GRADE_1_8,/*5n,1/8*/
    TONE_6_N0,GRADE_1_8,/*6n,1/8*/
    TONE_1_H1,GRADE_1_8,/*1h,1/8*/
    TONE_5_H1,GRADE_1_8,/*5h,1/8*/
    TONE_1_H2,GRADE_1_4,/*1hh,1/4*/
    
    TONE_7_H1,GRADE_1_8,/*7h,1/8*/
    TONE_6_H1,GRADE_1_8,/*6h,1/8*/
    TONE_5_H1,GRADE_1_4,/*5h,1/4*/

    TONE_3_H1,GRADE_1_4,/*3h,1/4*/
    TONE_2_H1,GRADE_1_4 + GRADE_1_4,/*2h,1/4-*/

    TONE_6_N0,GRADE_1_4 + GRADE_1_4,/*6n,1/4-*/
    TONE_5_N0,GRADE_1_4 *6,/*5n,1/4-----*/

    0x00,
};

UCHAR8 code yaolanqu[]={0xff,
    TONE_5_L1,GRADE_1_4,
    TONE_6_L1,GRADE_1_4,
    TONE_1_N0,GRADE_1_2,
    TONE_1_N0,GRADE_1_4,
    TONE_2_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_2,    
    TONE_3_N0,GRADE_1_4,
    TONE_5_N0,GRADE_1_4,

    TONE_6_N0,GRADE_1_4,
    TONE_5_N0,GRADE_1_4,
    TONE_1_N0,GRADE_1_4,
    TONE_2_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_2,

    TONE_2_N0,GRADE_1_4,
    TONE_1_N0,GRADE_1_4,

    TONE_6_L1,GRADE_1_2,
    TONE_5_L1,GRADE_1_4,
    TONE_3_L1,GRADE_1_4,
    TONE_5_L1,GRADE_1_2,
    TONE_6_L1,GRADE_1_4,
    TONE_7_L1,GRADE_1_4,

    TONE_1_N0,GRADE_1_2,
    TONE_3_N0,GRADE_1_4+GRADE_1_8,
    TONE_2_N0,GRADE_1_8,
    TONE_2_N0,GRADE_1_2,
#if 0    
    TONE_5_L1,GRADE_1_4,
    TONE_6_L1,GRADE_1_4,    
    TONE_1_N0,GRADE_1_2,
    TONE_3_N0,GRADE_1_4+GRADE_1_8,
    TONE_2_N0,GRADE_1_8,
    TONE_2_N0,GRADE_1_2,
#endif
    0xff, GRADE_1_8,
    0xff, GRADE_1_8,

    TONE_3_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_4,
    TONE_1_N0,GRADE_1_4,
    TONE_2_N0,GRADE_1_2,

    TONE_6_N0,GRADE_1_4+GRADE_1_8,
    TONE_5_N0,GRADE_1_8,
    TONE_3_N0,GRADE_1_4,
    TONE_6_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_8,
    TONE_5_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_4,
    
    0xff, GRADE_1_4,

    TONE_6_N0,GRADE_1_4,
    TONE_5_N0,GRADE_1_8,
    TONE_6_N0,GRADE_1_8 + GRADE_1_4,
    TONE_3_N0,GRADE_1_4,
    TONE_5_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_8,
    TONE_2_N0,GRADE_1_8,
    TONE_1_N0,GRADE_1_4,
    TONE_6_L1,GRADE_1_8,
    TONE_5_L1,GRADE_1_8,

    TONE_6_L1,GRADE_1_4,
    TONE_5_L1,GRADE_1_8,
    TONE_6_L1,GRADE_1_8*2,
    TONE_3_N0,GRADE_1_4 + GRADE_1_8,
    TONE_2_N0,GRADE_1_2*2,
    0x00,
};

UCHAR8 code yaolanqu_demo[]={0xff,
    TONE_5_L1,GRADE_1_4,
    TONE_6_L1,GRADE_1_4,
    TONE_1_N0,GRADE_1_2,
    TONE_1_N0,GRADE_1_4,
    TONE_2_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_2,    
    TONE_3_N0,GRADE_1_4,
    TONE_5_N0,GRADE_1_4,

    TONE_6_N0,GRADE_1_4,
    TONE_5_N0,GRADE_1_4,
    TONE_1_N0,GRADE_1_4,
    TONE_2_N0,GRADE_1_4,
    TONE_3_N0,GRADE_1_2,
    0x00,
};


#endif
void del(UINT16 yj)
{
  UCHAR8 yj2=2;
   while(yj!=0)
     {      
           while(yj2!=0)
              {
                 yj2--;
                 }
              yj2=2;
         yj--;  
   }
}


UCHAR8 Beat_counter=0;

void Beep_demo(UCHAR8 ucMusicId)
{
    if (0 == ucMusicId)
    {
        Play_music(yaolanqu_demo);
    }
    else if (1 == ucMusicId)
    {
        Play_music(yaolanqu);
    }
    else
    {
       Play_music(lianzheduoxihuan);
    }
}
void Play_music(UCHAR8 *pucToneLst)
{
    UCHAR8 Tone, Beat; 
    UINT16 uiIdx=0;
        
    TMOD=0x01;
    IE=0x82;
    TH0=0xd8;
    TL0=0xef;
    TR0=1;

    do {  
        Beat_counter=0;
        uiIdx++; 
        Tone=pucToneLst[uiIdx]; 
        uiIdx++;
        Beat=pucToneLst[uiIdx];
        
        while(Beat_counter != Beat)
        { 
            if(Tone!=0xff)
            {
                if(Tone!=0)
                {
                    BEEP_CTRL=!BEEP_CTRL;
                    del(Tone);
                }
                else
                {
                    uiIdx=0; 
                    goto PLAY_MUSIC_EXIT;
                }
            }
            else
            {
                BEEP_CTRL=0;
                del(Beat);
            }
        }
    } while (1);

PLAY_MUSIC_EXIT:
    return;

}


time0() interrupt 1  using 1
{
    TH0=0xd8;
    TL0=0xef;
    Beat_counter++;
}
#endif/*#ifdef PLAY_MUSIC*/
