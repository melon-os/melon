/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : melon_cfg.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS Configuration file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#ifndef __MELON_CFG_H__
#define __MELON_CFG_H__

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/
#define OS_VERSION                  "0.19a"
/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
/* Common compile configurate */
//#define MELON_DBG

#ifdef MELON_DBG
#define RUN_ON_SIMULATOR
#else
#define RUN_ON_BOARD
#endif
#define MANUEL_INPUT

/* OS feature support selection */
#define OS_SUPPORT_MEM              /* support memory management */
#define OS_SUPPORT_STAT             /* support os statistic */
#define OS_SUPPORT_TIMER            /* support timer */

/* task */
#define OS_TASK_CNT_MAX             8
#ifdef OS_SUPPORT_STAT
#define OS_TASK_CNT_MIN             3   /* idle task, stat task, app task */
#else
#define OS_TASK_CNT_MIN             2   /* idle task, app task */
#endif /* #ifdef OS_SUPPORT_STAT */
#define OS_TASK_PRIORITY_LOWEST     127
#define OS_TASK_NAME_LEN_MAX        16
#define OS_STKSIZE_DEFAULT          1024
/* #####################Do not modify macro in this area########################*/
#define OS_IDLE_TASK_ID             0
#define OS_TASK_ID_INVALID          (OS_TASK_CNT_MAX)
/* #####################Do not modify macro in this area########################*/

/* schedule */
#define OS_TICK_PERIOD              10  /* 10ms */
#define OS_TICK_HZ                  (1000/OS_TICK_PERIOD) /* schedule 100times per second */
#define OS_TICK_HZ_MAX              1000
//#define OS_SWX_TICK_REFRESH

/* mutex */
#define OS_MUTEX_CNT_MAX            4
//#define OS_MUTEX_SUPPORT_NAME       /* wether a mutex has a name or not */
#define OS_MUTEX_NAME_LEN_MAX       16
#define OS_MUTEX_WAITLIST_CNT_MAX   OS_TASK_CNT_MAX

/* mailbox */
#define OS_MBOX_CNT_MAX             3
#define OS_MBOX_MSG_CNT_MAX         4
#define OS_MBOX_SUPPORT_INTER       /* support mailbox in interrupt contex */

/* timer */
#define OS_TIMER_CNT_MAX            2
#define OS_TIMER_ID_INVALID         OS_TIMER_CNT_MAX
//#define OS_TIMER_SUPPORT_NAME       /* wether a timer has a name or not */
#define OS_TIMER_NAME_LEN_MAX       16

#ifdef OS_SUPPORT_MEM
/* memory management */
#define OS_MEM_SIZE                 10 /* size of memory pool, n*KBytes */
#define OS_MEM_CTRL_CNT             32 /* number of memory control item */
#endif

#endif /* __MELON_CFG_H__*/
