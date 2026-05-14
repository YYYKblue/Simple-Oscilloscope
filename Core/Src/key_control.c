/*
 * key_control.c
 *
 * Created on: 2025年10月25日
 * Author: zx
 */

#include "key_control.h"
#include "gpio.h" // 为了引脚定义 

// --- 变量定义 ---
// 定义全局页面变量，开机默认为 0 (第一页: 目录)
volatile uint8_t g_menuState = 0;

// --- 静态变量 (仅在此文件可见) ---
// 防抖时间 (毫秒) - 50ms 是一个比较合适的值
#define KEY_DEBOUNCE_TIME_MS 50 

// 用于防抖的静态变量，记录上次按键中断的时间
static volatile uint32_t last_key_press_time = 0;


/**
 * @brief 处理来自EXTI中断的按键事件 (包含非阻塞防抖)
 * @note  此函数应在 HAL_GPIO_EXTI_Callback 中被调用
 * @param GPIO_Pin: 触发中断的引脚
 */
void Key_Process_IT(uint16_t GPIO_Pin)
{
    // 1. 获取当前时间
    uint32_t current_time = HAL_GetTick();

    // 2. 检查防抖
    // 如果距离上次有效按键的时间太短，就认为是抖动，直接返回。
    if (current_time - last_key_press_time < KEY_DEBOUNCE_TIME_MS)
    {
        return; // 忽略抖动
    }

    // 3. 确认为一次有效按键，更新时间戳
    last_key_press_time = current_time;

    // 4. 根据按键引脚处理逻辑
    if (GPIO_Pin == oled_key_down_Pin)
    {
        // --- 逻辑：oled_key_down (减页数 / 向下) ---
        if (g_menuState == 0) // 如果在第0页
        {
            g_menuState = TOTAL_PAGES - 1; // 循环到最后一页 (即第4页)
        }
        else
        {
            g_menuState--; // 减一页
        }
    }
    else if (GPIO_Pin == oled_key_up_Pin)
    {
        // --- 逻辑：oled_key_up (加页数 / 向上) ---
        if (g_menuState == (TOTAL_PAGES - 1)) // 如果在最后一页 (第4页)
        {
            g_menuState = 0; // 循环到第一页 (第0页)
        }
        else
        {
            g_menuState++; // 加一页
        }
    }
}
