#include <drv/led.h>

/* 动态扫描8行, 每扫描到一行时,对于的点亮该行内应该点亮的的列 */
/* 因为列选是0为点亮, 下面数组内的数据写入之前要取反 */
/* 方正形字体 */
code unsigned char aucNumberMatrix_mode0[][NUM_SCAN_TIMES] = {
    {~0x3C, ~0x24, ~0x24, ~0x24, ~0x24, ~0x24, ~0x24, ~0x3C},   /* 0 */
    {~0x3C, ~0x18, ~0x18, ~0x18, ~0x18, ~0x18, ~0x38, ~0x18},   /* 1 */
    {~0x3C, ~0x20, ~0x20, ~0x3C, ~0x3C, ~0x04, ~0x04, ~0x3C},   /* 2 */
    {~0x3C, ~0x04, ~0x04, ~0x3C, ~0x3C, ~0x04, ~0x04, ~0x3C},   /* 3 */        
    {~0x04, ~0x04, ~0x04, ~0x3C, ~0x3C, ~0x24, ~0x24, ~0x24},   /* 4 */
    {~0x3C, ~0x04, ~0x04, ~0x3C, ~0x3C, ~0x20, ~0x20, ~0x3C},   /* 5 */
    {~0x3C, ~0x24, ~0x24, ~0x3C, ~0x3C, ~0x20, ~0x20, ~0x3C},   /* 6 */
    {~0x04, ~0x04, ~0x04, ~0x04, ~0x04, ~0x04, ~0x04, ~0x3C},   /* 7 */
    {~0x3C, ~0x24, ~0x24, ~0x3C, ~0x3C, ~0x24, ~0x24, ~0x3C},   /* 8 */
    {~0x3C, ~0x04, ~0x04, ~0x3C, ~0x3C, ~0x24, ~0x24, ~0x3C},   /* 9 */
};
/* 常规字体 */
code unsigned char aucNumberMatrix_mode1[][NUM_SCAN_TIMES] = {
    {~0x00,~0x00,~0x3e,~0x41,~0x41,~0x41,~0x3e,~0x00},  /* 0 */
    {~0x00,~0x00,~0x00,~0x00,~0x21,~0x7f,~0x01,~0x00},  /* 1 */
    {~0x00,~0x00,~0x27,~0x45,~0x45,~0x45,~0x39,~0x00},  /* 2 */
    {~0x00,~0x00,~0x22,~0x49,~0x49,~0x49,~0x36,~0x00},  /* 3 */        
    {~0x00,~0x00,~0x0c,~0x14,~0x24,~0x7f,~0x04,~0x00},  /* 4 */
    {~0x00,~0x00,~0x72,~0x51,~0x51,~0x51,~0x4e,~0x00},  /* 5 */
    {~0x00,~0x00,~0x3e,~0x49,~0x49,~0x49,~0x26,~0x00},  /* 6 */
    {~0x00,~0x00,~0x40,~0x40,~0x40,~0x4f,~0x70,~0x00},  /* 7 */
    {~0x00,~0x00,~0x36,~0x49,~0x49,~0x49,~0x36,~0x00},  /* 8 */
    {~0x00,~0x00,~0x32,~0x49,~0x49,~0x49,~0x3e,~0x00},  /* 9 */
};

static UCHAR8 (*pucNumberMatrix)[NUM_SCAN_TIMES] = NULL;

void led_init(void)
{
    /* shutdown all */
    P0 = 0;
    P1 = 0xFF;
    P2 = 0xFF;
    pucNumberMatrix = aucNumberMatrix_mode0;
}

void add_led(unsigned char x, unsigned char y, unsigned color)
{
    unsigned char p0_value = P0;/* x, 1为点亮 */
    unsigned char p1_value = P1;/* y, green, 0为点亮 */
    unsigned char p2_value = P2;/* y, red, 0为点亮 */

    if (GREEN == color)
    {
        p0_value |= 1 << x;
        p1_value &= ~(1 << y);
        P0 = p0_value;
        P1 = p1_value;    
    }
    else if (RED == color)
    {
        p0_value |= 1 << x;
        p2_value &= ~(1 << y);
        P0 = p0_value;
        P2 = p2_value; 
    }
    else
    {
        p0_value |= 1 << x;
        p1_value &= ~(1 << y);
        p2_value &= ~(1 << y);
        P0 = p0_value;
        P1 = p1_value;
        P2 = p2_value;
    }
}

void light_led(unsigned char x, unsigned char y, 
                unsigned char len_x, unsigned char len_y, unsigned color)
{
    unsigned char p0_value=0;/* x, 1为点亮 */
    unsigned char p1_value=0xFF;/* y, green, 0为点亮 */
    unsigned char p2_value=0xFF;/* y, red, 0为点亮 */

    if (GREEN == color)
    {
        p0_value = len_x << x;
        p1_value &= ~(len_y << y);
        P0 = p0_value;
        P1 = p1_value;
        P2 = 0xFF;
    }
    else if (RED == color)
    {
        p0_value = len_x << x;
        p2_value &= ~(len_y << y);
        P0 = p0_value;
        P1 = 0xFF;
        P2 = p2_value; 
    }
    else
    {
        p0_value = len_x << x;
        p1_value &= ~(len_y << y);
        p2_value &= ~(len_y << y);
        P0 = p0_value;
        P1 = p1_value;
        P2 = p2_value;
    }
}

/* 动态显示矩形 ,遍历所有LED节点 */
void led_drew_rect_demo(void)
{
    char x=0, y=0;
    char color_i = 0;
    char mark_cnt = 0;
    DIRECTION_E eDir = LEFT;    
    unsigned char mark_state[8][8] = {0};

    /*三种颜色遍历*/
    led_init();     
    for (x=0; x < 8; x++)
    {
        for (y=0; y < 8; y++)
        {
            light_led(x, y, 1, 1, RED);
            task_delay(DELAY_DISP_POINT);
        }
    }
    
    led_init();       
    for (y=7; y >= 0; y--)
    {
        for (x=7; x >= 0; x--)
        {
            light_led(x, y, 1, 1, YELLOW);
            task_delay(DELAY_DISP_POINT);
        }
    }
    
    led_init();       
    for (x=7; x >= 0; x--)
    {
        for (y=7; y >= 0; y--)
        {
            light_led(x, y, 1, 1, GREEN);
            task_delay(DELAY_DISP_POINT);
        }
    }
    
    led_init();
    /*三种颜色交替变换*/
    for (y=7; y >= 0; y--)
    {
        for (x=0; x < 8; x++)
        {
            color_i++;
            light_led(x, y, 1, 1, RED + color_i%COLOR_CNT);
            task_delay(DELAY_DISP_POINT);
        }
    }


    x=0;
    y=0;
    light_led(x, y, 1, 1, color_i);
    task_delay(DELAY_DISP_POINT*2);
    mark_state[x][y] = 1;
    mark_cnt++;
    /* 回字形遍历 */
    while (64 != mark_cnt)
    {    
        task_delay(DELAY_TIMER*2);
        if ((LEFT == eDir) && (y+1 < 8) && (0 == mark_state[x][y+1]))
        {
            y++;
            light_led(x, y, 1, 1, mark_cnt%COLOR_CNT);            
            mark_state[x][y] = 1;
            mark_cnt++;
            continue;
        }
        else
        {
            eDir = UP;
        }
        
        if ((UP == eDir) && (x+1 < 8) && (0 == mark_state[x+1][y]))
        {
            x++;
            light_led(x, y, 1, 1, mark_cnt%COLOR_CNT);
            mark_state[x][y] = 1;
            mark_cnt++;
            continue;
        }
        else
        {
            eDir = RIGHT;
        }
        
        if ((RIGHT== eDir) && (y-1 >= 0) && (0 == mark_state[x][y-1]))
        {
            y--;
            light_led(x, y, 1, 1, mark_cnt%COLOR_CNT);
            mark_state[x][y] = 1;
            mark_cnt++;
            continue;
        }
        else
        {
            eDir = DOWN;
        }
        
        if ((DOWN == eDir) && (x-1 >= 0) && (0 == mark_state[x-1][y]))
        {
            x--;
            light_led(x, y, 1, 1, mark_cnt%COLOR_CNT);
            mark_state[x][y] = 1;
            mark_cnt++;
            continue;
        }
        else
        {
            eDir = LEFT;
        }        
    }

    
    /*矩形增长  4个角同时 三种颜色遍历*/
    for (color_i = 0; color_i < COLOR_CNT; color_i++)
    {
        led_init();
        for (x=4; x >= 0; x--)/* 从中间向四个角扩展 */
        {                
            add_led(x, x, RED + color_i);
            add_led(7-x, 7-x, RED + color_i);
            task_delay(DELAY_TIMER*20);
        }        
        led_init();   
        for (x=0; x < 8/2; x++)/* 从四个角收缩到中间 */
        {                
            add_led(x, x, RED + color_i);
            add_led(7-x, 7-x, RED + color_i);
            task_delay(DELAY_TIMER*20);
        }        
    }
    
    /*矩形增长5, 中间向四个角扩散 混色*/    
    led_init();
    task_delay(10);
    color_i = RED;
    for (x=4; x >= 0; x--, color_i++)
    {                
        add_led(x, x, RED + color_i%COLOR_CNT);
        add_led(7-x, 7-x, RED + color_i%COLOR_CNT);
        if (4 != x)
        {
            task_delay(DELAY_TIMER*50);
        }
        
    }

    /*矩形增长4, 4个角同时向中间靠拢 混色 */    
    led_init();
    color_i = RED;
    for (x=0; x < 8/2; x++, color_i++)
    {                
        add_led(x, x, RED + color_i%COLOR_CNT);
        add_led(7-x, 7-x, RED + color_i%COLOR_CNT);
        task_delay(DELAY_TIMER*50);
    }

    
    task_delay(100);
    /*矩形增长1, 从某一个角落开始, 三种颜色遍历*/    
    led_init();
    task_delay(100);        
    for (x=0; x < 8; x++)
    {                
        add_led(x, x, RED);
        task_delay(DELAY_TIMER*6);
    }
    
    led_init();
    task_delay(100);
    for (x=0; x < 8; x++)
    {                
        add_led(x, 7-x, YELLOW);
        task_delay(DELAY_TIMER*6);
    }
    
    led_init();
    task_delay(100);               
    for (x=0; x < 8; x++)
    {                
        add_led(7-x, x, GREEN);
        task_delay(DELAY_TIMER*6);
    }    

    /*矩形增长2, 从某一个角落开始,混色 */        
    led_init();
    task_delay(100);                
    for (x=0; x < 8; x++)
    {                
        add_led(7-x, 7-x, RED + color_i++%COLOR_CNT);
        task_delay(DELAY_TIMER*6);
    }
    
    task_delay(1000);    
}


UCHAR8 led_change_font_mode(FONE_MODE_E ucFontMode)
{
    switch (ucFontMode)
    {
        case FONT_0:
            pucNumberMatrix = aucNumberMatrix_mode0;
            break;
        case FONT_1:
            pucNumberMatrix = aucNumberMatrix_mode1;
            break;
        default:
            return ERR_CODE_PARA;
    }
    return ERR_CODE_OK;
}
void led_display_num(UCHAR8 num, COLOR_E color, unsigned long display_time)
{
    char y=0;

    if (num > 9)
    {
        return;
    }

    display_time = display_time >> 3;
    
    switch (color)
    {
        case RED:
            while (display_time--)
            {
                for (y = 0; y < 8; y++)
                {
                    P0 = 1 << y;
                    P2 = pucNumberMatrix[num][y];
                    task_delay(1);
                    P2 = 0xFF;
                }
            }
            
            break;
        case YELLOW:
            while (display_time--)
            {
                for (y = 0; y < 8; y++)
                {
                    P0 = 1 << y;
                    P1 = P2 = pucNumberMatrix[num][y];
                    task_delay(1);
                    P1 = P2 = 0xFF;
                }
            }
            
            break;
        case GREEN:
            while (display_time--)
            {
                for (y = 0; y < 8; y++)
                {
                    P0 = 1 << y;
                    P1 = pucNumberMatrix[num][y];
                    task_delay(1);
                    P1 = 0xFF;
                }
            }
            break;
        default:

            break;
    }
}

void led_display_num_demo(void)
{
    char x=0, y=0;
    char idx = 0;
    char color_i = 0;
    unsigned long display_time = 0;    

#if 0
    unsigned char test_data[] = {
        0x4, 0x4, 0x4, 0x4, 0x3C, 0x24,0x24,0x24
    };

       for (y = 0; y < 8; y++)
       {
           P0 = 1<<y;
           P2 = ~test_data[y];
           task_delay(1);
           P2 = 0xFF;
       }
#else
    for (color_i = RED; color_i < COLOR_CNT; color_i++)
    {
        /* 循环显示 0~9 */
        for (idx = 0; idx < GET_ARRAY_CNT(aucNumberMatrix_mode0); idx++)
        {
            led_init();
            display_time = NUM_DISP_TIME/((1+color_i)*25 + idx*2 + idx);
            while (display_time--)
            {
                if (RED == color_i)
                {
                    for (y = 0; y < 8; y++)
                    {
                        P0 = 1 << y;
                        P2 = pucNumberMatrix[idx][y];
                        task_delay(1);
                        P2 = 0xFF;
                    }
                }
                else if (YELLOW == color_i)
                {
                    for (y = 0; y < 8; y++)
                    {
                        P0 = 1 << y;
                        P1 = P2 = pucNumberMatrix[idx][y];
                        task_delay(1);
                        P1 = P2 = 0xFF;

                    }
                }
                else
                {
                    for (y = 0; y < 8; y++)
                    {
                        P0 = 1 << y;
                        P1 = pucNumberMatrix[idx][y];
                        task_delay(1);
                        P1 = 0xFF;
                    }
                }
            }
            //task_delay(NUM_DISP_GAP_MS);
        }
    }
#endif
}

void led_disp_num_rand_color_demo(void)
{
    char x=0, y=0;
    char idx = 0, idx2 = 0;
    char colorArray[NUM_SCAN_TIMES] = {0}; /* 扫描一次的颜色列表 */
    unsigned long display_time = 0;

    srand(0);
    /* 循环显示 0~9 */
    for (idx = 0; idx < GET_ARRAY_CNT(aucNumberMatrix_mode0); idx++)
    {

        for (idx2 = 0; idx2 < NUM_SCAN_TIMES; idx2++)
        {        
            colorArray[idx2] = rand()%COLOR_CNT;
        }
    
        led_init();

        display_time = (NUM_DISP_TIME-400)/(idx*12 + idx + 100);
        while (display_time--)
        {   
            for (y = 0; y < NUM_SCAN_TIMES; y++)
            {
                P0 = 1 << y;
                if (RED == colorArray[y])
                {
                    P2 = pucNumberMatrix[idx][y];
                    task_delay(1);
                    P2 = 0xFF;
                }
                else if (GREEN== colorArray[y])
                {
                    P1 = pucNumberMatrix[idx][y];
                    task_delay(1);
                    P1 = 0xFF;
                }
                else
                {
                    P1 = P2 = pucNumberMatrix[idx][y];
                    task_delay(1);
                    P1 = P2  = 0xFF;
                }
            }
            
            task_delay(NUM_DISP_GAP_MS);
        }
    }
}
