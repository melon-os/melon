#include <drv/ad.h>
#include <drv/i2c.h>

UCHAR8 AD_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    // ADC1 configuration -----------------------------
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);


    // ADC1 regular channel16 Temp Sensor configuration
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_55Cycles5);
    // Enable the temperature sensor and vref internal channel
    ADC_TempSensorVrefintCmd(DISABLE);
    ADC_Cmd(ADC1, ENABLE);

    return ERR_CODE_OK;
}

/*
   Vtemp_sensor = ADC_ConvertedValue * Vdd / Vdd_convert_value;
   Current_Temp = (V25 - Vtemp_sensor)/Avg_Slope + 25;
*/
UCHAR8 AD_get_temp_mcu_core(USHORT16 *pusADCValue)
{
    
    ADC_TempSensorVrefintCmd(ENABLE);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    Sys_delay_cpu_us(500);
    //*pusADCValue = ADC_GetConversionValue(ADC1);
    *pusADCValue = (USHORT16)(*((ULONG32*)ADC1_DR_Address));
    ADC_TempSensorVrefintCmd(DISABLE);
    ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    return ERR_CODE_OK;
}

/*******************************************************************
ADC发送字节[命令]数据函数               
*******************************************************************/
bit AD_SendByte(UCHAR8 sla,UCHAR8 c)
{
   Start_I2c();              //启动总线
   SendByte(sla);            //发送器件地址
   if(I2C_ack_bitflag==0)return(0);
   SendByte(c);              //发送数据
   if(I2C_ack_bitflag==0)return(0);
   Stop_I2c();               //结束总线
   return(1);
}

/*******************************************************************
ADC读字节数据函数               
*******************************************************************/
UCHAR8 AD_RcvByte(UCHAR8 sla)
{  
    UCHAR8 c;

    Start_I2c();          //启动总线
    SendByte(sla+1);      //发送器件地址
    if(I2C_ack_bitflag==0)
        return(0);
    
    c=RcvByte();          //读取数据0

    Ack_I2c(1);           //发送非就答位
    Stop_I2c();           //结束总线
    return(c);
}



//******************************************************************/
UCHAR8 AD_get_pr(USHORT16 *pusPrData)
{  
    //USHORT16 usAD_data_ch3 = 0;

    if (NULL == pusPrData)
    {
        return ERR_CODE_NULL_POINTER;
    }
#if 0
    AD_SendByte(PCF8591,0x40);
    AD_RcvByte(PCF8591);
    AD_SendByte(PCF8591,0x41);
    AD_RcvByte(PCF8591);
    AD_SendByte(PCF8591,0x42);
    AD_RcvByte(PCF8591);   
#endif
    /* AD ch2 */
    if (!AD_SendByte(PCF8591,AD_PR_CH_ADDR))
    {
        return ERR_CODE_FAIL;
    }
    *pusPrData = AD_RcvByte(PCF8591);

    return ERR_CODE_OK;
}

UCHAR8 AD_get_temp(USHORT16 *pusTemp2)
{

    if (NULL == pusTemp2)
    {
        return ERR_CODE_NULL_POINTER;
    }
#if 1
    AD_SendByte(PCF8591,0x40);
    AD_RcvByte(PCF8591);
    AD_SendByte(PCF8591,0x41);
    AD_RcvByte(PCF8591);
#endif
    /* AD ch2 */
    if (!AD_SendByte(PCF8591,AD_TEMP_CH_ADDR))
    {
        return ERR_CODE_FAIL;
    }
    
    *pusTemp2 = AD_RcvByte(PCF8591)*2;/*放大2倍显示*/   
    
    return ERR_CODE_OK;
}

