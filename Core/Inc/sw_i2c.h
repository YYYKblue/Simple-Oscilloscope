#ifndef __SW_I2C_H
#define __SW_I2C_H

#include "main.h" // 包含HAL库定义

/*--------------------------------------------------*/
/* (这应该与cube中设置的引脚和标签匹配)        */

#define OLED_SCL_PORT   GPIOB
#define OLED_SCL_PIN    GPIO_PIN_7  // PB7 -> SCL

#define OLED_SDA_PORT   GPIOB
#define OLED_SDA_PIN    GPIO_PIN_6  // PB6 -> SDA
/*--------------------------------------------------*/


// 宏定义来控制引脚电平
#define I2C_SCL_1() HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_0() HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)

#define I2C_SDA_1() HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_0() HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)

#define I2C_SDA_READ() HAL_GPIO_ReadPin(OLED_SDA_PORT, OLED_SDA_PIN)


// 公共函数声明
void SW_I2C_Start(void);
void SW_I2C_Stop(void);
uint8_t SW_I2C_Wait_Ack(void);
void SW_I2C_Send_Byte(uint8_t byte);

/**
 * @brief 发送ACK信号
 */
void SW_I2C_Ack(void);

/**
 * @brief 发送NACK信号
 */
void SW_I2C_NAck(void);

/**
 * @brief 读取一个字节
 * @param ack 1 = 发送ACK, 0 = 发送NACK
 * @return 读取到的字节
 */
uint8_t SW_I2C_Read_Byte(uint8_t ack);

#endif
