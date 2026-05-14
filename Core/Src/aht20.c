#include "aht20.h"
#include "sw_i2c.h"  // 1. 包含新的头文件

#define AHT20_ADDRESS 0x70 // 0x38 << 1 (8位写地址)
#define AHT20_READ_ADDRESS (AHT20_ADDRESS | 0x01) // 0x71 (8位读地址)

void AHT20_Init(){
	uint8_t readBuffer = 0;
	HAL_Delay(40);

    // 2. 替换 HAL_I2C_Master_Receive
	SW_I2C_Start();
    SW_I2C_Send_Byte(AHT20_READ_ADDRESS); // 发送读地址
    if (SW_I2C_Wait_Ack() == 0) // 等待ACK
    {
        readBuffer = SW_I2C_Read_Byte(0); // 读取1字节并发送NACK (这是唯一字节)
    }
    SW_I2C_Stop();
	
	if((readBuffer & 0x08) == 0){
		uint8_t sendBuffer[3]={0xBE, 0X08, 0X00};
		
        // 3. 替换 HAL_I2C_Master_Transmit
		SW_I2C_Start();
        SW_I2C_Send_Byte(AHT20_ADDRESS); // 发送写地址
        if (SW_I2C_Wait_Ack() == 0)
        {
            // 循环发送数据
            for(int i=0; i<3; i++)
            {
                SW_I2C_Send_Byte(sendBuffer[i]);
                if (SW_I2C_Wait_Ack() != 0)
                {
                    break; // 如果中途NACK，则停止
                }
            }
        }
        SW_I2C_Stop();
	}
}

void AHT20_Read(float *temperature,float *humidity){
	uint8_t sendBuffer[3] = {0xAC, 0X33, 0x00};
	uint8_t readBuffer[6] = {0};
	
    // 1. 替换 HAL_I2C_Master_Transmit (触发测量)
	SW_I2C_Start();
    SW_I2C_Send_Byte(AHT20_ADDRESS); // 发送写地址
    if (SW_I2C_Wait_Ack() == 0)
    {
        for(int i=0; i<3; i++)
        {
            SW_I2C_Send_Byte(sendBuffer[i]);
            if (SW_I2C_Wait_Ack() != 0)
            {
                break; 
            }
        }
    }
    SW_I2C_Stop();

	HAL_Delay(80); // 等待测量完成

	// 2. 替换 HAL_I2C_Master_Receive (读取数据)
	SW_I2C_Start();
    SW_I2C_Send_Byte(AHT20_READ_ADDRESS); // 发送读地址
    if (SW_I2C_Wait_Ack() == 0) // 等待ACK
    {
        // 循环读取6字节
        readBuffer[0] = SW_I2C_Read_Byte(1); // 读第1字节, 发送ACK
        readBuffer[1] = SW_I2C_Read_Byte(1); // 读第2字节, 发送ACK
        readBuffer[2] = SW_I2C_Read_Byte(1); // 读第3字节, 发送ACK
        readBuffer[3] = SW_I2C_Read_Byte(1); // 读第4字节, 发送ACK
        readBuffer[4] = SW_I2C_Read_Byte(1); // 读第5字节, 发送ACK
        readBuffer[5] = SW_I2C_Read_Byte(0); // 读第6字节, 发送NACK (最后)
    }
    SW_I2C_Stop();
	
    // 传感器数据处理逻辑 (保持不变)
	if((readBuffer[0] &0x80) == 0){
		uint32_t data =0;
		data = ((uint32_t)readBuffer[3] >>4)+((uint32_t)readBuffer[2] << 4)+((uint32_t)readBuffer[1] << 12);
		*humidity = data * 100.0f/(1 << 20);
		
		data = ((uint32_t)(readBuffer[3] &0x0F )<<16)+((uint32_t)readBuffer[4] << 8)+((uint32_t)readBuffer[5]);
		*temperature =data  * 200.0f/( 1<<20 ) - 50;
	}
}
