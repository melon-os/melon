
/*************************�˲���ΪI2C���ߵ���������*************************************/

#include <drv/i2c.h>

bit I2C_ack_bitflag;                 /*Ӧ���־λ*/   

/*******************************************************************
                     �����ߺ���               
����ԭ��: void  Start_I2c();  
����:     ����I2C����,������I2C��ʼ����.  
********************************************************************/
void Start_I2c(void)
{
  //I2C_SDA=1;         /*������ʼ�����������ź�*/
  GPIO_SET_BIT(GPIOB, I2C_SDA);
  _nop_();
  //I2C_SCLK=1;
  GPIO_SET_BIT(GPIOB, I2C_SCLK);
  _nop_();        /*��ʼ��������ʱ�����4.7us,��ʱ*/
  _nop_();
  _nop_();     
  //I2C_SDA=0;         /*������ʼ�ź�*/
  GPIO_RESET_BIT(GPIOB, I2C_SDA);
  _nop_();        /* ��ʼ��������ʱ�����4��s*/
  _nop_();
  _nop_();        
  //I2C_SCLK=0;       /*ǯסI2C���ߣ�׼�����ͻ�������� */
  GPIO_RESET_BIT(GPIOB, I2C_SCLK);
  _nop_(); 
}

/*******************************************************************
                      �������ߺ���               
����ԭ��: void  Stop_I2c();  
����:     ����I2C����,������I2C��������.  
********************************************************************/
void Stop_I2c(void)
{
  //I2C_SDA=0;      /*���ͽ��������������ź�*/
  GPIO_RESET_BIT(GPIOB, I2C_SDA);
  _nop_();       /*���ͽ���������ʱ���ź�*/
  //I2C_SCLK=1;      /*������������ʱ�����4��s*/
  GPIO_SET_BIT(GPIOB, I2C_SCLK);
  _nop_();
  _nop_();
  _nop_(); 
 // I2C_SDA=1;      /*����I2C���߽����ź�*/
  GPIO_SET_BIT(GPIOB, I2C_SDA);
  _nop_();
  _nop_();  
}

/*******************************************************************
                 �ֽ����ݷ��ͺ���               
����ԭ��: void  SendByte(UCHAR c);
����:     ������c���ͳ�ȥ,�����ǵ�ַ,Ҳ����������,�����ȴ�Ӧ��,����
          ��״̬λ���в���.(��Ӧ����Ӧ��ʹack=0)     
           ��������������ack=1; I2C_ack_bitflag=0��ʾ��������Ӧ����𻵡�
********************************************************************/
void  SendByte(UCHAR8  c)
{
    UCHAR8  BitCnt;

    for(BitCnt=0;BitCnt<8;BitCnt++)  /*Ҫ���͵����ݳ���Ϊ8λ*/
    {
        if((c<<BitCnt)&0x80)   /*�жϷ���λ*/
        {  
            //I2C_SDA=1;
            GPIO_SET_BIT(GPIOB, I2C_SDA);
        }
        else
        {
           // I2C_SDA=0;
            GPIO_RESET_BIT(GPIOB, I2C_SDA);
        }
        _nop_();
        //I2C_SCLK=1;               /*��ʱ����Ϊ�ߣ�֪ͨ��������ʼ��������λ*/
        GPIO_SET_BIT(GPIOB, I2C_SCLK);
        _nop_(); 
        _nop_();             /*��֤ʱ�Ӹߵ�ƽ���ڴ���4��s*/
        _nop_();            
       // I2C_SCLK=0; 
        GPIO_RESET_BIT(GPIOB, I2C_SCLK);
    }
    
    _nop_();
    //I2C_SDA=1;                /*8λ��������ͷ������ߣ�׼������Ӧ��λ*/
    GPIO_SET_BIT(GPIOB, I2C_SDA);
    _nop_(); 
    //I2C_SCLK=1;
    GPIO_SET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    _nop_();
    //if(I2C_SDA==1)/*�ж��Ƿ���յ�Ӧ���ź�*/
    if (1 == GPIO_GET_BIT(GPIOB, I2C_SDA))
    {
        I2C_ack_bitflag=0;  
    }
    else
    {
        I2C_ack_bitflag=1;
    }
    //I2C_SCLK=0;
    GPIO_RESET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    _nop_();
}

/*******************************************************************
                 �ֽ����ݽ��պ���               
����ԭ��: UCHAR  RcvByte();
����:        �������մ���������������,���ж����ߴ���(����Ӧ���ź�)��
          ���������Ӧ����Ӧ��ӻ���  
********************************************************************/    
UCHAR8   RcvByte(void)
{
    UCHAR8  retc;
    UCHAR8  BitCnt;

    retc=0; 
    //I2C_SDA=1;                     /*��������Ϊ���뷽ʽ*/
    GPIO_SET_BIT(GPIOB, I2C_SDA);
    for(BitCnt=0;BitCnt<8;BitCnt++)
    {
        _nop_();           
        //I2C_SCLK=0;                  /*��ʱ����Ϊ�ͣ�׼����������λ*/
        GPIO_RESET_BIT(GPIOB, I2C_SCLK);
        _nop_();
        _nop_();                 /*ʱ�ӵ͵�ƽ���ڴ���4.7��s*/
        _nop_();
        //I2C_SCLK=1;                  /*��ʱ����Ϊ��ʹ��������������Ч*/
        GPIO_SET_BIT(GPIOB, I2C_SCLK);

        _nop_();
        retc=retc<<1;
        //if(I2C_SDA==1)
        if (1 == GPIO_GET_BIT(GPIOB, I2C_SDA))
        {
        retc=retc+1;  /*������λ,���յ�����λ����retc�� */
        }
        _nop_();
    }
    //I2C_SCLK=0;
    GPIO_RESET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    return(retc);
}

/********************************************************************
                     Ӧ���Ӻ���
����ԭ��:  void Ack_I2c(bit a);
����:      ����������Ӧ���ź�(������Ӧ����Ӧ���źţ���λ����a����)
********************************************************************/
void Ack_I2c(bit a)
{

    if(a==0)
    { 
        //I2C_SDA=0;              /*�ڴ˷���Ӧ����Ӧ���ź� */
        GPIO_RESET_BIT(GPIOB, I2C_SDA);
    }
    else
    {
        //I2C_SDA=1;
        GPIO_SET_BIT(GPIOB, I2C_SDA);
    }
    _nop_();
    _nop_();    
    //I2C_SCLK=1;
    GPIO_SET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    _nop_();                    /*ʱ�ӵ͵�ƽ���ڴ���4��s*/
   
    //I2C_SCLK=0;                     /*��ʱ���ߣ�ǯסI2C�����Ա��������*/
    GPIO_RESET_BIT(GPIOB, I2C_SCLK);

    _nop_();
}
