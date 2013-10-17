/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : serial.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Serial header file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef _SERIAL_H_
#define _SERIAL_H_	

#include <stm32f10x_usart.h>
#include <common_func.h>


typedef enum {
    S_4800_BPS = 4800,
    S_9600_BPS = 9600,
    S_14400_BPS = 14400,
    S_19200_BPS = 19200,
    S_115200_BPS = 115200,
    S_128000_BPS = 128000,
    S_256000_BPS = 256000,    
}SERIAL_SPEED_MODE_E;

#define SERIAL_RXBUF_SIZE           16
#define SERIAL_TXBUF_SIZE           256
/*一次最多打印60个字符, 与LCD(最大打印2行)匹配*/
#define SERIAL_PRINT_STR_MAX_LEN       SYS_PRINT_STR_MAX_LEN 

/* Disable the USART1 Receive interrupt */
/* Disable the USART1 transmit interrupt */
#define ENTER_CRITICAL_SECTION()
//do {\
//    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);\
//    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);\
//} while (0);

#define EXIT_CRITICAL_SECTION()
//do {\
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);\
//    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);\
//} while (0);


/*
printf("\033[1;40;32m%s\033[0m",” Hello,NSFocus\n”); 
\033 声明了转义序列的开始，然后是 [ 开始定义颜色。后面的 1 定义了高亮显示字符。然后是背景颜色，这里面是40
，表示黑色背景。接着是前景颜色，这里面是32，表示绿色。我们用 \033[0m 关闭转义序列， \033[0m 是终端默认颜色。 
通过上面的介绍，就知道了如何输出彩色字符了。因此，我就不再多说了。下面是对于彩色字符颜色的一些定义： 
前景            背景              颜色
    ---------------------------------------
    30                40              黑色
    31                41              紅色
    32                42              綠色
    33                43              黃色
    34                44              藍色
    35                45              紫紅色
    36                46              青藍色
    37                47              白色 
    代码              意义
    -------------------------
    0                终端默认设置
    1                高亮显示
    4                使用下划线
    5                闪烁
    7                反白显示
    8                不可见
*/
#define VT_DEFAULT  "\033[0m"
#define VT_UNDERLINE "\033[4m"
#define VT_BLACK    "\033[1;40;30m"
#define VT_RED      "\033[1;40;31m"
#define VT_GREEN    "\033[1;40;32m"
#define VT_YELLOW   "\033[1;40;33m"
#define VT_BLUE     "\033[1;40;34m"
#define VT_WHITE    "\033[1;40;37m"


extern UCHAR8 Serial_init(SERIAL_SPEED_MODE_E eSpeedMode);
extern UCHAR8 Serial_write_char(const UCHAR8 ucSendData);
extern UCHAR8 Serial_read_char(UCHAR8 *pucSendData);
extern int fputc(int ch, FILE *f);
//extern int fgetc(FILE *f);
extern int sgetc(void);
extern void Serial_printf(const UCHAR8* fmt, ...);
extern UCHAR8 Serial_print_string(UCHAR8 *pcBuf);
extern OS_RET_CODE Serial_set_color(PRT_COLOR_E color);

#endif /*#ifndef _SERIAL_H_*/
