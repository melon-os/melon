/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : serial.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Serial source file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/


#include <drv/serial.h>
#include <drv/clock.h>
#include <drv/lcd.h>


volatile UCHAR8  g_RxDataBuf[SERIAL_RXBUF_SIZE] = {0};
volatile UCHAR8  *pucRxDataBuf = NULL;
volatile USHORT16  g_RxBufIndex_R = 0;
volatile USHORT16  g_RxBufIndex_W = 0;

volatile UCHAR8  g_TxDataBuf[SERIAL_TXBUF_SIZE] = {0};
volatile UCHAR8  *pucTxDataBuf = NULL;
volatile USHORT16  g_TxBufIndex_R = 0;
volatile USHORT16  g_TxBufIndex_W = 0;

UCHAR8 g_serial_printf_buf[SERIAL_PRINT_STR_MAX_LEN] = {0};

UCHAR8 Serial_init(SERIAL_SPEED_MODE_E eSpeedMode)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_InitClockStruct;

    /* DISABLE the USART1 */
    USART_Cmd(USART1, DISABLE);

    g_RxBufIndex_R = 0;
    g_RxBufIndex_W = 0;
    pucRxDataBuf = g_RxDataBuf;

    g_TxBufIndex_R = 0;
    g_TxBufIndex_W = 0;
    pucTxDataBuf = g_TxDataBuf;

    /* Enable USART1, GPIOA, GPIOx and AFIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);


    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = USART1_RXD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART1_TXD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    USART_InitStructure.USART_BaudRate = (UINT32)eSpeedMode;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   
    /* Configure USART1 */
    USART_Init(USART1, &USART_InitStructure);

    USART_InitClockStruct.USART_Clock = USART_Clock_Disable;
    USART_InitClockStruct.USART_CPHA = USART_CPHA_2Edge;
    USART_InitClockStruct.USART_CPOL = USART_CPOL_Low;
    USART_InitClockStruct.USART_LastBit = USART_LastBit_Enable;
    USART_ClockInit(USART1, &USART_InitClockStruct);

    
    USART_ClearITPendingBit(USART1, USART_IT_TXE);
    USART_ClearFlag(USART1, USART_FLAG_TXE);
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    USART_ClearFlag(USART1, USART_FLAG_RXNE);
    USART_ReceiveData(USART1);

    /* Enable USART1 Receive and disable Transmit interrupts */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    /* Enable the USART1 */
    USART_Cmd(USART1, ENABLE);

    return ERR_CODE_OK;
}

UCHAR8 Serial_write_char(const UCHAR8 ucSendData)
{    
    ENTER_CRITICAL_SECTION();

    if ((g_TxBufIndex_W + 1) % SERIAL_TXBUF_SIZE == g_TxBufIndex_R)
    {
        EXIT_CRITICAL_SECTION();
        return ERR_CODE_OVERFLOW;
    }

    pucTxDataBuf[g_TxBufIndex_W] = ucSendData;
    g_TxBufIndex_W++;
    g_TxBufIndex_W = g_TxBufIndex_W % SERIAL_TXBUF_SIZE;    
    
    EXIT_CRITICAL_SECTION();
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    
    return ERR_CODE_OK;
}

UCHAR8 Serial_read_char(UCHAR8 *pucSendData)
{
    if (NULL == pucSendData)
    {
        return ERR_CODE_NULL_POINTER;
    }
    
    ENTER_CRITICAL_SECTION();
    if (g_RxBufIndex_R == g_RxBufIndex_W)
    {
        EXIT_CRITICAL_SECTION();
        return ERR_CODE_UNDERFLOW;
    }

    *pucSendData = pucRxDataBuf[g_RxBufIndex_R];
    g_RxBufIndex_R++;
    g_RxBufIndex_R = g_RxBufIndex_R % SERIAL_RXBUF_SIZE;

    EXIT_CRITICAL_SECTION();
    
    return ERR_CODE_OK;
}

int fputc(int ch, FILE *f)
{
   while (ERR_CODE_OK != Serial_write_char(ch))
   {
        OS_task_sleep(50);
   }
   return ch;
}

int sgetc(void)
{
    UCHAR8 ucData = 0;
    /* Loop until received a char */
    while (ERR_CODE_OK != Serial_read_char(&ucData))
    {
        OS_task_sleep(200);
    }
    return ucData;
}


UCHAR8 Serial_print_string(UCHAR8 *pcBuf)
{    
    if (NULL == pcBuf)
    {
        return ERR_CODE_NULL_POINTER;
    }
    
    if (EOC != pcBuf[SERIAL_PRINT_STR_MAX_LEN - 1])
    {
        pcBuf[SERIAL_PRINT_STR_MAX_LEN - 1] = EOC;        
    }

    while (EOC != *pcBuf)
    {
        while(ERR_CODE_OK != Serial_write_char(*pcBuf))
        {
            //if (OS_STATE_RUN == OS_get_state())
            {
                //OS_task_sleep(1);
            }
            //else
            {
                //Sys_delay_cpu_ms(1);
            }            
        }
        pcBuf++;
    }
    
    return ERR_CODE_OK;
}

void Serial_printf(const UCHAR8* fmt, ...)
{
    va_list ap;    
    va_start(ap,fmt);     
    vsprintf((char*)g_serial_printf_buf, (const char*)fmt,ap);
    Serial_print_string(g_serial_printf_buf);
    va_end(ap);
}

/*****************************************************************************
 Prototype    : Serial_set_color
 Description  : Set print color in console
 Input        : PRT_COLOR_E
 Output       : None
 Return Value : OS_RET_CODE
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2013/7/13
    Author       : QQ 381624054
    Modification : Created function

*****************************************************************************/
OS_RET_CODE Serial_set_color(PRT_COLOR_E color)
{
    switch (color)
    {
     case red:
     case pink:
         Serial_printf(VT_RED);
         break;
     case green:
         Serial_printf(VT_GREEN);
         break;
     case blue:
         Serial_printf(VT_BLUE);
         break;
     case yellow:
     case brown:
         Serial_printf(VT_YELLOW);
         break;
     case black:
         Serial_printf(VT_BLACK);
         break;
     case white:
         Serial_printf(VT_WHITE);
         break;
     case transp:
         Serial_printf(VT_DEFAULT);
         break;
     case underline:
         Serial_printf(VT_UNDERLINE);
     default:
         return OS_RET_PARAM;
    }

    return OS_RET_SUCCESS;
}


/****************************************************
               ´®¿ÚÖÐ¶Ï³ÌÐò
******************************************************/
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        if ((g_RxBufIndex_W + 1) % SERIAL_RXBUF_SIZE != g_RxBufIndex_R)
        {
            pucRxDataBuf[g_RxBufIndex_W] = USART_ReceiveData(USART1);
            g_RxBufIndex_W++;
            g_RxBufIndex_W = g_RxBufIndex_W % SERIAL_RXBUF_SIZE;
        }
        else
        {         
            /* overflow, do nothing, */
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        USART_ClearFlag(USART1, USART_FLAG_RXNE);
        //USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    }

    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_TXE);
        USART_ClearFlag(USART1, USART_FLAG_TXE);
        
        if (g_TxBufIndex_R != g_TxBufIndex_W)
        {
            /* Write one byte to the transmit data register */
            USART_SendData(USART1, pucTxDataBuf[g_TxBufIndex_R]);           
            g_TxBufIndex_R++;
            g_TxBufIndex_R = g_TxBufIndex_R % SERIAL_TXBUF_SIZE;
        }   
        else
        {
            /* underflow, disable inter */
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }
}

