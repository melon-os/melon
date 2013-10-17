#include <drv/lcd.h>
#include <drv/ir.h>
#include <drv/serial.h>
#include <drv/speaker.h>
#include <common_func.h>

static UCHAR8 g_system_printf_buf[SYS_PRINT_STR_MAX_LEN] = {0};
static PRT_COLOR_E g_system_print_fcolor = white;
PRT_COLOR_E g_system_print_bcolor = black;
volatile ULONG32 __sys_timer2_delay_cnt = 0;

#if defined(_CPU_ARM_STM32F103C8_)

void Sys_delay_cpu_opr(ULONG32 nCount)
{
  for(; nCount != 0; nCount--);
}

void Sys_timer_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);


    /*
        ((1+TIM_Prescaler )/72M)*(1+TIM_Period )=((1+35999)/72M)*(1+2000)=1��
        TIM_Prescaler������������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ��
        ����ȡֵ������0x0000��0xFFFF֮�䡣
    */
    TIM_TimeBaseStructure.TIM_Prescaler = 36-1 ;/* 72M/36 = 2Mhz, 0.5us*/
    /*
        TIM_Period����������һ�������¼�װ�����Զ���װ�ؼĴ���
        ���ڵ�ֵ������ȡֵ������0x0000��0xFFFF֮�䡣
    */
    TIM_TimeBaseStructure.TIM_Period = 1; /* n+1, 2*0.5us */
    /* TIM_ClockDivision������ʱ�ӷָ*/
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    /* TIM_CounterModeѡ���˼�����ģʽ */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    //��ֹARRԤװ�ػ�����
    //TIM_ARRPreloadConfig(TIM2, DISABLE);

    /* Clear the TIM2 Capture Compare 1 flag */
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    /* Disable the TIM2 counter, enable when user call the Sys_delay_cpu_us */
    TIM_Cmd(TIM2, DISABLE);
}

void System_timer2_int_handler(void)
{
    if (0 != __sys_timer2_delay_cnt)
    {
        __sys_timer2_delay_cnt--;
    }
    /* Clear the TIM2 Capture Compare 1 flag */
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void Sys_delay_cpu_ms(u32 nTime)
{
#ifndef STM32F103C8_SUPPORT_TIM2
        ULONG32 ulIdx = 9*nTime*1000;
        while(ulIdx--);
#else
        __sys_timer2_delay_cnt = nTime*1000;

        TIM_Cmd(TIM2, ENABLE);
        /* Enables the TIM2 Capture Compare channel 1 Interrupt source */
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

        while (0 != __sys_timer2_delay_cnt)
        {
            /* do nothing here, wait for
                TIM2_Handler decrease __sys_timer2_delay_cnt
            */
        }
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM2, DISABLE);
#endif
}
void Sys_delay_cpu_us(u32 nTime)
{
#ifndef STM32F103C8_SUPPORT_TIM2
    UCHAR8 ucIdx = 9;
    while(nTime--)
    {
        while(ucIdx--);
        ucIdx = 9;
    }

#else
    __sys_timer2_delay_cnt = nTime;

    TIM_Cmd(TIM2, ENABLE);
    /* Enables the TIM2 Capture Compare channel 1 Interrupt source */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    while (0 != __sys_timer2_delay_cnt)
    {
        /* do nothing here, wait for
            TIM2_Handler decrease __sys_timer2_delay_cnt
        */
    }
    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM2, DISABLE);
#endif
}

static void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;

    //��λRCC�ⲿ�豸�Ĵ�����Ĭ��ֵ
    RCC_DeInit();

    //���ⲿ���پ���
    RCC_HSEConfig(RCC_HSE_ON);

    //�ȴ��ⲿ����ʱ��׼����
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)   //�ⲿ����ʱ���Ѿ�׼���
    {
        //����FLASH��Ԥȡ����
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        //FLASH�ӳ�2������
        FLASH_SetLatency(FLASH_Latency_2);

        //����AHB(HCLK)ʱ��=SYSCLK
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        //����APB2(PCLK2)��=AHBʱ��
        RCC_PCLK2Config(RCC_HCLK_Div1);

        //����APB1(PCLK1)��=AHB 1/2ʱ��
        RCC_PCLK1Config(RCC_HCLK_Div2);

        //����PLLʱ�� == �ⲿ���پ���ʱ��*9  PLLCLK = 8MHz * 9 = 72 MHz
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

        //ʹ��PLLʱ��
        RCC_PLLCmd(ENABLE);

        //�ȴ�PLLʱ�Ӿ���
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

        //����ϵͳʱ�� = PLLʱ��
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        //���PLLʱ���Ƿ���Ϊϵͳʱ��
        while(RCC_GetSYSCLKSource() != 0x08);

    }
}

/*******************************************************************************
*                             NVIC���ú���
*******************************************************************************/
static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif


    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //����GPIOģ��ʱ��
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB
                            | RCC_APB2Periph_AFIO,
                                    ENABLE);

   //�ѵ���������ͨIO��
   GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;   //����GPIOΪͬһ���Ͷ˿�
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //��������Ƶ��Ϊ50HZ
   GPIO_Init(GPIOA, &GPIO_InitStructure);   //��ʼ��GPIOA�˿�
   GPIO_Init(GPIOB, &GPIO_InitStructure);   //��ʼ��GPIOB�˿�

   GPIO_Write(GPIOA,0xffff);  //��GPIOA 16���˿�ȫ����Ϊ�ߵ�ƽ
   GPIO_Write(GPIOB,0xffff);  //��GPIOB 16���˿�ȫ����Ϊ�ߵ�ƽ

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;    //����GPIOΪͬһ���Ͷ˿�
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //��������Ƶ��Ϊ50HZ
   GPIO_Init(GPIOA, &GPIO_InitStructure);   //��ʼ��GPIOA�˿�

   /* CLK ��DATA�˿� */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;    //����GPIOΪͬһ���Ͷ˿�
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //��������Ƶ��Ϊ50HZ
   GPIO_Init(GPIOA, &GPIO_InitStructure);   //��ʼ��GPIOA�˿�

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //��������Ƶ��Ϊ50HZ
   GPIO_Init(GPIOB, &GPIO_InitStructure);   //��ʼ��GPIOA�˿�
}
void Sys_bsp_init(void)
{
    RCC_Configuration();/*ϵͳʱ�����ú���*/
    GPIO_Configuration();
    NVIC_Configuration();/* �жϳ�ʼ�� */
#ifdef STM32F103C8_SUPPORT_TIM2
    Sys_timer_init();
#endif    
}

/*
    STM32F10xxxϵ��MCU�ڲ�����һ���������̻���96bitΨһʶ��ID
    ��ID��������оƬ���ܡ��豸ʶ���һ������Ӧ�á�
    ��ȡ��ID�ķ���
    u32 DevID;
    DevID = *(ULONG32*)(0x1ffff7e8);
    DevID = *(ULONG32*)(0x1ffff7ec);
    DevID = *(ULONG32*)(0x1ffff7f0);
*/
void Sys_get_mcu_id(ULONG32 aulMCUId[3])
{
    aulMCUId[0] = *(ULONG32*)(0x1ffff7e8);
    aulMCUId[1] = *(ULONG32*)(0x1ffff7ec);
    aulMCUId[2] = *(ULONG32*)(0x1ffff7f0);
    return;
}

#elif defined(_CPU_ARM_STM32F103C8_)
/*C51��Ҫʵ����Щ����*/
#else
#error "INVALID CPU TYPE"
#endif

void Sys_set_print_color(PRT_COLOR_E fcolor, PRT_COLOR_E bcolor)
{
    g_system_print_fcolor = (PRT_COLOR_E)fcolor;
    g_system_print_bcolor = (PRT_COLOR_E)bcolor;
    Serial_set_color(fcolor);
    LCD_set_forColor(g_system_print_fcolor);
    LCD_set_bgColor(g_system_print_bcolor);
}

void Sys_printf(UCHAR8 *fmt, ...)
{
    va_list ap;

    va_start(ap,fmt);
    vsprintf((char*)g_system_printf_buf, (char*)fmt, ap);
    Serial_print_string(g_system_printf_buf);
    LCD_printf_color(g_system_printf_buf, g_system_print_fcolor, g_system_print_bcolor);
    va_end(ap);
}

/*****************************************************************************
 Prototype    : Sys_display_evn 
 Input        : void
 Output       : None
 Return Value : void
 Calls        :
 Description  : Display the environment information

*****************************************************************************/
void Sys_display_evn(void)
{
    USHORT16 data_size = 0;
    //ULONG32 aulMCUId[3] = {0};

    //Sys_printf("CPU Serial No->\r\n");
    //Sys_get_mcu_id(aulMCUId);
    //Sys_printf("%u\n%u\n%u\r\n", aulMCUId[0],aulMCUId[1], aulMCUId[2]);
    Sys_printf(OS_INFO_GAP);
    Sys_printf("Compile Info:\r\n");
    Sys_printf("\t%s\r\n",__DATE__);
    Sys_printf("\t%s\r\n",__TIME__);
    Sys_printf(OS_INFO_GAP);
    Sys_printf("CPU Info\r\n");
    Sys_printf(OS_INFO_GAP);
    data_size = 0x1234;
    if (0x12 == *((UCHAR8*)(&data_size)))
    {
        Sys_printf("big endian\r\n");
    }
    else if (0x34 == *((UCHAR8*)(&data_size)))
    {
        Sys_printf("little endian\r\n");
    }
    else
    {
        Sys_printf("Error!!!\r\n");
    }

    data_size = sizeof(short);
    Sys_printf("short\t\t:%d\r\n", data_size);
    data_size = sizeof(int);
    Sys_printf("int\t\t:%d\r\n", data_size);
    data_size = sizeof(long);
    Sys_printf("long\t\t:%d\r\n", data_size);
    data_size = sizeof(float);
    Sys_printf("float\t\t:%d\r\n", data_size);
    data_size = sizeof(double);
    Sys_printf("double\t\t:%d\r\n", data_size);
    data_size = sizeof(long long);
    Sys_printf("long long\t:%d\r\n", data_size);
    Sys_printf(OS_INFO_GAP);
    Sys_delay_cpu_ms(3000);
    Sys_printf("\r\n\r\n");
}

