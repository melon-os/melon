/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : lcd.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : LCD header file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef _LCD_H_
#define _LCD_H_
#include <common_func.h>


#define TYPE_LCD_DATA		1
#define TYPE_LCD_COMMAND	0
#define LCD_SIZE_W		    240
#define LCD_SIZE_H		    400

#define LCD_CHAR_W         8
#define LCD_CHAR_H         16
/*限定最大可一次打印2行, 60个字符*/
#define LCD_PRINT_STR_MAX_LEN  SYS_PRINT_STR_MAX_LEN
#define LCD_RECT_CONTENT_MAX_LEN    32

typedef enum tag_LCD_REPAINT_FLAG{
    LCD_RECT_REPAINT_NONE = 0x0,
    LCD_RECT_REPAINT_BACKGROUND = 0x1,
    LCD_RECT_REPAINT_TXT = 0x2,    
    LCD_RECT_REPAINT_EXDATA = 0x4,
    LCD_RECT_REPAINT_ALL = 0xFF,
} LCD_REPAINT_FLAG_E;

#define LCD_IS_VALID_POS(x, y) (\
                            ((x) >= 0) \
                            && ((x) < LCD_SIZE_W) \
                            && ((y) >= 0) \
                            && ((y) < LCD_SIZE_H))

typedef struct tag_LCD_RECT {
    UCHAR8 rect_id;
    USHORT16 pos_x;
    USHORT16 pos_y;
    USHORT16 height;    
    USHORT16 length;
    USHORT16 rect_color;
    USHORT16 txt_color;
    UCHAR8 txt[LCD_RECT_CONTENT_MAX_LEN];
    UCHAR8 rpt_flag;/*是否重画的标志,背景,txt, extdata分别占据一个bit*/
    void *pdata_ext;
} LCD_RECT_S;

extern void LCD_init(void);
extern void LCD_demo(void);
extern void LCD_clear_screen(void);
extern void LCD_set_bgColor(PRT_COLOR_E usColor);
extern PRT_COLOR_E LCD_get_bgColor(void);
extern void LCD_set_forColor(PRT_COLOR_E usColor);
extern PRT_COLOR_E LCD_get_forColor(void);
extern void LCD_printf(UCHAR8 *fmt, ...);
extern void LCD_printf_color(UCHAR8 *pucStr, PRT_COLOR_E Front_eColor, PRT_COLOR_E BKGD_eColor);
extern void LCD_drew_point(USHORT16 x, USHORT16 y, PRT_COLOR_E usColor);
extern void LCD_drew_line(USHORT16 x0, USHORT16 y0, USHORT16 x1, USHORT16 y1, PRT_COLOR_E usColor);
extern UCHAR8 LCD_rect_drew(LCD_RECT_S *pstRect);
extern UCHAR8 LCD_rect_reLocate(LCD_RECT_S *pstRect, SHORT16 dx, SHORT16 dy);
extern UCHAR8 LCD_rect_swap_pos(LCD_RECT_S *dst, LCD_RECT_S *src);
extern UCHAR8 LCD_rect_dismiss(LCD_RECT_S *pstRect);
extern UCHAR8 LCD_rect_set_content(LCD_RECT_S *pstRect, UCHAR8 *pucSrc);
extern UCHAR8 LCD_rect_set_exdata(LCD_RECT_S *pstRect, void *pData);
extern UCHAR8 LCD_rect_set_bg_color(LCD_RECT_S *pstRect, USHORT16 usColor);
extern UCHAR8 LCD_rect_set_content_color(LCD_RECT_S *pstRect, USHORT16 usColor);
extern void LCD_drew_char(USHORT16 x,USHORT16 y,USHORT16 For_color,USHORT16 Bk_color,char ch);
extern UCHAR8 LCD_drew_string(USHORT16 x,USHORT16 y,USHORT16 For_color,USHORT16 Bk_color,UCHAR8 *p);
#endif /*#ifndef _LCD_H_*/
