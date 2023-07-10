/* ͷ�ļ� */
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


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define TASK1_PRIO		10
//�����ջ��С	
#define TASK1_SIZE 		128  
//������
TaskHandle_t TASK1_Handler;
//������
void TASK1(void *pvParameters);

//�������ȼ�
#define TASK2_PRIO		11
//�����ջ��С	
#define TASK2_SIZE 		256  
//������
TaskHandle_t TASK2_Handler;
//������
void TASK2(void *pvParameters);

//�������ȼ�
#define TASK3_PRIO		2
//�����ջ��С	
#define TASK3_SIZE 		128  
//������
TaskHandle_t TASK3_Handler;
//������
void TASK3(void *pvParameters);

/*************************************************************************/
QueueHandle_t DMA_queue;    //DMA���о��
SemaphoreHandle_t BinarySemaphore;		//��ֵ�ź������
SemaphoreHandle_t DMAdone;		//��ֵ�ź������
EventGroupHandle_t xCreatedEventGroup;		//�¼�����
u8 	CMD_TYPE;
/*************************************************************************/	

/** 
  * @brief  ���һ�μ��MAX_TASK_NUM������������ڸ���������Ҫ�������¼���־��
  */
#define WDG_BIT_DOWN_TASK_1             (1 << 0)
#define WDG_BIT_DOWN_TASK_2             (1 << 1)
#define WDG_BIT_DOWN_TASK_3             (1 << 2)
#define WDG_BIT_TASK_ALL                (WDG_BIT_DOWN_TASK_1)

/*************************************************************************/	

//������������õ�����ֵ
#define LED0ON	1
#define LED0OFF	2
#define BEEPON	3
#define BEEPOFF	4
#define COMMANDERR	0XFF

//���ַ����е�Сд��ĸת��Ϊ��д
//str:Ҫת�����ַ���
//len���ַ�������
void LowerToCap(u8 *str,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		if((96<str[i])&&(str[i]<123))	//Сд��ĸ
		str[i]=str[i]-32;				//ת��Ϊ��д
	}
}

//������������ַ�������ת��������ֵ
//str������
//����ֵ: 0XFF�������������ֵ������ֵ
u8 CommandProcess(u8 *str)
{
	u8 CommandValue=COMMANDERR;/*strcmp���ַ����Ƚ�*/
	if(strcmp((char*)str,"LED0ON")==0) CommandValue=LED0ON;
	else if(strcmp((char*)str,"LED0OFF")==0) CommandValue=LED0OFF;
	else if(strcmp((char*)str,"BEEPON")==0) CommandValue=BEEPON;
	else if(strcmp((char*)str,"BEEPOFF")==0) CommandValue=BEEPOFF;
	return CommandValue;
}

/*************************************************************************/	

void freertos_demo(void)
{
	//��������
	
		DMA_queue = xQueueCreate( 1, sizeof(uint8_t*) );
		if(DMA_queue != NULL)
		{
			printf("����DMA_queue�����ɹ�\r\n");	
		}else printf("����DMA_queue����ʧ��\r\n");	


	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

/*************************************************************************/	
//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
   
    //������ֵ�ź���
		BinarySemaphore = xSemaphoreCreateBinary();	
		DMAdone = xSemaphoreCreateBinary();
		//����TASK1����
    xTaskCreate((TaskFunction_t )TASK1,     	
                (const char*    )"TASK1",   	
                (uint16_t       )TASK1_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )TASK1_PRIO,	
                (TaskHandle_t*  )&TASK1_Handler);   
    //����TASK2����
    xTaskCreate((TaskFunction_t )TASK2,     
                (const char*    )"TASK2",   
                (uint16_t       )TASK2_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TASK2_PRIO,
                (TaskHandle_t*  )&TASK2_Handler);
		//����TASK3����
    xTaskCreate((TaskFunction_t )TASK3,     
                (const char*    )"TASK3",   
                (uint16_t       )TASK3_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TASK3_PRIO,
                (TaskHandle_t*  )&TASK3_Handler);

    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//TASK1������ ʵ�ִ���
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
								//�ͷŶ�ֵ�ź���
								printf("123: %s\r\n",addr_recebuff);
								xSemaphoreGive(BinarySemaphore);				
						}else
						{
								printf("CRCУ������Ч��CRCӦΪ%d\r\n",crc16Modbus(ReceBuff,Datalen));
								myfree(SRAMIN,ReceBuff);
						}
						
					}
			}				
			if(BinarySemaphore!=NULL)
			{
				err=xSemaphoreTake(BinarySemaphore,portMAX_DELAY);
				if(err==pdTRUE)		
				{
					printf("���\r\n");
					xQueueSend(DMA_queue,&addr_recebuff,portMAX_DELAY);
					myfree(SRAMIN,addr_recebuff);
				}
			}

		}		
}   

//TASK2������ ʵ�ִ����ݳ��ӣ���������
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
						printf("����DMA_queue��ȡʧ��\r\n");	
				}else
				{		
						printf("����\r\n");
						Datalen2 = Datalen*2;
						CommandStr=mymalloc(SRAMIN,Datalen2+1);
						sprintf((char*)CommandStr,"%s",addr_recebuff2);
						CommandStr[Datalen2]='\0';							//�����ַ�����β����
						LowerToCap(CommandStr,Datalen2);						//���ַ���ת��Ϊ��д		
						CommandValue=CommandProcess(CommandStr);		//�������	
						
						if(CommandValue != COMMANDERR)
						{

							printf("��������Ϊ:%d\r\n",CMD_TYPE);
							printf("����Ϊ:%s\r\n",CommandStr);
								switch(CMD_TYPE)
								{
									case 1:
										switch(CommandValue)						//��������
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
							printf("����Ϊ:%s\r\n",ReceBuff);
							printf("��Ч���������������!!\r\n");
						}
						
//						memset(USART_RX_BUF,0,USART_REC_LEN);			//���ڽ��ջ���������
//						memset(addr_recebuff2,0,Datalen);
						myfree(SRAMIN,CommandStr);						//�ͷ��ڴ�	
						
				}
	
    }
}


//TASK3������ �¼��鿴�Ź��������¼���־��


void TASK3(void *pvParameters)
{
#if IWDG_ON		
		
		static TickType_t    xTicksToWait = 100 / portTICK_PERIOD_MS*10; /* ����ӳ�1s */
    EventBits_t            uxBits;
    /* �����¼���־�� */
    xCreatedEventGroup = xEventGroupCreate();
    if(xCreatedEventGroup == NULL)
    {
        printf("ʧ��\r\n");
        
				return;
    }
    while(1)
    {
      
				/* �ȴ������������¼���־ */
        uxBits = xEventGroupWaitBits(xCreatedEventGroup,     /* �¼���־���� */
                                     WDG_BIT_TASK_ALL,        /* �ȴ�WDG_BIT_TASK_ALL������ */
                                     pdTRUE,                /* �˳�ǰWDG_BIT_TASK_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
                                     pdTRUE,                /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
                                     xTicksToWait);            /* �ȴ��ӳ�ʱ�� */
        if((uxBits & WDG_BIT_TASK_ALL) == WDG_BIT_TASK_ALL)
        {
            IWDG_Feed();
        }
        else
        {
            /* ͨ������uxBits�򵥵Ŀ����ڴ˴�����Ǹ�������û�з������б�־ */
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
