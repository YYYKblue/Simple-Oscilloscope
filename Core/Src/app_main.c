//该项目相关
#include "app_drivers.h"
#include "app_main.h"
#include "oled.h" 
#include "font.h" 
#include "FFT.h"
#include "tim.h"
#include "adc.h"
#include "boot_animation.h"

//c语言相关
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h> 

//按键状态变量
volatile uint8_t SHOW = 1;
volatile uint8_t STOP = 0;
volatile uint8_t SPEC = 0;
volatile uint8_t RIGHT = 2; 
volatile uint8_t CODE_Choice = 0;

uint16_t ADCData[ADCDataLength]; //采集的256个点的原始ADC数据
float VOL[ADCDataLength];        //转换成浮点电压值
complex data[ADCDataLength];     //复数数组 complex
volatile uint8_t ADC_Sign = 0;   //采集是否完成标志位

float Fre_Sam = 2560;  // 初始采样率 2560Hz 后续会根据频率更改
float MAX;           // FFT计算得到的最大幅值
uint16_t index_MAX; // 索引号
float Fre;         // 测量得到的频率

float Point = 16;     // 每个波形采集 16 个点
// float VPP = 3.3;      // 默认 Vpp 为 3.3V
// float MIN_WAVE = 0.0; // 默认最小电压为 0V
float VPP = 20.3;      // 默认 Vpp 为 20V
float MIN_WAVE = -9.9; // 默认最小电压为 -10V
//float VPP = 11.33;      // 默认 Vpp 为 20V
//float MIN_WAVE = -12.2; // 默认最小电压为 -10V
float WaveNum = 4;     // 每个波形采集 4 个周期

// 初始化函数
void App_Main_Init(void)
{
    HAL_Delay(20);
    OLED_Init();
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);  
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_ADCEx_Calibration_Start(&hadc1);  //adc校准
    HAL_Delay(20);
		OLED_PlayBootAnimation();   //shu初始化动画
		lightpb0(); //pb0常亮灯
}

// 设置采样频率
float Set_SamplingFre_HAL(float fs)
{
    uint32_t psc = 1;
    uint32_t arr = 1;

    HAL_TIM_Base_Stop(&htim3);

    do
    {
        arr = 72000000 / (fs * psc);
        psc++;
    } while (arr > 65535);
    psc--;

    fs = (float)72000000 / (psc * arr);

    __HAL_TIM_SET_AUTORELOAD(&htim3, arr - 1);
    __HAL_TIM_SET_PRESCALER(&htim3, psc - 1); 

    return fs;
}

float Convert_ADC_To_Actual_Voltage(float vadc)
{
    return (vadc - 1.61f) / 0.1625f;//基于实际电阻值进行计算
}



void MY_Get_Vol_HAL(float vol_data[ADCDataLength])
{
    ADC_Sign = 0; // 清除标志位
    
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADCData, ADCDataLength);
    HAL_TIM_Base_Start(&htim3);

    while (ADC_Sign == 0); // 阻塞等待dma获得256个点 中断采集完成adcsign=1

    for (uint32_t count = 0; count < ADCDataLength; count++)
    {
        // 1. 计算ADC引脚上的电压 (0-3.3V)
        float vadc = (float)(ADCData[count] & 0x0000ffff) / 0xfff * BaseVol;
        
        // 2. 将其转换为实际输入电压 (-10V 到 +10V) 并存入数组
        vol_data[count] = Convert_ADC_To_Actual_Voltage(vadc);
    }
}

void OLED_Show_Wave_HAL(void)
{
    OLED_NewFrame();   
    
    const float step = OLED_Width / (Point * WaveNum);
    float lastX = 0;
    float x = step;
    int begin = 0;
    
    // 计算触发电平
    const float mid = VPP / 2 + MIN_WAVE;
    
    // 安全的触发点查找
    for(int i = 0; i < ADCDataLength/2 - 3; i++)  // 防止越界
    {
        if(VOL[i] < mid && VOL[i + 1] < mid && 
           VOL[i + 2] > mid && VOL[i + 3] > mid)
        {
            begin = i;
            break;
        }
    }
    
    // 绘制波形（添加 Y 轴限制）
    float lastY = ((VOL[begin] - MIN_WAVE) / VPP * OLED_Height);
    lastY = fmaxf(0, fminf(lastY, OLED_Height));  // 限制 Y 范围
    
    const int end = (int)fminf(begin + OLED_Width/step, ADCDataLength);
    for(int i = begin; i < end; i++, x += step)
    {
        float newY = ((VOL[i] - MIN_WAVE) / VPP * OLED_Height);
        newY = fmaxf(0, fminf(newY, OLED_Height));
        
        OLED_DrawLine(lastX, 64 - lastY, x, 64 - newY, OLED_COLOR_NORMAL);
        
        lastX = x;
        lastY = newY;
    }
    
    OLED_ShowFrame();
}

void OLED_Show_Date( float fre_sam )
{
    OLED_NewFrame(); // 清空屏幕显存

    Printf(1,"Fre: %0.1lf            ",Fre);    // 打印频率
    float max = VOL[0],min = VOL[0],duty = 0;
    for(int i = 0;i<ADCDataLength/2;i++) // 计算 Vmax, Vmin
    {
        if(max < VOL[i]) max = VOL[i];
        if(min > VOL[i]) min = VOL[i];
    }
    Printf(2,"max: %0.3lf            ",max); // 打印 Vmax
    Printf(3,"min: %0.3lf            ",min); // 打印 Vmin
    Printf(4,"vpp: %0.3lf            ",max - min); // 打印 Vpp

    float mid = (max - min)/2 + min;
    for(int i = 0;i<ADCDataLength/2;i++) // 计算占空比
    {
        if(VOL[i] > mid) duty++;
    }
    Printf(5,"duty: %0.3lf                    ",duty * 2.0 / ADCDataLength); // 打印占空比

    switch (Judge_Wave())   // 判断并打印波形类型
    {
        case 0: Printf(6,"Sine                        "); break;
        case 1: Printf(6,"Triangular                   "); break;
        case 2: Printf(6,"Square wave                  "); break;
        case 3: Printf(6,"Sawtooth wave                "); break; 
    }

    Printf(7,"                               "); // 清空第 7 行
    Printf(8,"Fre_Sam:%0.1lf                  ",fre_sam); // 打印采样率

    OLED_ShowFrame(); // 将显存内容更新到屏幕
}

void OLED_Show_SPEC( float fre_sam )
{
    unsigned int count = 0;
    for (count = 0; count < ADCDataLength; count++)
    {
        data[count].real = VOL[count];
        data[count].imag = 0;
    }
    fft(ADCDataLength, data);
    for (count = 0; count < ADCDataLength; count++)
    {
        data[count].real = sqrt(data[count].real * data[count].real + data[count].imag * data[count].imag);
    }
    data[0].real = data[0].real / ADCDataLength; // 直流分量
    for (count = 1; count < ADCDataLength; count++)
    {
        data[count].real = data[count].real / ADCDataLength * 2; // 谐波分量
    }
    // fft计算完成

    OLED_NewFrame();   // 清空屏幕显存
    uint16_t lastX = 1;
    // y 坐标应基于计算后的 data[1].real，且需要缩放。原代码缩放系数为10
    // 同时 Y 轴原点在左上角，需要用 64 减去计算值
    // 限制Y坐标在屏幕内 (0-63)
    uint16_t lastY = 64 - fmin(data[1].real * 10, 63); // 乘以10是原代码的缩放, fmin 保证不超限

    // 在第一行打印频率信息
    Printf(1,"F=%0.0lf FS=%0.0lf",Fre,fre_sam); // Printf 不会自动更新

    // 绘制频谱图
    for(int i = 2; i < OLED_Width && i < ADCDataLength / 2; i++) // 只绘制到屏幕宽度或奈奎斯特频率
    {
        // 计算当前点的Y坐标，同样需要缩放和反转
        uint16_t currentY = 64 - fmin(data[i].real * 10, 63); // 乘以10是原代码的缩放, fmin 保证不超限

        // 绘制从上一个点到当前点的线段
        OLED_DrawLine(lastX, lastY, i, currentY, OLED_COLOR_NORMAL); 

        lastX = i;
        lastY = currentY;
    }
    OLED_ShowFrame();  // 将显存内容更新到屏幕
}

//获取输入信号的实际频率
float Obtain_Fre( void )
{
    float frequency = 0;
    unsigned int count = 0;

	Set_SamplingFre_HAL(Fre_Sam); //全局变量 fre_sam 
    MY_Get_Vol_HAL(VOL);          
    // 将电压数据转化为复数
    for (count = 0; count < ADCDataLength; count++)
    {
        data[count].real = VOL[count];  // 实部为电压值
        data[count].imag = 0;           // 虚部为0
    }
    fft(ADCDataLength, data);
    for (count = 0; count < ADCDataLength; count++)
    {
        VOL[count] = sqrt(data[count].real * data[count].real + data[count].imag * data[count].imag);//模值计算 
    }
    VOL[0] = VOL[0] / ADCDataLength;// 提取直流量 第一个数据除以采样点数为直流分量 
    for (count = 1; count < ADCDataLength; count++) // 提取谐波分量
    {
        VOL[count] = VOL[count] / ADCDataLength * 2;                         
    }
    for (count = 1, MAX = 0, index_MAX = 1; count < ADCDataLength / 2; count++) //获取最大值来计算此时的频率
    {
        if (VOL[count] > MAX)
        {
            MAX = VOL[count];    //获取幅度最大的波 也就是基波
            index_MAX = count;   //记录幅度最大的波的索引
        }
    }
    frequency = index_MAX * Fre_Sam / ADCDataLength;
    return frequency;
}

void Right_Fre( void )
{
    WaveNum = 4;
    float now_frequency = 0.0f;
    float last_frequency = -1.0f;
    Fre_Sam = (float)MAX_Fre_Sam * 2.0f / 3.0f;
    const int max_iters = 10;
    const float eps = 0.1f; // 频率收敛容限
    int iters = 0;

    while (iters++ < max_iters)
    {
        now_frequency = Obtain_Fre();
        if (now_frequency > 0.0f && fabsf(now_frequency - last_frequency) < eps)
        {
            break;
        }
        last_frequency = now_frequency;

        if (now_frequency <= 0.0f) {
            // 保护：避免用 0 计算采样率
            Fre_Sam = MIN_Fre_Sam;
        } else {
            Fre_Sam = now_frequency * 2.56f;
        }

        if (Fre_Sam > MAX_Fre_Sam) {
            Fre_Sam = MAX_Fre_Sam;
            now_frequency = Obtain_Fre();
            break;
        }
        if (Fre_Sam < MIN_Fre_Sam) {
            Fre_Sam = MIN_Fre_Sam;
            now_frequency = Obtain_Fre();
            break;
        }
    }

    Fre = now_frequency;
}

uint8_t Judge_Wave( void )  
{
    unsigned int count, index;
    float max = 0;
    float average = 0; // 用于计算直流分量

    for (count = 0; count < ADCDataLength; count++)
    {
        average += VOL[count];
    }
    average /= ADCDataLength;

    for (count = 0; count < ADCDataLength; count++)
    {
        data[count].real = VOL[count] - average; // 减去平均值
        data[count].imag = 0;
    }

    fft(ADCDataLength, data);

    for (count = 0; count < ADCDataLength; count++)
    {
        // 计算模值
        data[count].real = sqrt(data[count].real * data[count].real + data[count].imag * data[count].imag); 
    }
    
    data[0].real = data[0].real / ADCDataLength; // 直流分量 (现在应该接近0)
    for (count = 1; count < ADCDataLength / 2; count++)
    {
        data[count].real = data[count].real / ADCDataLength * 2; // 谐波分量
    }

    for (count = 1, max = 0, index = 1; count < ADCDataLength / 2; count++)
    {
        if (data[count].real > max)
        {
            max = data[count].real;
            index = count;
        }
    }

    // 检查锯齿波 (A2 > 0.45 * A1)
    if ( (index * 2 < ADCDataLength / 2) &&   // 必须检查边界！
         (data[index * 2].real > max * 0.45) )
    {
        return 3; // 3: Sawtooth wave
    }
    
    // 检查方波 (A3 > 0.2 * A1)
    if ( (index * 3 < ADCDataLength / 2) &&   // 必须检查边界！
         (data[index * 3].real > max * 0.20) )
    {
        return 2; // 2: Square wave
    }
    
    // 检查三角波 (A3 > 0.1 * A1)
    if ( (index * 3 < ADCDataLength / 2) &&   // 必须检查边界！
         (data[index * 3].real > max * 0.10) )
    {
        return 1; // 1: Triangular
    }

    return 0; // 0: Sine
}

void Printf(int i, char *format, ... )
{
    i--;
    if(i > 8) i = 8;
    char String[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);

    // 使用 8x6 字体, 颜色 0=正常
    OLED_PrintASCIIString(0, i*8, String, &afont8x6, OLED_COLOR_NORMAL); 
    // 此函数不自动调用 OLED_ShowFrame()
}

//主循环
void App_Main_Loop(void)
{

    static float fre_sam_wave = 0; //不会随循环执行而多次初始化

    if(STOP == 0) // 如果没有暂停
    {
        Fre = Obtain_Fre();     // 测量当前频率
        if(RIGHT == 2)          // 上电自动初始化 auto键位置right为2
        {
            OLED_NewFrame(); 
            Printf(1,".....   AUTO   ..... ");
            OLED_ShowFrame();
            
            Right_Fre();  // 执行自动频率和采样率锁定
            
            Point = 16;   // 每个波形采集 16 个点
            fre_sam_wave = Fre * Point; // 计算理想采样率
            
            if(fre_sam_wave > MAX_Fre_Sam)   // 限制最大采样率
            {
                fre_sam_wave = MAX_Fre_Sam;
            }
            if(fre_sam_wave < MIN_Fre_Sam)   // 限制最小采样率
            {
                fre_sam_wave = MIN_Fre_Sam;
            }
            
            Point = fre_sam_wave / Fre;    // 更新 Point 值
            RIGHT = 1; // 标记自动设置完成第一阶段
        }
        
        Set_SamplingFre_HAL(fre_sam_wave); // 设置显示用的采样率
        MY_Get_Vol_HAL(VOL);               // 采集数据到 VOL 数组
        
        if(RIGHT == 1) // 如果是自动设置刚完成的状态
        {
            float max = VOL[0],min = VOL[0];
            for(int i = 0; i<ADCDataLength/2; i++) // 只分析半个缓冲区数据足够
            {
                if(max < VOL[i]) max = VOL[i];
                if(min > VOL[i]) min = VOL[i];
            }
            MIN_WAVE = min - 0.1; // 计算垂直偏移，加一点裕量
            VPP = max - min + 0.2; // 计算垂直幅度，加一点裕量
            RIGHT = 0; // 自动设置彻底完成
        }
    }

    // 根据当前模式显示波形或频谱
    if(SPEC == 1) // 如果处于频谱视图
    {
        if(STOP == 1) // 如果暂停了，需要重新采一帧数据显示频谱
        {
            // 这里 fre_sam_wave 仍然是上次的值
            Set_SamplingFre_HAL(fre_sam_wave);
            MY_Get_Vol_HAL(VOL);
        }
        OLED_Show_SPEC(fre_sam_wave);
    }
    else // 如果不在频谱视图
    {
        if(SHOW == 1) // 如果处于波形视图
        {
            OLED_Show_Wave_HAL();   
        }
        else // 如果处于数据显示视图
        {
            OLED_Show_Date(fre_sam_wave);   
        }
    }
}
