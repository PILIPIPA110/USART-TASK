#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "freertos_demo.h"
#include "iwdg.h"
#include "beep.h"

/************************************************
 ALIENTEK 战舰STM32F103开发板 FreeRTOS实验2-1
 FreeRTOS移植实验-库函数版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


int main(void)
{
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
		delay_init();	    				//延时函数初始化	  
		uart_init(115200);					//初始化串口
		LED_Init();		  					//初始化LED
    KEY_Init();
		BEEP_Init();
		IWDG_Init(4,3125);    //预分频数为64,重载值为5*625,溢出时间为5s	
		freertos_demo();
}

