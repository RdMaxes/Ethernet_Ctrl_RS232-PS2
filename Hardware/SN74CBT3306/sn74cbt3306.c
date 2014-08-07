#include "sn74cbt3306.h"

//SN74CBT3306 configuration 
void SN74CBT3306_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    //Enable PORT Control Pin Clock
	RCC_APB2PeriphClockCmd(PORTCTRL_CLK, ENABLE);
	//Configure RS232 Control Pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = 	PORTCTRL_Pin; 	
	GPIO_Init(PORTCTRL_Port, &GPIO_InitStructure);
}
