#include "iwdg.h"
#include "UserConfig.h"



void IWDG_Init(u8 prer,u16 rlr) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //使能对寄存器IWDG_PR和IWDG_RLR的写操作
	
	IWDG_SetPrescaler(prer);  //设置IWDG预分频值:设置IWDG预分频值为64
	
	IWDG_SetReload(rlr);  //设置IWDG重装载值
	
	IWDG_ReloadCounter();  //按照IWDG重装载寄存器的值重装载IWDG计数器
#if IWDG_ON
	IWDG_Enable();  //使能IWDG
#endif	
}
//喂独立看门狗
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();	//重载计数值									   
}

