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
/*һ������ӡ60���ַ�, ��LCD(����ӡ2��)ƥ��*/
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
printf("\033[1;40;32m%s\033[0m",�� Hello,NSFocus\n��); 
\033 ������ת�����еĿ�ʼ��Ȼ���� [ ��ʼ������ɫ������� 1 �����˸�����ʾ�ַ���Ȼ���Ǳ�����ɫ����������40
����ʾ��ɫ������������ǰ����ɫ����������32����ʾ��ɫ�������� \033[0m �ر�ת�����У� \033[0m ���ն�Ĭ����ɫ�� 
ͨ������Ľ��ܣ���֪������������ɫ�ַ��ˡ���ˣ��ҾͲ��ٶ�˵�ˡ������Ƕ��ڲ�ɫ�ַ���ɫ��һЩ���壺 
ǰ��            ����              ��ɫ
    ---------------------------------------
    30                40              ��ɫ
    31                41              �tɫ
    32                42              �Gɫ
    33                43              �Sɫ
    34                44              �{ɫ
    35                45              �ϼtɫ
    36                46              ���{ɫ
    37                47              ��ɫ 
    ����              ����
    -------------------------
    0                �ն�Ĭ������
    1                ������ʾ
    4                ʹ���»���
    5                ��˸
    7                ������ʾ
    8                ���ɼ�
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
