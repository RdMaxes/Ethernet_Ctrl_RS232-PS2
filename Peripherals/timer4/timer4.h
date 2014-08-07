#ifndef _TIMER_H
#define _TIMER_H
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
 
void TIM4_Init(u16 arr,u16 psc);
void TIM4_IRQHandler(void);

#endif
