#ifndef _SN74CBT3306_H
#define _SN74CBT3306_H

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

/*Port control pin Definition*/
#define PORTCTRL_Port				GPIOA
#define PORTCTRL_CLK				RCC_APB2Periph_GPIOA
#define PORTCTRL_Pin				GPIO_Pin_7
#define PORT_CONNECT();				GPIO_ResetBits( PORTCTRL_Port, PORTCTRL_Pin );  
#define PORT_DISCONNECT();			GPIO_SetBits( PORTCTRL_Port, PORTCTRL_Pin );

/*Function Prototype*/
//SN74CBT3306 Initial 
void SN74CBT3306_Init(void);

#endif