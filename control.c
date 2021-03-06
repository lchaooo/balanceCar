/*
 * control.c
 *
 *  Created on: 2017年6月18日
 *      Author: lchao
 */

#include "control.h"
#include "filter.h"
#include "MPU9150.h"
#include <math.h>
#include "varible.h"
#include "motor.h"
#include <stdio.h>
#include <msp430.h>
#include "I2C.h"

volatile int Balance_Pwm,Velocity_Pwm,Turn_Pwm;
volatile float accel[3]={0};
volatile float gyro[3]={0};

u8 Flag_Target;
u32 Flash_R_Count;
int Voltage_Temp,Voltage_Count,Voltage_All;


static unsigned char txData[] =              // Table of data to transmit
{0,0,0};

//char temp[20] = {0};

//#pragma vector=PORT2_VECTOR
//__interrupt
//void Port_2(void)
//{
//	P2IE &= ~BIT4;
//	switch(P2IFG)
//	{
//	case BIT4:
//		P8OUT ^= BIT1;
//		getAngle();
//		Balance_Pwm =balance(Angle_Balance,Gyro_Balance);
//		Moto1=Balance_Pwm;
//		Moto2=Balance_Pwm;
//		Xianfu_Pwm();
//		if(angle > 0) P8OUT |= BIT2;
//		else P8OUT &= ~BIT2;
//		if(angle > - 15 && angle < 15)
//		{
//			int a = (int)angle;
//			P1OUT &= ~(BIT1+BIT2+BIT3+BIT4);
//			P1OUT |= a << 1;
//		}
//	    setPwm(Moto1/72,Moto2/72);
//		break;
//	default:
//		break;
//	}
//	P2IFG &= 0x0;
//	P2IE |= BIT4;
//}

int balance(float Angle,float Gyro)
{
	float Bias;
	int balance;
	Bias=Angle-ZHONGZHI;

	balance=Balance_Kp*Bias+Gyro*Balance_Kd;
	__no_operation();
	return balance;
}

int velocity(int encoder_left,int encoder_right)
{
	static float Velocity,Encoder_Least,Encoder;
	static float Encoder_Integral;
	Encoder_Least =(Encoder_Left+Encoder_Right)-0;
	Encoder *= 0.8;
	Encoder += Encoder_Least*0.2;
	Encoder_Integral +=Encoder;
	//Encoder_Integral=Encoder_Integral-Movement;
	if(Encoder_Integral>10000)  	Encoder_Integral=10000;
	if(Encoder_Integral<-10000)	Encoder_Integral=-10000;
	Velocity=Encoder*Velocity_Kp+Encoder_Integral*Velocity_Ki;
//	if(Turn_Off(Angle_Balance,Voltage)==1||Flag_Stop==1)   Encoder_Integral=0;
	return Velocity;
}

int turn(int encoder_left,int encoder_right,float gyro)
{
	static float Turn_Target,Turn,Encoder_temp,Turn_Convert=0.9,Turn_Count;
	Turn=-Turn_Target*Turn_Kp -gyro*Turn_Kd;
	return Turn;
}

//void Xianfu_Pwm(volatile int *leftPwm, volatile int *rightPwm)
//{
//	int Amplitude=6900;    //===PWMÂú·ùÊÇ7200 ÏÞÖÆÔÚ6900
//	if(Flag_Qian==1)  Moto1-=DIFFERENCE;  //DIFFERENCEÊÇÒ»¸öºâÁ¿Æ½ºâÐ¡³µµç»úºÍ»úÐµ°²×°²îÒìµÄÒ»¸ö±äÁ¿¡£Ö±½Ó×÷ÓÃÓÚÊä³ö£¬ÈÃÐ¡³µ¾ßÓÐ¸üºÃµÄÒ»ÖÂÐÔ¡£
//	if(Flag_Hou==1)   Moto2+=DIFFERENCE;
//	if(*rightPwm<-Amplitude) Moto2=-Amplitude;
//	else if(*rightPwm>Amplitude)  Moto2=Amplitude;
//    if(*leftPwm<-Amplitude) Moto1=-Amplitude;
//    else if(*leftPwm>Amplitude)  Moto1=Amplitude;


//}

void getAngle(volatile float *Gyro_Balance, volatile float *Angle_Balance, volatile float *Gyro_Turn, volatile float *Acceleration_Z)
{
	unsigned char buf[6];
//	MPU6050_ReadData(MPU6050_GYRO_OUT,buf,6);
	__no_operation();
	txData[0]=MPU6050_GYRO_OUT;
	sendI2C(txData,1,NO_STOP);
	__no_operation();//0x0078
	readI2CBytes(6,buf);
	__no_operation();//0x014B
	gyro[0] = (buf[0] << 8) + buf[1];
	gyro[1] = (buf[2] << 8) + buf[3];
	gyro[2] = (buf[4] << 8) + buf[5];
//	MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);
	__no_operation();
	txData[0]=MPU6050_ACC_OUT;
	sendI2C(txData,1,NO_STOP);
	__no_operation();
	readI2CBytes(6,buf);
	__no_operation();
	accel[0] = (buf[0] << 8) + buf[1];
	accel[1] = (buf[2] << 8) + buf[3];
	accel[2] = (buf[4] << 8) + buf[5];
   	//readMPU6050GyroFloat(gyro);
   	//readMPU6050AccFloat(accel);
	__no_operation();
	if(gyro[1]>32768)  gyro[1]-=65536;
	if(gyro[2]>32768)  gyro[2]-=65536;
	if(accel[0]>32768) accel[0]-=65536;
	if(accel[2]>32768) accel[2]-=65536;
	__no_operation();//0x004B
	*Gyro_Balance=-gyro[1];
	__no_operation();//0X8BA9
	accel[1]=atan2(accel[0],accel[2])*180/PI;
	__no_operation();//0xb000
	gyro[1]=gyro[1]/16.4;
	__no_operation();
	*Angle_Balance=Yijielvbo(accel[1],-gyro[1]);
	__no_operation();//¸üÐÂÆ½ºâÇã½Ç
	*Gyro_Turn=gyro[2];
	__no_operation();//¸üÐÂ×ªÏò½ÇËÙ¶È
	*Acceleration_Z=accel[2];
	__no_operation();
}


