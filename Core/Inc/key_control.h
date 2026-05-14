/*
 * key_control.h
 *
 * Created on: 2025年10月25日
 * Author: YourName
 */

#ifndef __KEY_CONTROL_H
#define __KEY_CONTROL_H

#include "main.h" // 包含 main.h 以获取 HAL 和引脚定义

// --- 宏定义 ---
// 您的菜单逻辑 (g_menuState == 0 到 4) 表明总共有 5 个页面
#define TOTAL_PAGES 4

// --- 外部变量声明 ---
// g_menuState 会在中断中被修改，在主循环中被读取，
// 必须声明为 volatile，以防止编译器过度优化。
extern volatile uint8_t g_menuState;

// --- 函数原型 ---
/**
 * @brief 处理来自EXTI中断的按键事件
 * @note  此函数应在 HAL_GPIO_EXTI_Callback 中被调用
 * @param GPIO_Pin: 触发中断的引脚
 */
void Key_Process_IT(uint16_t GPIO_Pin);

#endif /* __KEY_CONTROL_H */
