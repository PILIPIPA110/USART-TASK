/* 头文件 */
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "freertos_demo.h"
#include "queue.h"
#include "semphr.h"
#include "malloc.h"
#include "string.h"
#include "beep.h"
#include "event_groups.h"
#include "iwdg.h"
#include "UserConfig.h"
#include "crc.h"
/*************************************************************************/


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define TASK1_PRIO		10
//任务堆栈大小	
#define TASK1_SIZE 		128  
//任务句柄
TaskHandle_t TASK1_Handler;
//任务函数
void TASK1(void *pvParameters);

//任务优先级
#define TASK2_PRIO		11
//任务堆栈大小	
#define TASK2_SIZE 		256  
//任务句柄
TaskHandle_t TASK2_Handler;
//任务函数
void TASK2(void *pvParameters);

//任务优先级
#define TASK3_PRIO		2
//任务堆栈大小	
#define TASK3_SIZE 		128  
//任务句柄
TaskHandle_t TASK3_Handler;
//任务函数
void TASK3(void *pvParameters);

/*************************************************************************/
QueueHandle_t DMA_queue;    //DMA队列句柄
SemaphoreHandle_t BinarySemaphore;		//二值信号量句柄
SemaphoreHandle_t DMAdone;		//二值信号量句柄
EventGroupHandle_t xCreatedEventGroup;		//事件组句柄
u8 	CMD_TYPE;
/*************************************************************************/	

/** 
  * @brief  最多一次监测MAX_TASK_NUM个任务，如果多于该数，则需要定义多个事件标志组
  */
#define WDG_BIT_DOWN_TASK_1             (1 << 0)
#define WDG_BIT_DOWN_TASK_2             (1 << 1)
#define WDG_BIT_DOWN_TASK_3             (1 << 2)
#define WDG_BIT_TASK_ALL                (WDG_BIT_DOWN_TASK_1)

/*************************************************************************/	

//用于命令解析用的命令值
#define LED0ON	1
#define LED0OFF	2
#define BEEPON	3
#define BEEPOFF	4
#define COMMANDERR	0XFF

//将字符串中的小写字母转换为大写
//str:要转换的字符串
//len：字符串长度
void LowerToCap(u8 *str,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		if((96<str[i])&&(str[i]<123))	//小写字母
		str[i]=str[i]-32;				//转换为大写
	}
}

//命令处理函数，将字符串命令转换成命令值
//str：命令
//返回值: 0XFF，命令错误；其他值，命令值
u8 CommandProcess(u8 *str)
{
	u8 CommandValue=COMMANDERR;/*strcmp是字符串比较*/
	if(strcmp((char*)str,"LED0ON")==0) CommandValue=LED0ON;
	else if(strcmp((char*)str,"LED0OFF")==0) CommandValue=LED0OFF;
	else if(strcmp((char*)str,"BEEPON")==0) CommandValue=BEEPON;
	else if(strcmp((char*)str,"BEEPOFF")==0) CommandValue=BEEPOFF;
	return CommandValue;
}

/*************************************************************************/	

void freertos_demo(void)
{
	//创建队列
	
		DMA_queue = xQueueCreate( 1, sizeof(uint8_t*) );
		if(DMA_queue != NULL)
		{
			printf("队列DMA_queue创建成功\r\n");	
		}else printf("队列DMA_queue创建失败\r\n");	


	
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

/*************************************************************************/	
//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
   
    //创建二值信号量
		BinarySemaphore = xSemaphoreCreateBinary();	
		DMAdone = xSemaphoreCreateBinary();
		//创建TASK1任务
    xTaskCreate((TaskFunction_t )TASK1,     	
                (const char*    )"TASK1",   	
                (uint16_t       )TASK1_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )TASK1_PRIO,	
                (TaskHandle_t*  )&TASK1_Handler);   
    //创建TASK2任务
    xTaskCreate((TaskFunction_t )TASK2,     
                (const char*    )"TASK2",   
                (uint16_t       )TASK2_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TASK2_PRIO,
                (TaskHandle_t*  )&TASK2_Handler);
		//创建TASK3任务
    xTaskCreate((TaskFunction_t )TASK3,     
                (const char*    )"TASK3",   
                (uint16_t       )TASK3_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TASK3_PRIO,
                (TaskHandle_t*  )&TASK3_Handler);

    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//TASK1任务函数 实现传输
void TASK1(void *pvParameters)
{
		BaseType_t err=pdFALSE;
		BaseType_t err2=pdFALSE;
		u8* addr_recebuff;

		uint16_t crc16 = 0; 
		
		
	
		while(1)
    {

			if(DMAdone!=NULL)
			{
				err2=xSemaphoreTake(DMAdone,portMAX_DELAY);
				if(err2==pdTRUE)
				{
					printf("crc\r\n");
					crc16 = ReceBuff[Datalen];
					crc16 <<= 8;
					crc16 += ReceBuff[Datalen+1];
					if(crc16 == crc16Modbus(ReceBuff,Datalen))
						{
								ReceBuff[Datalen] = 0;
								ReceBuff[Datalen+1] = 0;
								CMD_TYPE = ReceBuff[0];
								addr_recebuff = myrealloc(SRAMIN,&ReceBuff[1],Datalen);
								//释放二值信号量
								printf("123: %s\r\n",addr_recebuff);
								xSemaphoreGive(BinarySemaphore);				
						}else
						{
								printf("CRC校验码无效，CRC应为%d\r\n",crc16Modbus(ReceBuff,Datalen));
								myfree(SRAMIN,ReceBuff);
						}
						
					}
			}				
			if(BinarySemaphore!=NULL)
			{
				err=xSemaphoreTake(BinarySemaphore,portMAX_DELAY);
				if(err==pdTRUE)		
				{
					printf("入队\r\n");
					xQueueSend(DMA_queue,&addr_recebuff,portMAX_DELAY);
					myfree(SRAMIN,addr_recebuff);
				}
			}

		}		
}   

//TASK2任务函数 实现大数据出队，解析函数
void TASK2(void *pvParameters)
{
		u8* addr_recebuff2;
		u8* CommandStr;
		u8 CommandValue=COMMANDERR;
		u8 Datalen2=0;
		BaseType_t err =0;


	
		while(1)
    {
				err =	xQueueReceive(DMA_queue,&addr_recebuff2,portMAX_DELAY);

				if(err != pdTRUE)
				{
						printf("队列DMA_queue读取失败\r\n");	
				}else
				{		
						printf("出队\r\n");
						Datalen2 = Datalen*2;
						CommandStr=mymalloc(SRAMIN,Datalen2+1);
						sprintf((char*)CommandStr,"%s",addr_recebuff2);
						CommandStr[Datalen2]='\0';							//加上字符串结尾符号
						LowerToCap(CommandStr,Datalen2);						//将字符串转换为大写		
						CommandValue=CommandProcess(CommandStr);		//命令解析	
						
						if(CommandValue != COMMANDERR)
						{

							printf("命令类型为:%d\r\n",CMD_TYPE);
							printf("命令为:%s\r\n",CommandStr);
								switch(CMD_TYPE)
								{
									case 1:
										switch(CommandValue)						//处理命令
										{
											case LED0ON: 
												LED0=0;
												break;
											case LED0OFF:
												LED0=1;
												break;
										}
									break;
									case 2:
										switch(CommandValue)
										{
											case BEEPON:
												BEEP=1;
												break;
											case BEEPOFF:
												BEEP=0;
												break;
										}
									break;
								}
						}	
						else
						{
							printf("命令为:%s\r\n",ReceBuff);
							printf("无效的命令，请重新输入!!\r\n");
						}
						
//						memset(USART_RX_BUF,0,USART_REC_LEN);			//串口接收缓冲区清零
//						memset(addr_recebuff2,0,Datalen);
						myfree(SRAMIN,CommandStr);						//释放内存	
						
				}
	
    }
}


//TASK3任务函数 事件组看门狗，监视事件标志组


void TASK3(void *pvParameters)
{
#if IWDG_ON		
		
		static TickType_t    xTicksToWait = 100 / portTICK_PERIOD_MS*10; /* 最大延迟1s */
    EventBits_t            uxBits;
    /* 创建事件标志组 */
    xCreatedEventGroup = xEventGroupCreate();
    if(xCreatedEventGroup == NULL)
    {
        printf("失败\r\n");
        
				return;
    }
    while(1)
    {
      
				/* 等待所有任务发来事件标志 */
        uxBits = xEventGroupWaitBits(xCreatedEventGroup,     /* 事件标志组句柄 */
                                     WDG_BIT_TASK_ALL,        /* 等待WDG_BIT_TASK_ALL被设置 */
                                     pdTRUE,                /* 退出前WDG_BIT_TASK_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
                                     pdTRUE,                /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
                                     xTicksToWait);            /* 等待延迟时间 */
        if((uxBits & WDG_BIT_TASK_ALL) == WDG_BIT_TASK_ALL)
        {
            IWDG_Feed();
        }
        else
        {
            /* 通过变量uxBits简单的可以在此处检测那个任务长期没有发来运行标志 */
        }
				if(DMA_IWDG == 1)
				{
					xEventGroupSetBits(xCreatedEventGroup,WDG_BIT_DOWN_TASK_1);
				}	
		}
#else
		while(1)
    {
			LED1 =! LED1;
			delay_xms(100);
		}
#endif
}
