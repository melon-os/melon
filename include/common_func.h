#ifndef _COMMON_FUNC_H_
#define _COMMON_FUNC_H_
#include <arch/os_cpu.h>
#if defined(_CPU_C51_STC90C516RD_)
#include <REG51.H>
#include <intrins.h>
#elif defined(_CPU_ARM_STM32F103C8_)
#include <stm32f10x_lib.h>
#include <stm32f10x_type.h>
#include <stm32f10x_map.h>
#else
#error "INVALID CPU TYPE"
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <melon/melon.h>
#include <melon/os_core.h>
#include <melon/os_task.h>
#include <melon/os_mutex.h>
#include <melon/os_mem.h>
#include <melon/os_mailbox.h>


#if defined(_CPU_ARM_STM32F103C8_)
#define STATIC  static
#define CONST   const
#define CODE    const

#define data
#define idata
#define xdata
#define code
typedef unsigned char bit;
#endif


/* configuration block */
#if defined(_CPU_C51_STC90C516RD_)
#if 0
#define OSC_MODE_12M
#else
#define OSC_MODE_24M
#endif

#if 0
#define OSC_CLOCK_12T
#else
#define OSC_CLOCK_6T
#endif

#elif defined(_CPU_ARM_STM32F103C8_)
#define STM32F103C8_SUPPORT_TIM2
#else
#error "INVALID CPU TYPE"
#endif


/* LCD一次最多打印2行,60个字符, 串口与此匹配 */
#define SYS_PRINT_STR_MAX_LEN   61
#define GET_HIGHER_SPEED

typedef enum tag_print_color {
     red = 0xf800,
     green = 0x07e0,
     blue = 0x001f,
     yellow = 0xffe0,
     black = 0x0000,
     brown = 0x07ff,
     pink = 0xf81f,
     white = 0xffff,
     transp = 0x1111,
     underline = 0x2222,
} PRT_COLOR_E;


typedef enum {
    ERR_CODE_OK = 0x0,
    ERR_CODE_FAIL,
    ERR_CODE_PARA,
    ERR_CODE_NULL_POINTER,
    ERR_CODE_NOT_SUPPORT,
    ERR_CODE_NOT_READY,
    ERR_CODE_UNDERFLOW, /* no data in buf */
    ERR_CODE_OVERFLOW,
}ERROR_CODE_E;

typedef enum {
    D_45 = 0x0,
    D_135,
    D_225,
    D_UP ,
    D_DOWN,
    D_LEFT,
    D_RIGHT,
    D_315,
    D_CNT,
} DIRECTION_E;
#define GET_ARRAY_CNT(arrName) (sizeof(arrName)/sizeof(arrName[0]))

#if defined(_CPU_C51_STC90C516RD_)
#define GPIOA NULL
#define GPIOB NULL

#define GPIO_SET_BIT(TYPE, ID) do {\
    ID = 1;\
} while (0);
#define GPIO_RESET_BIT(TYPE, ID) do {\
    ID = 0;\
} while (0);

#define GPIO_GET_BIT(TYPE, ID)   (ID)

#define GPIO_WRITE(TYPE, ID, VALUE) ID = VALUE;

#define CPU_DELAY(cnt) _nop_()

#define  _nop_()  _nop_()   /* 定义空指令 */


/* I2C, photoresistor */
sbit    I2C_SDA =  P2^0; //I2C  数据
sbit    I2C_SCLK = P2^1; //I2C  时钟

/* 8位数码管 */
sbit NIX_8NUM_LS138A = P2^2;
sbit NIX_8NUM_LS138B = P2^3;
sbit NIX_8NUM_LS138C = P2^4;
#define NIX_8NUM_DATA P0

/* lcd pin */
sbit LCD_WR = P2^5;	 //WR  引脚定义
sbit LCD_RD = P2^6;	 //RD  引脚定义
sbit LCD_CS = P2^7;	 //CS  引脚定义

sbit LCD_RS = P3^2;	 //RS  引脚定义
sbit LCD_RST = P3^3;    //RST 引脚定义
#define LCD_DATA P0

/* clock */
sbit CLK_SDA = P3^4;	//数据
sbit CLK_RST = P3^5;// DS1302复位
sbit CLK_SCK = P3^6;	//时钟
/* temperature pin */
sbit D18B20 = P3^7;

/* beep */
sbit BEEP_CTRL =  P1^7 ;

#elif defined(_CPU_ARM_STM32F103C8_)
#define CPU_DELAY(cnt) Sys_delay_cpu_opr(cnt);
#define  _nop_()       Sys_delay_cpu_us(2)


#define GPIO_SET_BIT(TYPE, ID) do {\
    GPIO_SetBits((TYPE), (ID));\
} while (0);
#define GPIO_RESET_BIT(TYPE, ID) do {\
    GPIO_ResetBits((TYPE), (ID));\
} while (0);
#define GPIO_GET_BIT(TYPE, ID) GPIO_ReadInputDataBit((TYPE), (ID))

#define GPIO_WRITE(TYPE, ID, VALUE) do {\
    (0 == (VALUE)) ? GPIO_ResetBits((TYPE), (ID)) : GPIO_SetBits((TYPE), (ID));\
} while (0);

/* Serial */
#define USART1_RXD              GPIO_Pin_10 /*PA10*/
#define USART1_TXD              GPIO_Pin_9 /*PA9*/

/* I2C, photoresistor */
#define I2C_SDA                 GPIO_Pin_7  /*PB7*/
#define I2C_SCLK                GPIO_Pin_6  /*PB6*/

/* 8位数码管 */
#define NIX_8NUM_LS138A         GPIO_Pin_5
#define NIX_8NUM_LS138B         GPIO_Pin_4
#define NIX_8NUM_LS138C         GPIO_Pin_3

#define NIX_8NUM_DATA           GPIO_Pin_All

/* lcd pin */

#define LCD_WR                  GPIO_Pin_2  /*GPIOB*/
#define LCD_RD                  GPIO_Pin_1  /*GPIOB*/
#define LCD_CS                  GPIO_Pin_0  /*GPIOB*/
#define LCD_RS                  GPIO_Pin_8  /*GPIOA*/
#define LCD_RST                 GPIO_Pin_11  /*GPIOA*/
#define LCD_DATA                GPIO_Pin_All /*GPIOx->BSRR GPIOx->BRR*/

/* clock */
#define CLK_SDA                 GPIO_Pin_12 /*PA12*/
#define CLK_RST                 GPIO_Pin_13 /*PA13*/
#define CLK_SCK                 GPIO_Pin_14 /*PA14*/

/* temperature pin */
#define D18B20                  GPIO_Pin_15 /*PA15*/

/* beep */
#define BEEP_CTRL               GPIO_Pin_7
#else
#error "INVALID CPU TYPE"
#endif

#define OS_INFO_GAP "--------------------------\n\r"


extern void Sys_printf(UCHAR8 *fmt, ...);
extern void Sys_set_print_color(USHORT16 fcolor, USHORT16 bcolor);
extern void Sys_timer_init(void);
extern void Sys_delay_cpu_opr(ULONG32 nCount);
extern void Sys_delay_cpu_us(ULONG32 nTime);
extern void Sys_delay_cpu_ms(u32 nTime);
extern void Sys_bsp_init(void);
extern void Sys_get_mcu_id(ULONG32 aulMCUId[3]);
extern void Sys_display_evn(void);
#endif/*#ifndef _COMMON_FUNC_H_*/
