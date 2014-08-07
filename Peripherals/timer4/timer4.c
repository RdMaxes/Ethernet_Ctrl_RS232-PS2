#include "timer4.h"

unsigned int uip_timer=0;//uip system counter

//Timer4 global interrupt handler
void TIM4_IRQHandler(void)
{ 	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) 
	{
  		uip_timer++;	
	} 
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );
				    		  			    	    
}
 
//setup Timer4
//arr: period -1
//psc: prescaler -1
void TIM4_Init(u16 arr,u16 psc)
{	
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 

	TIM_TimeBaseStructure.TIM_Period = arr; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc;   
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); 
 
	TIM_ITConfig( TIM4,TIM_IT_Update|TIM_IT_Trigger,ENABLE);
 
	TIM_Cmd(TIM4, ENABLE);  
 	
  	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);   								 
}
