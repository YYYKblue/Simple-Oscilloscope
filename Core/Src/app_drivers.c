#include "app_drivers.h"
#include "tim.h"
#include "gpio.h"

void LED_Turn(void)
{
    // 最小系统板子上的 LED 灯控制翻转代码
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

//获取计数器的值
int16_t Encoder_TIM1_Get(void)
{
    int16_t Temp;
    Temp = __HAL_TIM_GET_COUNTER(&htim1); 
    __HAL_TIM_SET_COUNTER(&htim1, 0);     
    return Temp;
}

//设置一个常亮led

void lightpb0(void)
{
		HAL_GPIO_WritePin(led_b0_GPIO_Port,led_b0_Pin,GPIO_PIN_SET);

}

void LED_Turn1(void){

		HAL_GPIO_TogglePin(led_b1_GPIO_Port,led_b1_Pin);
}
