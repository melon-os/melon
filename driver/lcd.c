/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : LCD.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : LCD source file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#include <drv/lcd.h>

extern code UCHAR8 Font8x16[][16];
static code USHORT16 default_color[]= {
/* red     green   blue   yellow black   brown   pink   white  */
   0xf800,0x07e0,0x001f,0xffe0,0x0000,0x07ff,0xf81f,0xffff,
};

static volatile USHORT16 g_print_CurrX = 0;
static volatile USHORT16 g_print_CurrY = 0;
static UCHAR8 g_lcd_printf_buf[LCD_PRINT_STR_MAX_LEN] = {0};
static PRT_COLOR_E g_lcd_bkcolor = black;
static PRT_COLOR_E g_lcd_forcolor = yellow;

#define LCD_ACCESS_GAP  5

/**********************************************************
 写命令与数据子函数
**********************************************************/
void LCD_send_cmd(USHORT16 value)
{
        GPIO_RESET_BIT(GPIOB, LCD_CS);
        //GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_COMMAND);
        GPIO_RESET_BIT(GPIOA, LCD_RS);/*TYPE_LCD_COMMAND*/
        GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
        LCD_DATA    = (UCHAR8)(value>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
        GPIOB->BSRR = value & 0xff00;
        GPIOB->BRR  = (~value) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
        GPIO_SET_BIT(GPIOB, LCD_WR);

        CPU_DELAY(LCD_ACCESS_GAP);

        GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
        LCD_DATA    = (UCHAR8)value;
#elif defined(_CPU_ARM_STM32F103C8_)
        GPIOB->BSRR = (value)<<8 & 0xff00;
        GPIOB->BRR  = ((~value)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
        GPIO_SET_BIT(GPIOB, LCD_WR);
        CPU_DELAY(LCD_ACCESS_GAP);
        GPIO_SET_BIT(GPIOB, LCD_CS);

}

void LCD_send_data_batch(USHORT16 value, ULONG32 ulCnt)
{
    while(ulCnt--)
    {
        GPIO_RESET_BIT(GPIOB, LCD_CS);
        //GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_DATA);
        GPIO_SET_BIT(GPIOA, LCD_RS);/*TYPE_LCD_DATA*/
        GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
        LCD_DATA    = (UCHAR8)(value>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
        GPIOB->BSRR = value & 0xff00;
        GPIOB->BRR  = (~value) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
        GPIO_SET_BIT(GPIOB, LCD_WR);

        CPU_DELAY(LCD_ACCESS_GAP);

        GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
        LCD_DATA    = (UCHAR8)value;
#elif defined(_CPU_ARM_STM32F103C8_)
        GPIOB->BSRR = (value)<<8 & 0xff00;
        GPIOB->BRR  = ((~value)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
        GPIO_SET_BIT(GPIOB, LCD_WR);
        CPU_DELAY(LCD_ACCESS_GAP);
        GPIO_SET_BIT(GPIOB, LCD_CS);

        CPU_DELAY(LCD_ACCESS_GAP);
    }
}

void LCD_send_data(USHORT16 value)    // color data
{
        GPIO_RESET_BIT(GPIOB, LCD_CS);
        GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_DATA);
        GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
        LCD_DATA    = (UCHAR8)(value>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
        GPIOB->BSRR = value & 0xff00;
        GPIOB->BRR  = (~value) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
        GPIO_SET_BIT(GPIOB, LCD_WR);

        CPU_DELAY(LCD_ACCESS_GAP);

        GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
        LCD_DATA    = (UCHAR8)value;
#elif defined(_CPU_ARM_STM32F103C8_)
        GPIOB->BSRR = (value)<<8 & 0xff00;
        GPIOB->BRR  = ((~value)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
        GPIO_SET_BIT(GPIOB, LCD_WR);
        CPU_DELAY(LCD_ACCESS_GAP);
        GPIO_SET_BIT(GPIOB, LCD_CS);
}


/*********************************************************
 写寄存器子函数
**********************************************************/
void Reg_Write(USHORT16 reg, USHORT16 value)
{
    GPIO_RESET_BIT(GPIOB, LCD_CS);
    //GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_COMMAND);
    GPIO_RESET_BIT(GPIOA, LCD_RS);/*TYPE_LCD_COMMAND*/
    GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
    LCD_DATA    = (UCHAR8)(reg>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
    GPIOB->BSRR = reg & 0xff00;
    GPIOB->BRR  = (~reg) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
    GPIO_SET_BIT(GPIOB, LCD_WR);

    CPU_DELAY(LCD_ACCESS_GAP);

    GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
    LCD_DATA    = (UCHAR8)reg;
#elif defined(_CPU_ARM_STM32F103C8_)
    GPIOB->BSRR = (reg)<<8 & 0xff00;
    GPIOB->BRR  = ((~reg)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
    GPIO_SET_BIT(GPIOB, LCD_WR);
    CPU_DELAY(LCD_ACCESS_GAP);
    GPIO_SET_BIT(GPIOB, LCD_CS);

    /*gap*/
    CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(LCD_ACCESS_GAP);

    GPIO_RESET_BIT(GPIOB, LCD_CS);
    //GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_DATA);
    GPIO_SET_BIT(GPIOA, LCD_RS);/*TYPE_LCD_DATA*/
    GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
    LCD_DATA    = (UCHAR8)(reg>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
    GPIOB->BSRR = value & 0xff00;
    GPIOB->BRR  = (~value) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
    GPIO_SET_BIT(GPIOB, LCD_WR);

    CPU_DELAY(LCD_ACCESS_GAP);

    GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
    LCD_DATA    = (UCHAR8)reg;
#elif defined(_CPU_ARM_STM32F103C8_)
    GPIOB->BSRR = (value)<<8 & 0xff00;
    GPIOB->BRR  = ((~value)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
    GPIO_SET_BIT(GPIOB, LCD_WR);
    CPU_DELAY(LCD_ACCESS_GAP);
    GPIO_SET_BIT(GPIOB, LCD_CS);

}


/**********************************************************
设置显示窗口子函数
**********************************************************/
void LCD_set_drew_range(USHORT16 xStart, USHORT16 xEnd, USHORT16 yStart, USHORT16 yEnd)
{
#ifndef GET_HIGHER_SPEED
    if ((xStart > xEnd) || (LCD_SIZE_W <= xEnd) || (yStart > yEnd) || (LCD_SIZE_H <= yEnd))
    {
        return;
    }
#endif
	Reg_Write(0x200, xStart);
    Reg_Write(0x201, yStart);

    Reg_Write(0x0210, xStart);
    Reg_Write(0x0212, yStart);

    Reg_Write(0x211, xEnd);
    Reg_Write(0x213, yEnd);

	LCD_send_cmd(0x0202);
}

void LCD_init(void)
{
    g_print_CurrX = 0;
    g_print_CurrY = 0;

    GPIO_SET_BIT(GPIOA, LCD_RST);
    CPU_DELAY(LCD_ACCESS_GAP);
    GPIO_RESET_BIT(GPIOA, LCD_RST);
    CPU_DELAY(LCD_ACCESS_GAP);
    GPIO_SET_BIT(GPIOA, LCD_RST);
    CPU_DELAY(LCD_ACCESS_GAP);
    GPIO_RESET_BIT(GPIOB, LCD_CS);

    Reg_Write(0x000,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x000,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x000,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x000,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(100);
    LCD_send_data(0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    LCD_send_data(0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    LCD_send_data(0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    LCD_send_data(0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(100);

    Reg_Write(0x400,0x6200); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x008,0x0808); CPU_DELAY(LCD_ACCESS_GAP);

    Reg_Write(0x300,0x0c0c); CPU_DELAY(LCD_ACCESS_GAP);//GAMMA
    Reg_Write(0x301,0xff13); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x302,0x0f0f); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x303,0x150b); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x304,0x1020); CPU_DELAY(LCD_ACCESS_GAP);

    Reg_Write(0x305,0x0a0b); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x306,0x0003); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x307,0x0d06); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x308,0x0504); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x309,0x1030); CPU_DELAY(LCD_ACCESS_GAP);

    Reg_Write(0x010,0x001b); CPU_DELAY(LCD_ACCESS_GAP);   //60Hz
    Reg_Write(0x011,0x0101); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x012,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x013,0x0001); CPU_DELAY(LCD_ACCESS_GAP);

    Reg_Write(0x100,0x0330); CPU_DELAY(LCD_ACCESS_GAP);//BT,AP 0x0330
    Reg_Write(0x101,0x0247); CPU_DELAY(LCD_ACCESS_GAP);//DC0,DC1,VC
    Reg_Write(0x103,0x1000); CPU_DELAY(LCD_ACCESS_GAP);//VDV	//0x0f00
    Reg_Write(0x280,0xbf00); CPU_DELAY(LCD_ACCESS_GAP);//VCM
    Reg_Write(0x102,0xd1b0); CPU_DELAY(LCD_ACCESS_GAP);//VRH,VCMR,PSON,PON

    CPU_DELAY(1220);

    Reg_Write(0x001,0x0100); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x002,0x0100); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x003,0x1030); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x009,0x0001); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x00C,0x0000); CPU_DELAY(LCD_ACCESS_GAP);	//MCU interface
    Reg_Write(0x090,0x8000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x00f,0x0000); CPU_DELAY(LCD_ACCESS_GAP);


    Reg_Write(0x210,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x211,0x00ef); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x212,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x213,0x018f); CPU_DELAY(LCD_ACCESS_GAP);

    Reg_Write(0x500,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x501,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x502,0x005f); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x401,0x0001); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x404,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(500);

    Reg_Write(0x0007,0x0100); CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(1000);

    Reg_Write(0x200,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    Reg_Write(0x201,0x0000); CPU_DELAY(LCD_ACCESS_GAP);
    CPU_DELAY(500);

    LCD_send_cmd(0x0202);
    CPU_DELAY(500);
}

void LCD_set_bgColor(PRT_COLOR_E usColor)
{
    g_lcd_bkcolor = usColor;
}

PRT_COLOR_E LCD_get_bgColor(void)
{
    return (PRT_COLOR_E)g_lcd_bkcolor;
}
void LCD_set_forColor(PRT_COLOR_E usColor)
{
    g_lcd_forcolor = usColor;
}

PRT_COLOR_E LCD_get_forColor(void)
{
    return (PRT_COLOR_E)g_lcd_forcolor;
}


/**********************************************************
 清屏子函数
**********************************************************/
void LCD_clear_screen(void)
{
#if !defined(GET_HIGHER_SPEED)
    unsigned int com,seg;
#endif
    g_print_CurrX = g_print_CurrY = 0;

    LCD_set_drew_range(0, LCD_SIZE_W - 1, 0, LCD_SIZE_H - 1);

#if defined(GET_HIGHER_SPEED)
    LCD_send_data_batch(g_lcd_bkcolor, LCD_SIZE_W * LCD_SIZE_H);
    //LCD_send_data_batch(g_lcd_bkcolor, 48000);
    //LCD_send_data_batch(g_lcd_bkcolor, 48000);
#else
    for(seg=0;seg<LCD_SIZE_H;seg++)
    {
        for(com=0;com<LCD_SIZE_W;com++)
        {
            LCD_send_data(g_lcd_bkcolor);
        }
    }
#endif
}

void LCD_clear_line(void)
{
    USHORT16 usLoopCnt = LCD_CHAR_H * LCD_SIZE_W;

    LCD_set_drew_range(g_print_CurrX, LCD_SIZE_W - 1, g_print_CurrY, g_print_CurrY + LCD_CHAR_H -1);

#ifdef GET_HIGHER_SPEED
    LCD_send_data_batch(g_lcd_bkcolor, usLoopCnt);
#else
    while (usLoopCnt--)
    {
        LCD_send_data(g_lcd_bkcolor);
    }
#endif
}
void LCD_drew_point(USHORT16 x, USHORT16 y, PRT_COLOR_E usColor)
{
    LCD_set_drew_range(x, x, y, y);
    LCD_send_data(usColor);
}
void LCD_drew_line11(USHORT16 x0,USHORT16 y0,USHORT16 x1,USHORT16 y1, PRT_COLOR_E usColor)
{
    float dy,dx,x,y,m;
    dx=x1-x0;
    dy=y1-y0;
    m=dy/dx;
    if(x0<x1)
    {
        if(m<=1&&m>=-1)
        {
              y=y0;
              for(x=x0;x<=x1;x++)
              {
                  LCD_drew_point(x,(USHORT16)(y+0.5), usColor);
                  y+=m;
              }
        }
    }
    if(x0>x1)
    {
        if(m<=1&&m>=-1)
        {
              y=y0;
              for(x=x0;x>=x1;x--)
              {
                  LCD_drew_point(x,(USHORT16)(y+0.5), usColor);
                  y-=m;
              }
        }
    }
    if(y0<y1)
    {
        if(m>=1||m<=-1)
        {
              m=1/m;
              x=x0;
              for(y=y0;y<=y1;y++)
              {
                  LCD_drew_point((USHORT16)(x+0.5),y, usColor);
                  x+=m;
              }
        }
    }
    if(y0>y1)
    {
        if(m<=-1||m>=1)
        {
              m=1/m;
              x=x0;
              for(y=y0;y>=y1;y--)
              {
                  LCD_drew_point((USHORT16)(x+0.5),y, usColor);
                  x-=m;
              }
        }
    }
}
UCHAR8 LCD_rect_swap_pos(LCD_RECT_S *dst, LCD_RECT_S *src)
{
    USHORT16 usTemp = 0;
    if ((NULL == dst) || (NULL == src))
    {
        return ERR_CODE_NULL_POINTER;
    }

    usTemp = dst->pos_x;
    dst->pos_x = src->pos_x;
    src->pos_x = usTemp;

    usTemp = dst->pos_y;
    dst->pos_y = src->pos_y;
    src->pos_y = usTemp;

    dst->rpt_flag = LCD_RECT_REPAINT_ALL;
    src->rpt_flag = LCD_RECT_REPAINT_ALL;

    return ERR_CODE_OK;
}
UCHAR8 LCD_rect_dismiss(LCD_RECT_S *pstRect)
{
    USHORT16 bkRectColor = 0;
    if (NULL == pstRect)
    {
        return ERR_CODE_NULL_POINTER;
    }

    bkRectColor = pstRect->rect_color;

    pstRect->rect_color = LCD_get_bgColor();
    pstRect->rpt_flag = 0;
    pstRect->rpt_flag |= LCD_RECT_REPAINT_BACKGROUND;

    LCD_rect_drew(pstRect);

    pstRect->rect_color = bkRectColor;

    return ERR_CODE_OK;
}

UCHAR8 LCD_rect_reLocate(LCD_RECT_S *pstRect, SHORT16 dx, SHORT16 dy)
{
    if (NULL == pstRect)
    {
        return ERR_CODE_NULL_POINTER;
    }
    if (0 == (dx | dy))
    {
        return ERR_CODE_OK;
    }

    if (!LCD_IS_VALID_POS(dx, dy))
    {
        return ERR_CODE_PARA;
    }

    if (!LCD_IS_VALID_POS(dx + pstRect->length - 1, dy + pstRect->height - 1))
    {
        return ERR_CODE_PARA;
    }

    LCD_rect_dismiss(pstRect);

    pstRect->pos_x = dx;
    pstRect->pos_y = dy;
    pstRect->rpt_flag = LCD_RECT_REPAINT_ALL;
    LCD_rect_drew(pstRect);
    pstRect->rpt_flag = LCD_RECT_REPAINT_NONE;

    return ERR_CODE_OK;
}
UCHAR8 LCD_rect_set_content(LCD_RECT_S *pstRect, UCHAR8 *pucSrc)
{
    if ((NULL == pstRect) || (NULL == pucSrc))
    {
        return ERR_CODE_NULL_POINTER;
    }
    if (LCD_RECT_CONTENT_MAX_LEN <= strlen((const char*)pucSrc))
    {
        return ERR_CODE_OVERFLOW;
    }
    strcpy((char*)pstRect->txt, (char*)pucSrc);
    pstRect->rpt_flag |= LCD_RECT_REPAINT_TXT;

    return ERR_CODE_OK;
}

UCHAR8 LCD_rect_set_exdata(LCD_RECT_S *pstRect, void *pData)
{
    if ((NULL == pstRect) || (NULL == pData))
    {
        return ERR_CODE_NULL_POINTER;
    }

    pstRect->pdata_ext = pData;
    pstRect->rpt_flag |= LCD_RECT_REPAINT_EXDATA;

    return ERR_CODE_OK;
}


UCHAR8 LCD_rect_set_bg_color(LCD_RECT_S *pstRect, USHORT16 usColor)
{
    if (NULL == pstRect)
    {
        return ERR_CODE_NULL_POINTER;
    }

    pstRect->rect_color = usColor;
    pstRect->rpt_flag |= LCD_RECT_REPAINT_BACKGROUND | LCD_RECT_REPAINT_TXT | LCD_RECT_REPAINT_EXDATA;


    return ERR_CODE_OK;
}

UCHAR8 LCD_rect_set_content_color(LCD_RECT_S *pstRect, USHORT16 usColor)
{
    if (NULL == pstRect)
    {
        return ERR_CODE_NULL_POINTER;
    }

    pstRect->txt_color= usColor;
    pstRect->rpt_flag |= LCD_RECT_REPAINT_TXT;
    pstRect->rpt_flag |= LCD_RECT_REPAINT_EXDATA;

    return ERR_CODE_OK;
}


UCHAR8 LCD_rect_drew(LCD_RECT_S *pstRect)
{
    ULONG32 ulRectSize = 0;

    if (NULL == pstRect)
    {
        return ERR_CODE_NULL_POINTER;
    }
    /* rect dispear 时只有背景需要重画, 所以要单独判断 */
    if (LCD_RECT_REPAINT_BACKGROUND & pstRect->rpt_flag)
    {
        pstRect->rpt_flag &= ~LCD_RECT_REPAINT_BACKGROUND;
        LCD_set_drew_range(pstRect->pos_x,
                        pstRect->pos_x + pstRect->length - 1,
                        pstRect->pos_y,
                        pstRect->pos_y + pstRect->height - 1);

        ulRectSize = pstRect->height * pstRect->length;

#ifdef GET_HIGHER_SPEED
        LCD_send_data_batch(pstRect->rect_color, ulRectSize);
#else
        while(ulRectSize--)
        {

            LCD_send_data(pstRect->rect_color);
        }
#endif
    }

    if (LCD_RECT_REPAINT_TXT & pstRect->rpt_flag)
    {
        pstRect->rpt_flag &= ~LCD_RECT_REPAINT_TXT;
        LCD_drew_string(pstRect->pos_x,
                        pstRect->pos_y + 2,
                        pstRect->txt_color,
                        pstRect->rect_color,
                        pstRect->txt);
    }

    if ((NULL != pstRect->pdata_ext)
        && (LCD_RECT_REPAINT_EXDATA & pstRect->rpt_flag))
    {
        pstRect->rpt_flag &= ~LCD_RECT_REPAINT_EXDATA;
        LCD_drew_string(pstRect->pos_x,
                        pstRect->pos_y + LCD_CHAR_H + 2,
                        pstRect->txt_color,
                        pstRect->rect_color,
                        pstRect->pdata_ext);
    }

    return ERR_CODE_OK;
}
void LCD_drew_line(USHORT16 x0, USHORT16 y0, USHORT16 x1, USHORT16 y1, PRT_COLOR_E usColor)
{
    float k = 0;
    USHORT16 x = 0;
    USHORT16 y = 0;
    CHAR8 signx = 0;
    CHAR8 signy = 0;

#define GET_MIN(a, b) ((a) < (b)) ? (a) : (b)
#define GET_MAX(a, b) ((a) > (b)) ? (a) : (b)

    if (x0 == x1)
    {
        y = GET_MIN(y0, y1);
        y1 = GET_MAX(y0, y1);
        y0 = y;

        for ( ; y0 <= y1; y0++)
        {
            LCD_drew_point(x0, y0, usColor);
        }
    }
    else if (y0 == y1)
    {
        x = GET_MIN(x0, x1);
        x1 = GET_MAX(x0, x1);
        x0 = x;

        for ( ; x0 <= x1; x0++)
        {
            LCD_drew_point(x0, y0, usColor);
        }
    }
    else
    {
        signx = (x0 < x1) ? 1 : -1;
        signy = (y0 < y1) ? 1 : -1;
        k = (float)(y1 - y0) / (x1 - x0);

        for (; x0 != x1; x0 += signx)
        {
            LCD_drew_point(x0,  (USHORT16)(y0 + k*signy + (0.5*signy)), usColor);
            y0 += k*signy;
        }
    }

}


/**********************************************************
  字符显示子函数
**********************************************************/
void LCD_drew_char(USHORT16 x, USHORT16 y, USHORT16 For_color,
                                USHORT16 Bk_color, char ch)
{
    data USHORT16 dx = 0;
    data USHORT16 dy = 0;
    data UCHAR8 char_map = 0;
#ifndef GET_HIGHER_SPEED
    if( x> (LCD_SIZE_W - LCD_CHAR_W) || y > (LCD_SIZE_H - LCD_CHAR_H))
    {
        return;
    }
#endif

    /*LCD_set_drew_range(dx, dx, dy, dy);*/
	Reg_Write(0x200, x);
    Reg_Write(0x201, y);
    Reg_Write(0x0210, x);
    Reg_Write(0x0212,y);
    Reg_Write(0x211,x + LCD_CHAR_W - 1);
    Reg_Write(0x213,y + LCD_CHAR_H - 1);
	LCD_send_cmd(0x0202);

    ch -= 0x20;
    for (dy = y; dy < y + LCD_CHAR_H; dy++)
    {
        char_map= Font8x16[ch][dy - y];
        for (dx = x; dx < x + LCD_CHAR_W; dx++)
        {
            if (char_map & 0x80)
            {
                GPIO_RESET_BIT(GPIOB, LCD_CS);
                //GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_DATA);
                GPIO_SET_BIT(GPIOA, LCD_RS);/*TYPE_LCD_DATA*/
                GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
                LCD_DATA    = (UCHAR8)(For_color>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
                GPIOB->BSRR = For_color & 0xff00;
                GPIOB->BRR  = (~For_color) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
                GPIO_SET_BIT(GPIOB, LCD_WR);

                CPU_DELAY(LCD_ACCESS_GAP);

                GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
                LCD_DATA    = (UCHAR8)For_color;
#elif defined(_CPU_ARM_STM32F103C8_)
                GPIOB->BSRR = (For_color)<<8 & 0xff00;
                GPIOB->BRR  = ((~For_color)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
                GPIO_SET_BIT(GPIOB, LCD_WR);
                CPU_DELAY(LCD_ACCESS_GAP);
                GPIO_SET_BIT(GPIOB, LCD_CS);
            }
            else
            {
                GPIO_RESET_BIT(GPIOB, LCD_CS);
                //GPIO_WRITE(GPIOA, LCD_RS, TYPE_LCD_DATA);
                GPIO_SET_BIT(GPIOA, LCD_RS);/*TYPE_LCD_DATA*/
                GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
                LCD_DATA    = (UCHAR8)(Bk_color>>8) ;
#elif defined(_CPU_ARM_STM32F103C8_)
                GPIOB->BSRR = Bk_color & 0xff00;
                GPIOB->BRR  = (~Bk_color) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
                GPIO_SET_BIT(GPIOB, LCD_WR);

                CPU_DELAY(LCD_ACCESS_GAP);

                GPIO_RESET_BIT(GPIOB, LCD_WR);
#if defined(_CPU_C51_STC90C516RD_)
                LCD_DATA    = (UCHAR8)Bk_color;
#elif defined(_CPU_ARM_STM32F103C8_)
                GPIOB->BSRR = (Bk_color)<<8 & 0xff00;
                GPIOB->BRR  = ((~Bk_color)<<8) & 0xff00;
#else
#error "INVALID CPU TYPE"
#endif
                GPIO_SET_BIT(GPIOB, LCD_WR);
                CPU_DELAY(LCD_ACCESS_GAP);
                GPIO_SET_BIT(GPIOB, LCD_CS);
            }
            char_map <<= 1;
        }
    }

}

void LCD_printf(UCHAR8 *fmt, ...)
{
    va_list ap;

    va_start(ap,fmt);
    vsprintf((char*)g_lcd_printf_buf, (char*)fmt, ap);
    LCD_printf_color(g_lcd_printf_buf, LCD_get_forColor(), transp);
    va_end(ap);
}

void LCD_printf_color(UCHAR8 *pucStr, PRT_COLOR_E Front_eColor, PRT_COLOR_E BKGD_eColor)
{
    UCHAR8 ucIdx = 0;
    UCHAR8 ucNewLineFlag = FALSE;
    UCHAR8 ucPrtCnt = 0;
    UCHAR8 ucSpaceCnt = 0;


    if (EOC != pucStr[LCD_PRINT_STR_MAX_LEN - 1])
    {
        pucStr[LCD_PRINT_STR_MAX_LEN - 1] = EOC;
    }

    /*如果用透明色,则把背景颜色设置为系统底色*/
    if (transp == BKGD_eColor)
    {
       BKGD_eColor = LCD_get_bgColor();
    }

    LCD_clear_line();
    while(EOC != pucStr[ucIdx])
    {
        if ((LCD_SIZE_W - LCD_CHAR_W) < g_print_CurrX)
        {
            g_print_CurrX = 0;
            g_print_CurrY += LCD_CHAR_H;
            LCD_clear_line();
        }
        if ((LCD_SIZE_H - LCD_CHAR_H) < g_print_CurrY)
        {
            g_print_CurrX = 0;
            g_print_CurrY = 0;
            LCD_clear_line();
        }

        if (TRUE == ucNewLineFlag)
        {
            LCD_clear_line();
            ucNewLineFlag = FALSE;
            ucPrtCnt = 0;
        }
#define TAP_ALIGN_WIDTH     8
        switch (pucStr[ucIdx])
        {
            case '\t':
                ucSpaceCnt = (0 == ucPrtCnt) ? TAP_ALIGN_WIDTH : (((ucPrtCnt+(TAP_ALIGN_WIDTH-1))/TAP_ALIGN_WIDTH)*TAP_ALIGN_WIDTH)
                                - ucPrtCnt;
                while(ucSpaceCnt--)
                {
                    LCD_drew_char( g_print_CurrX,
                                            g_print_CurrY,
                                             Front_eColor,
                                                BKGD_eColor,
                                                 ' ');
                    g_print_CurrX += LCD_CHAR_W;
                    ucPrtCnt++;
                }
                ucPrtCnt = 0;
                break;
            case '\r':
                g_print_CurrX = 0;
                ucPrtCnt = 0;
                break;
            case '\n':
                g_print_CurrY += LCD_CHAR_H;
                ucNewLineFlag = TRUE;
                break;
            default:
                LCD_drew_char( g_print_CurrX,
                        g_print_CurrY,
                         Front_eColor,
                            BKGD_eColor,
                             pucStr[ucIdx]);

                g_print_CurrX += LCD_CHAR_W;
                ucPrtCnt++;
        }

        ucIdx++;
    }
}

/**********************************************************
 显示字符串子函数
 x,y:起点坐标
 *p:字符串起始地址
**********************************************************/
UCHAR8 LCD_drew_string(USHORT16 x, USHORT16 y,
                    USHORT16 For_color, USHORT16 Bk_color, UCHAR8 *p)
{
   if (NULL == p)
   {
        return ERR_CODE_NULL_POINTER;
   }
   while(*p!='\0')
   {
     LCD_drew_char(x,y,For_color,Bk_color,*p);
     x += LCD_CHAR_W;
     p++;
   }

   return ERR_CODE_OK;
}
void LCD_demo(void)
{
    int x = 0;
    int y = 0;
    int drew_cnt = 0;
    USHORT16 color = 0;

//    task_delay(1000);

    LCD_drew_line(0, 0, 239, 0, blue);
    LCD_drew_line(0, 0, 0, 399, green);
    LCD_drew_line(0, 399, 239, 399, brown);
    LCD_drew_line(239, 0, 239, 399, red);


#if 1
    LCD_drew_line(0, 100, 239, 100, yellow);
    LCD_drew_line(0, 200, 239, 200, white);
    LCD_drew_line(0, 300, 239, 300, blue);
    LCD_drew_line(120, 0, 120, 399, red);
#endif

#define LCD_DEMO_CNT 500

    while(drew_cnt++ < LCD_DEMO_CNT)
    {
        x = rand() % LCD_SIZE_W;
        y = LCD_SIZE_H - (rand() % (drew_cnt/25+10));
        color = x % GET_ARRAY_CNT(default_color);
        LCD_drew_point(x, y, (PRT_COLOR_E)default_color[color]);
        //task_delay(3);
    }

    LCD_set_bgColor(black);
    LCD_clear_screen();
}
