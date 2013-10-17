/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : main.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS main entry source file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <common_func.h>
#include <drv/lcd.h>
#include <drv/temperature.h>
#include <drv/clock.h>
#include <drv/ir.h>
#include <drv/ad.h>
#include <drv/serial.h>
#include <drv/speaker.h>

extern int melon_entry(void);

int main(void)
{
    Sys_bsp_init();
    Serial_init(S_256000_BPS);
#ifdef RUN_ON_BOARD
    Temperaturn_init_18b20();
    LCD_init();
    Sys_set_print_color(yellow, black);
    LCD_clear_screen();
    Sys_display_evn();
    Beep_on(100);
#endif
    melon_entry();
}

