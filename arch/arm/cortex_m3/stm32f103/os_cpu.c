/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_cpu.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS cpu source file
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
#include <melon/melon.h>
#include <melon/os_task.h>
#include <arch/os_cpu.h>

#ifdef _CPU_ARM_STM32F103C8_

#include <stm32f10x_systick.h>
#include <stm32f10x_nvic.h>


#define NVIC_INT_CTRL        0xE000ED04     //Interrupt control state register.
#define NVIC_SYSPRI14        0xE000ED22     //System priority register (priority 14).
#define NVIC_PENDSV_PRI      0xFF           //PendSV priority value (lowest).
#define NVIC_PENDSVSET       0x10000000     //Value to trigger PendSV exception.


void OS_TaskReturn(void  *ptcb)
{
    OS_task_exit();
}

/*
*********************************************************************************************************
* This function is called by OS_task_create to
* initialize the stack frame of the task being created.
* This function is highly processor specific.
*********************************************************************************************************
*/
OS_STK* OS_task_stack_init (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, USHORT16 opt)
{
    OS_STK *pstk_top;

    (void)opt;                              /* 'opt' is not used, prevent warning */
    pstk_top       = ptos;                  /* Load stack pointer */

    /* Registers stacked as if auto-saved on exception    */
    *(pstk_top)    = (OS_STK)0x01000000uL;  /* xPSR */
    *(--pstk_top)  = (OS_STK)task;           /* Entry Point */
    *(--pstk_top)  = (OS_STK)OS_TaskReturn;  /* R14 (LR) */
    *(--pstk_top)  = (OS_STK)0xCCCCCCCCuL;  /* R12 */
    *(--pstk_top)  = (OS_STK)0x33333333uL;  /* R3 */
    *(--pstk_top)  = (OS_STK)0x22222222uL;  /* R2 */
    *(--pstk_top)  = (OS_STK)0x11111111;    /* R1 */
    *(--pstk_top)  = (OS_STK)p_arg;          /* R0 : argument */

    /* Remaining registers saved on process stack */
    *(--pstk_top)  = (OS_STK)0xBBBBBBBB;    /* R11 */
    *(--pstk_top)  = (OS_STK)0xAAAAAAAA;    /* R10 */
    *(--pstk_top)  = (OS_STK)0x99999999;    /* R9  */
    *(--pstk_top)  = (OS_STK)0x88888888;    /* R8 */
    *(--pstk_top)  = (OS_STK)0x77777777;    /* R7 */
    *(--pstk_top)  = (OS_STK)0x66666666;    /* R6 */
    *(--pstk_top)  = (OS_STK)0x55555555;    /* R5 */
    *(--pstk_top)  = (OS_STK)0x44444444;    /* R4 */

    return pstk_top;
}

/*****************************************************************************
 Prototype    : OS_tick_start
 Input        : void
 Output       : None
 Return Value : void
 Description  : start os tick generator
 Calls        :
*****************************************************************************/
void OS_tick_start(void)
{
    SysTick_CounterCmd(SysTick_Counter_Enable);
    return;
}

/*****************************************************************************
 Prototype    : OS_tick_stop
 Input        : void
 Output       : None
 Return Value : void
 Calls        :
 Description  : stop os tick generator

*****************************************************************************/
void OS_tick_stop(void)
{
    SysTick_CounterCmd(SysTick_Counter_Disable);
    return;
}

/*****************************************************************************
 Prototype    : OS_tick_init
 Input        : ULONG32 tick_hz
 Output       : None
 Return Value : void
 Description  : Init os tick
 Calls        :
*****************************************************************************/
OS_RET_CODE OS_tick_init(ULONG32 tick_hz)
{
    if (tick_hz > OS_TICK_HZ_MAX)
    {
        return OS_RET_NOT_SUPPORT;
    }

    /* 设置AHB时钟为SysTick时钟,8分频 */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    /* 设置SysTicks中断抢占优先级 3， 从优先级0*/
    NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 3, 0);
    /* 1秒钟9Mhz的递减速率, 设置为10ms周期 */
    SysTick_SetReload(CPU_OPS_1SEC/tick_hz);
    /* 开启SysTick中断 */
    SysTick_ITConfig(ENABLE);

    return OS_RET_SUCCESS;
}

void HD_reset(void)
{
    NVIC_SETFAULTMASK();
    NVIC_GenerateSystemReset();
}

void fault_dbgstk(ULONG32* pwdSF)
{
    extern void Serial_printf(const UCHAR8* fmt, ...);
    extern void Beep_on(ULONG32 time);
    extern void Sys_delay_cpu_opr(ULONG32 nCount);

    ULONG32 beep_cnt;
    ULONG32 stacked_r0;
    ULONG32 stacked_r1;
    ULONG32 stacked_r2;
    ULONG32 stacked_r3;
    ULONG32 stacked_r12;
    ULONG32 stacked_lr;
    ULONG32 stacked_pc;
    ULONG32 stacked_psr;

    stacked_r0 =  pwdSF[0];
    stacked_r1 =  pwdSF[1];
    stacked_r2 =  pwdSF[2];
    stacked_r3 =  pwdSF[3];
    stacked_r12 = pwdSF[4];
    stacked_lr =  pwdSF[5];
    stacked_pc =  pwdSF[6];
    stacked_psr = pwdSF[7];

    Serial_printf("[Hard fault handler]\n");
    Serial_printf("R0 = %x\n\r", stacked_r0);
    Serial_printf("R1 = %x\n\r", stacked_r1);
    Serial_printf("R2 = %x\n\r", stacked_r2);
    Serial_printf("R3 = %x\n\r", stacked_r3);
    Serial_printf("R12 = %x\n\r", stacked_r12);
    Serial_printf("LR = %x\n\r", stacked_lr);
    Serial_printf("PC = %x\n\r", stacked_pc);
    Serial_printf("PSR = %x\n\r", stacked_psr);
    Serial_printf("BFAR = %x\n\r", (*((volatile ULONG32 *)(0xE000ED38))));
    Serial_printf("CFSR = %x\n\r", (*((volatile ULONG32 *)(0xE000ED28))));
    Serial_printf("HFSR = %x\n\r", (*((volatile ULONG32 *)(0xE000ED2C))));
    Serial_printf("DFSR = %x\n\r", (*((volatile ULONG32 *)(0xE000ED30))));
    Serial_printf("AFSR = %x\n\r", (*((volatile ULONG32 *)(0xE000ED3C))));

    beep_cnt = 3;
    while (beep_cnt--)
    {
        Beep_on(100);
        Sys_delay_cpu_opr(1000*1000*9); /* delay 1000ms */
    }
    Sys_delay_cpu_opr(10*1000*1000*9);
    HD_reset();
    return;
}

__asm void svc_fault_handler(void)
{
    IMPORT fault_dbgstk
    TST LR, #4
    ITE EQ
    MRSEQ R0, MSP
    MRSNE R0, PSP
    B fault_dbgstk
}

/*
    Init main stack after OS_pHandler_main_stk got value,
    I must call it in a unreturn function,
    Because mainstk will be  modified in this function, LR is missed,
    So it can not return normally
*/
__asm void OS_mainstk_init(void)
{
    EXTERN  OS_pHandler_main_stk

    //Set the PendSV exception priority
    LDR     R0, =NVIC_SYSPRI14
    LDR     R1, =NVIC_PENDSV_PRI
    STRB    R1, [R0]

    //Set the PSP to 0 for initial context switch call
    MOVS    R0, #0
    MSR     PSP, R0
    //Initialize the MSP to the OS_handler_main_stk
    LDR     R0, =OS_pHandler_main_stk
    LDR     R1, [R0]
    MSR     MSP, R1
    BX	    LR
    NOP
}

/*
    Set prio int mask to mask all (except faults)
*/
__asm OS_ISR_STATE OS_int_disable(void)
{
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR
}

__asm void OS_int_enable(OS_ISR_STATE cpu_sr)
{
    MSR     PRIMASK, R0
    BX      LR
}

/*
    Trigger the PendSV exception (causes context switch)
*/
__asm void OS_ctx_switch_t(void)
{

        LDR     R0, =NVIC_INT_CTRL
        LDR     R1, =NVIC_PENDSVSET
        STR     R1, [R0]
        BX      LR
}

/*
    Trigger the PendSV exception (causes context switch)
*/
__asm void OS_ctx_switch_i(void)
{
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR
    NOP
}

__asm void OS_CPU_PendSVHandler(void)
{
    EXTERN  OS_pCurrTask
    EXTERN  OS_pNextTask

    //Prevent interruption during context switch
    CPSID   I
    //Skip register save the first time
    MRS     R0, PSP
    CBZ     R0, CTX_RESTORE

    SUBS    R0, R0, #0x20
    //Save remaining regs r4-11 on process stack
    STM     R0, {R4-R11}
    //把OS_pCurrTCB存入R1, 即当前的TCB指针
    LDR     R1, =OS_pCurrTask
    //把当前TCB的第一个字读入R1, 即当前堆栈的地址
    LDR     R1, [R1]
    //R0 is SP of process being switched out
    STR     R0, [R1]
    //At this point, entire context of process has been saved

CTX_RESTORE
    LDR     R0, =OS_pNextTask
    LDR     R1, =OS_pCurrTask
    LDR     R2, [R0]
    //copy OS_pNextTask to OS_pCurrTask
    STR     R2, [R1]
    //R1 gets the current tcb
    LDR     R3, [R1]
    //把NextTCB内的sp指针取出
    LDR     R0, [R3]
    //Restore r4-11 from new process stack
    LDM     R0, {R4-R11}
    //增加8个字，指向了堆栈的顶部
    ADDS    R0, R0, #0x20
    //把R0的值，即当前的堆栈顶部地址给PSP
    STR     R0,  [R3]
    //refresh the sp
    MSR     PSP, R0
    //Ensure exception return uses process stack
    ORR     LR, LR, #0x04
    CPSIE   I
    //Exception return will restore remaining context
    BX      LR
    //如果有编译warning, 加一个空指令NOP来align4
}
#endif /* #ifdef _CPU_ARM_STM32F103C8_ */

