/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_core.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS core header file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef __OS_CORE_H__
#define __OS_CORE_H__
#include <melon/melon.h>

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void OS_init(void);
extern void OS_start(void);
extern OS_STATE OS_get_state(void);
extern void OS_lock_scheduler(BOOL b);
extern void OS_schedule(void);
extern void OS_tick_schedule(void);
extern CHAR8*  __strcpy(CHAR8 *dst, CHAR8 *src);
extern INT32  __strcmp(CHAR8 *dst, CHAR8 *src);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __OS_CORE_H__ */
