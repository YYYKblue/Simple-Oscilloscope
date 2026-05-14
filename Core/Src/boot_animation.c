#include "boot_animation.h"
#include "oled.h" 
#include "font.h" 
#include "main.h" 

void OLED_PlayBootAnimation(void)
{
    OLED_NewFrame();

    //绘制鼠鼠Logo
    OLED_DrawImage(0, 0, &shuImg, OLED_COLOR_NORMAL);
    
    // 绘制进度条外框
    uint8_t bar_x = 14;     
    uint8_t bar_y = 52;     
    uint8_t bar_w = 100;    
    uint8_t bar_h = 8;      
    OLED_DrawRectangle(bar_x, bar_y, bar_w, bar_h, OLED_COLOR_NORMAL);
    
    // 立即显示Logo和外框
    OLED_ShowFrame();
    HAL_Delay(100); 

    // 动态填充进度条
    // 我们从 (bar_x + 1) 画到 (bar_x + bar_w )
    for (int i = 1; i <= bar_w; i++)
    {
        // 绘制一根垂直线来填充
        OLED_DrawLine(bar_x + i, bar_y + 1, bar_x + i, bar_y + bar_h - 1, OLED_COLOR_NORMAL);

        // 为了让动画可见，每绘制几格刷新一次
        if (i % 4 == 0) 
        {
            OLED_ShowFrame();
            HAL_Delay(5); // 控制动画速度
        }
    }

    HAL_Delay(300); // 满格后停留 300ms
    OLED_NewFrame();
    OLED_ShowFrame();
}
