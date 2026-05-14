#include "sw_i2c.h"

// 简单的延时函数。
// 注意：这个延时的精确度决定了I2C速度。
// 对于72MHz的F103，这个值大致在100-400kHz范围内。
// 如果通信失败，可能需要增加 'i' 的值来降低速度。
static void I2C_Delay(void)
{
    volatile uint8_t i = 5; 
    while (i--);
}

// 将SDA引脚设置为输出模式 (开漏模式已在CubeMX中配置)
static void SDA_Set_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // 开漏输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_SDA_PORT, &GPIO_InitStruct);
}

// 将SDA引脚设置为输入模式 (用于读取ACK)
static void SDA_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // 启用上拉
    HAL_GPIO_Init(OLED_SDA_PORT, &GPIO_InitStruct);
}

/**
  * @brief  I2C 开始信号
  */
void SW_I2C_Start(void)
{
    SDA_Set_Output(); // 确保SDA是输出
    I2C_SDA_1();
    I2C_SCL_1();
    I2C_Delay();
    I2C_SDA_0(); // SCL高电平期间，SDA从高->低
    I2C_Delay();
    I2C_SCL_0(); // 钳住总线
    I2C_Delay();
}

/**
  * @brief  I2C 停止信号
  */
void SW_I2C_Stop(void)
{
    SDA_Set_Output(); // 确保SDA是输出
    I2C_SCL_0();
    I2C_SDA_0();
    I2C_Delay();
    I2C_SCL_1(); // SCL高电平期间，SDA从低->高
    I2C_Delay();
    I2C_SDA_1();
    I2C_Delay();
}

/**
  * @brief  发送一个字节
  */
void SW_I2C_Send_Byte(uint8_t byte)
{
    uint8_t i;
    SDA_Set_Output(); // 确保SDA是输出
    I2C_SCL_0(); // 拉低时钟准备数据
    I2C_Delay();

    for (i = 0; i < 8; i++)
    {
        if ((byte & 0x80) >> 7) // 发送最高位
        {
            I2C_SDA_1();
        }
        else
        {
            I2C_SDA_0();
        }
        byte <<= 1;
        I2C_Delay();
        I2C_SCL_1(); // 拉高时钟，OLED读取数据
        I2C_Delay();
        I2C_SCL_0(); // 拉低时钟，准备下一位
        I2C_Delay();
    }
}

/**
  * @brief  等待ACK信号
  * @retval 0 = 收到ACK, 1 = 收到NACK
  */
uint8_t SW_I2C_Wait_Ack(void)
{
    uint8_t ack_val;
    
    I2C_SCL_0();
    I2C_Delay();
    SDA_Set_Input();  // 关键：切换SDA为输入模式以读取OLED的响应
    I2C_SCL_1();      // OLED在SCL高电平时拉低SDA表示ACK
    I2C_Delay();

    ack_val = I2C_SDA_READ(); // 读取ACK位

    I2C_SCL_0(); // 拉低时钟
    I2C_Delay();
    SDA_Set_Output(); // 恢复SDA为输出模式

    return ack_val; // 0 (低电平) = ACK, 1 (高电平) = NACK
}
// ... (SW_I2C_Wait_Ack 函数的下面)

/**
  * @brief  发送ACK信号
  */
void SW_I2C_Ack(void)
{
    I2C_SCL_0();
    SDA_Set_Output();
    I2C_SDA_0();
    I2C_Delay();
    I2C_SCL_1();
    I2C_Delay();
    I2C_SCL_0();
}

/**
  * @brief  发送NACK信号
  */
void SW_I2C_NAck(void)
{
    I2C_SCL_0();
    SDA_Set_Output();
    I2C_SDA_1();
    I2C_Delay();
    I2C_SCL_1();
    I2C_Delay();
    I2C_SCL_0();
}

/**
  * @brief  读取一个字节
  * @param  ack: 1 = 发送ACK, 0 = 发送NACK
  * @retval 读取到的字节
  */
uint8_t SW_I2C_Read_Byte(uint8_t ack)
{
    uint8_t i, receive = 0;
    SDA_Set_Input(); // 关键：切换SDA为输入
    I2C_Delay();

    for (i = 0; i < 8; i++)
    {
        I2C_SCL_0(); // 拉低时钟
        I2C_Delay();
        I2C_SCL_1(); // 拉高时钟，读取数据
        I2C_Delay();
        receive <<= 1;
        if (I2C_SDA_READ())
        {
            receive |= 1;
        }
    }
    
    // 读完8位后，发送 (N)ACK
    if (ack)
    {
        SW_I2C_Ack(); // 发送ACK
    }
    else
    {
        SW_I2C_NAck(); // 发送NACK (表示这是最后一个字节)
    }
    
    SDA_Set_Output(); // 恢复SDA为输出模式
    return receive;
}
