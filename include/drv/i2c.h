#ifndef _I2C_H
#define _I2C_H
#include <common_func.h>


extern bit I2C_ack_bitflag;
//�����ߺ���
extern void Start_I2c(void);
//�������ߺ���  
extern void Stop_I2c(void);
//Ӧ���Ӻ���
extern void Ack_I2c(bit a);
//�ֽ����ݷ��ͺ���
extern void  SendByte(UCHAR8  c);
//���ӵ�ַ���Ͷ��ֽ����ݺ���               
extern bit ISendStr(UCHAR8 sla,UCHAR8 suba,UCHAR8 *s,UCHAR8 no) ;
//���ӵ�ַ���Ͷ��ֽ����ݺ���   
extern bit ISendStrExt(UCHAR8 sla,UCHAR8 *s,UCHAR8 no);
//���ӵ�ַ���ֽ����ݺ���               
extern UCHAR8 RcvByte(void);

#endif/*#ifndef _I2C_H*/
            
