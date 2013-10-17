/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.  
    * If you plan on using Melon in a commercial product you need to contact me to properly license 
    * its use in your product. I provide ALL the source code for your convenience and to help you experience 
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a 
    * licensing fee.

    File Name     : os_cpu.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS cpu typedef file
    For           : ARMv7M Cortex-M3
    Mode          : Thumb2
    Toolchain     :   RealView Development Suite
                      RealView Microcontroller Development Kit (MDK)
                      ARM Developer Suite (ADS)
                      Keil uVision
                      
    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#ifndef  __OS_CPU_H__
#define  __OS_CPU_H__

#define _CPU_ARM_STM32F103C8_ /* 当前支持的CPU类型 */
#undef _CPU_C51_STC90C516RD_

#define CPU_OPS_1SEC        9000000  /* cortex-m3 每秒可以对tick计数器递增/递减这些次*/
typedef unsigned char       BOOL;
typedef char                CHAR8;
typedef unsigned char       UCHAR8;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef signed int          INT32;
typedef unsigned int        UINT32;
typedef short               SHORT16;
typedef unsigned short      USHORT16;
typedef long                LONG32;
typedef unsigned long       ULONG32;
typedef unsigned long long  ULONG64;


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern ULONG32 OS_int_disable(void);
extern void OS_int_enable(ULONG32 cpu_sr);
extern ULONG32* OS_task_stack_init (void (*task)(void *p_arg), void *p_arg, ULONG32 *ptos, USHORT16 opt);
extern void OS_ctx_switch_t(void);
extern void OS_ctx_switch_i(void);
extern void OS_mainstk_init(void);
extern UCHAR8 OS_tick_init(ULONG32 tick_hz);
extern void OS_tick_start(void);
extern void OS_tick_stop(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__OS_CPU_H__*/
