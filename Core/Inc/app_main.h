/* app_main.h */
#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#define MAX_Fre_Sam 768000
#define MIN_Fre_Sam 256

#define ADCDataLength 256
#define BaseVol 3.3

#define OLED_Width  128
#define OLED_Height 64

#include "main.h" 
#include "FFT.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

//按键状态变量
extern volatile uint8_t SHOW;
extern volatile uint8_t STOP;
extern volatile uint8_t SPEC;
extern volatile uint8_t RIGHT;
extern volatile uint8_t CODE_Choice;

extern uint16_t ADCData[ADCDataLength];//原始ADC数据
extern float VOL[ADCDataLength];        //转换成浮点电压值
extern complex data[ADCDataLength];     //复数数组 complex
extern volatile uint8_t ADC_Sign;       //采集是否完成标志位

extern float Fre_Sam;     // 初始采样率 2560Hz
extern float MAX;           // FFT计算得到的最大幅值
extern uint16_t index_MAX; // 索引号
extern float Fre;          // 测量得到的频率

extern float Point;        // 每个波形周期的点数
extern float WaveNum;      // 屏幕上显示的波形周期数
extern float VPP;          // 垂直幅度
extern float MIN_WAVE;     // 最小电压值

void App_Main_Init(void);   //初始化      
float Set_SamplingFre_HAL(float fs);                   //设置采样频率
float Convert_ADC_To_Actual_Voltage(float vadc);       //偏置电压转换
void MY_Get_Vol_HAL(float vol_data[ADCDataLength]);    //采集电压数据
void OLED_Show_Wave_HAL(void);      //波形显示
void OLED_Show_Date(float fre_sam_wave); //数据显示
void OLED_Show_SPEC(float fre_sam_wave); //频谱显示
float Obtain_Fre( void );                     //获取频率
void Right_Fre( void );                       //自动调节频率
uint8_t Judge_Wave( void );                   //判断波形类型
void Printf(int i, char *format, ... );       //打印行数
void App_Main_Loop(void);                     //主循环

#endif
