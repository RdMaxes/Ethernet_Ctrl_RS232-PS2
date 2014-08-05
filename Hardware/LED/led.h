#ifndef _LED_H
#define _LED_H

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

/*LED1 Pin Definition*/
#define LED1_Port		GPIOA
#define LED1_CLK		RCC_APB2Periph_GPIOA
#define LED1_Pin		GPIO_Pin_11
#define GREEN_ON();		GPIO_SetBits( LED1_Port, LED1_Pin ); GPIO_SetBits 
#define GREEN_OFF();	GPIO_ResetBits( LED1_Port, LED1_Pin );

/*LED2 Pin Definition*/
#define LED2_Port		GPIOA
#define LED2_CLK		RCC_APB2Periph_GPIOA
#define LED2_Pin		GPIO_Pin_12
#define RED_ON();		GPIO_SetBits( LED2_Port, LED2_Pin );  
#define RED_OFF();		GPIO_ResetBits( LED2_Port, LED2_Pin );
/*Function Prototype*/
//LED Initial 
void LED_Init(void);

#endif