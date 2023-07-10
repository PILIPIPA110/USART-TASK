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
 ALIENTEK ս��STM32F103������ FreeRTOSʵ��2-1
 FreeRTOS��ֲʵ��-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


int main(void)
{
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
		delay_init();	    				//��ʱ������ʼ��	  
		uart_init(115200);					//��ʼ������
		LED_Init();		  					//��ʼ��LED
    KEY_Init();
		BEEP_Init();
		IWDG_Init(4,3125);    //Ԥ��Ƶ��Ϊ64,����ֵΪ5*625,���ʱ��Ϊ5s	
		freertos_demo();
}

