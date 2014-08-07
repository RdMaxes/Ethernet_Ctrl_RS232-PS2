#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "led.h"
#include "spi2.h"
#include "usart2.h"
#include "myprintf.h"
#include "enc28j60.h"
#include "timer4.h"
#include "uip.h"
#include "tapdev.h"
#include "timer.h" //it is timer.h of uIP Lib
#include "uip_arp.h"
#include "string.h"
#include "sn74cbt3306.h"
#include "stdlib.h"

void uip_polling(void);	//prototype of uIP check job
uint8_t SerialDataCheck(void); //check serial data
uint8_t ServerDataCheck(uint8_t* dataBuf); //check server local port data
void Switch2ConnectMode(void); //mode switch
void Switch2DisconnectMode(void);
uint8_t GetIPv4Information(char* ptrBuf, uint8_t* ptrIP1,uint8_t* ptrIP2,uint8_t* ptrIP3,uint8_t* ptrIP4);
uint8_t GetMonitorPortInformation(char* cmd_buf, uint16_t* monport);


//General Command Lines
uint8_t str_POFF[] = "POFF";
uint8_t str_PON[] = "PON";
uint8_t str_PS[] = "PS";
//IP Setup Command Lines
uint8_t str_SETDIP[] = "SETDIP";
uint8_t str_SETHIP[] = "SETHIP";
uint8_t str_SETGWAY[] = "SETGWAY";
uint8_t str_SETMASK[] = "SETMASK";
//Echo Command Lines
uint8_t str_ECHODIP[] = "ECHODIP";
uint8_t str_ECHOHIP[] = "ECHOHIP";
uint8_t str_ECHOGWAY[] = "ECHOGWAY";
uint8_t str_ECHOMASK[] = "ECHOMASK";
uint8_t str_ECHOCPORT[] = "ECHOCPORT";
uint8_t str_ECHOSPORT[] = "ECHOSPORT";
const uint8_t sudomymac[6]={0x04,0x02,0x35,0x00,0x00,0x01};	//local MAC address
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])	
uint8_t CONNECTION_STATUS = 1; // 0 = disconnect, 1 = connect
#define CONNECT_MODE 1
#define DISCONNECT_MODE 0

#define CMD_POFF 		1
#define CMD_PON  		2
#define CMD_PS   		3
#define CMD_SETDIP		4
#define CMD_SETHIP		5
#define CMD_SETGWAY		6	
#define CMD_SETMASK		7
#define CMD_ECHODIP		8
#define CMD_ECHOHIP		9
#define CMD_ECHOGWAY	10
#define CMD_ECHOMASK	11
#define CMD_ECHOCPORT	12
#define CMD_ECHOSPORT	13

static void delay(uint32_t delay_count)
{
	while (delay_count) delay_count--;
}


int main(void)
{
	u8 tcp_server_tsta=0XFF;
	u8 tcp_client_tsta=0XFF;
 	uip_ipaddr_t ipaddr;
 	uint8_t CMD = 0;
 	uint16_t i=0;
 	uint8_t IP1,IP2,IP3,IP4;
	//============================TEST Variables==================================

	//=========================END of TEST Variables==============================	
	LED_Init();
	Usart2_Init(38400);
	Myprintf_Init(0x00,myputc);
	SN74CBT3306_Init();
	//============================TEST============================================
	//if(GetIPv4Information(DIP_buf,&IP1,&IP2,&IP3,&IP4))	my_printf("\r\nCommand Line Error.");
	//my_printf("\r\nDevice IP=%d.%d.%d.%d",IP1,IP2,IP3,IP4);
	//============================END of TEST=====================================	

 	while(tapdev_init())	//ENC28J60 
	{								   
		GREEN_ON();  RED_OFF(); delay(8000*250); 
		RED_ON(); GREEN_OFF(); delay(8000*250);
	}
	GREEN_OFF();
	RED_OFF();

	uip_init();	//uIP initialize

 	uip_ipaddr(ipaddr, 192,168,1,71);	//setup local IP
	uip_sethostaddr(ipaddr);					    
	uip_ipaddr(ipaddr, 192,168,1,1); 	//setup Gateway
	uip_setdraddr(ipaddr);						 
	uip_ipaddr(ipaddr, 255,255,255,0);	//setup Mask
	uip_setnetmask(ipaddr);

	uip_listen(HTONS(1200));			//listen to port 1200, as TCP server
  	tcp_client_reconnect();	   		    //listem to remote port 1400, as client
  	my_printf("\r\nHardware initialize finished");
	Switch2ConnectMode();				//set to connect mode as default
	while (1) 
	{
		if((USART2_RX_STA>>15)==1) //if there's data ready
		{
			CMD = 0;
			CMD = SerialDataCheck(); //check if receive command from serial
			if(CONNECTION_STATUS==CONNECT_MODE) //in connect mode
			{
				if(CMD==CMD_POFF)	//only accept POFF command here
				{
					Switch2DisconnectMode();  delay(8000*10);
					my_printf("\r\nPOFF"); 	delay(8000*10);	
				}
			}
			else //in disconnect mode
			{
				switch(CMD)
				{
					//General Command Lines
					case CMD_POFF:
						Switch2DisconnectMode(); delay(8000*10);
						my_printf("\r\nPOFF"); delay(8000*10);
						break;
					case CMD_PON:
						my_printf("\r\nPON"); delay(8000*10);
						Switch2ConnectMode(); delay(8000*10);
						break;
					case CMD_PS:
						if(CONNECTION_STATUS==CONNECT_MODE)	my_printf("\r\nPON"); 
						else if(CONNECTION_STATUS==DISCONNECT_MODE)	my_printf("\r\nPOFF");
						break;	
					//IP Setup Command Lines
					case CMD_SETDIP:
						if(GetIPv4Information((char*)USART2_RX_BUF,&IP1,&IP2,&IP3,&IP4)) break; //data error
						uip_ipaddr(ipaddr, IP1,IP2,IP3,IP4);	//setup local IP
						uip_sethostaddr(ipaddr);	
						my_printf("\r\nDevice IP Set:%d.%d.%d.%d",IP1,IP2,IP3,IP4);
						break;
					case CMD_SETHIP:
						if(GetIPv4Information((char*)USART2_RX_BUF,&IP1,&IP2,&IP3,&IP4)) break; //data error
						serverip[0] = IP1;
						serverip[1] = IP2;
						serverip[2] = IP3;
						serverip[3] = IP4;
						tcp_client_reconnect(); 
						my_printf("\r\nHost IP Set:%d.%d.%d.%d",IP1,IP2,IP3,IP4);
						break;
					case CMD_SETGWAY:
						if(GetIPv4Information((char*)USART2_RX_BUF,&IP1,&IP2,&IP3,&IP4)) break; //data error
						uip_ipaddr(ipaddr, IP1,IP2,IP3,IP4); 	//setup Gateway
						uip_setdraddr(ipaddr);	
						my_printf("\r\nGateway Set:%d.%d.%d.%d",IP1,IP2,IP3,IP4);
						break;
					case CMD_SETMASK:
						if(GetIPv4Information((char*)USART2_RX_BUF,&IP1,&IP2,&IP3,&IP4)) break; //data error
						uip_ipaddr(ipaddr, IP1,IP2,IP3,IP4);	//setup Mask
						uip_setnetmask(ipaddr);
						my_printf("\r\nIP Mask Set:%d.%d.%d.%d",IP1,IP2,IP3,IP4);
						break;
					default:
						break;
					//Echo Command Lines		
				}		
			}
			for(i=0;i<USART2_MAX_RECV_LEN;i++)	USART2_RX_BUF[i] = 0x00; //clear buffer
			USART2_RX_STA = 0;	//clear usart2 rx flag and counter	
		}


		uip_polling();	//check uIP  mission every cycle
		if(tcp_server_tsta!=tcp_server_sta)//TCP Server status change
		{															 
 			if(tcp_server_sta&(1<<6))	//Rx new data
			{
    			CMD = 0;
    			CMD = ServerDataCheck(tcp_server_databuf);
    			switch(CMD)
    			{
					case CMD_POFF:
						Switch2DisconnectMode(); delay(8000*10);
						my_sprintf((char*)tcp_server_databuf,"\r\nPOFF");
						tcp_server_sta|=1<<5; //set tx data ready flag
						break;
					case CMD_PON:
						Switch2ConnectMode(); delay(8000*10);
						my_sprintf((char*)tcp_server_databuf,"\r\nPON");
						tcp_server_sta|=1<<5; //set tx data ready flag
	
						break;
					case CMD_PS:
						if(CONNECTION_STATUS==CONNECT_MODE) 
						{
							my_sprintf((char*)tcp_server_databuf,"\r\nPON");
						}
						else if(CONNECTION_STATUS==DISCONNECT_MODE)	
						{
							my_sprintf((char*)tcp_server_databuf,"\r\nPOFF");
						}
						tcp_server_sta|=1<<5; //set tx data ready flag
						break;	
					default:
						break;
    			}
				tcp_server_sta&=~(1<<6);		//clear Rx data ready flag	

			}
			tcp_server_tsta=tcp_server_sta; //update current server status
		}
		if(tcp_client_tsta!=tcp_client_sta)//TCP Client status change
		{																 
			//else my_printf("\r\nTCP Client Disconnected");
 			if(tcp_client_sta&(1<<6))	//Rx new data
			{
				tcp_client_sta&=~(1<<6);	//clear Rx data ready flag

			}
			tcp_client_tsta=tcp_client_sta;	//update current server status
		}
		//delay(1000);		
	}
}


//uIP event check
//need to be called eevery cycle of main process
void uip_polling(void)
{
	u8 i;
	static struct timer periodic_timer, arp_timer;
	static u8 timer_ok=0;	 
	if(timer_ok==0)//initialize timer at first time
	{
		timer_ok = 1;
		timer_set(&periodic_timer,CLOCK_SECOND/2); //creat a timer for 0.5 sec 
		timer_set(&arp_timer,CLOCK_SECOND*10);	   //creat a timer for 10 sec 
	}				 
	uip_len=tapdev_read();	//read an IP packet form ehternet.
							//get data length uip_len which defined in uip.c 
	if(uip_len>0) 			//if data exist
	{   
		//handle IP packet, only the verified packet is used 
		if(BUF->type == htons(UIP_ETHTYPE_IP))//IP packet? 
		{
			uip_arp_ipin();	//delete ethernet part
			uip_input();   	//IP packet handle
			//if data transmission is necessary
			//Tx data is in the uip_buf, length is uip_len	    
			if(uip_len>0)//response data is necessary
			{
				uip_arp_out();
				tapdev_send();
			}
		}else if (BUF->type==htons(UIP_ETHTYPE_ARP)) //is ARP packet?
		{
			uip_arp_arpin();
			//if data transmission is necessary
			//Tx data is in the uip_buf, length is uip_len	
 			if(uip_len>0)tapdev_send(); //send data via tapdev_send()	 
		}
	}else if(timer_expired(&periodic_timer))	//if 0.5sec timer expired
	{
		timer_reset(&periodic_timer);		//reset 0.5sec timer 
		//handle every TCP link  
		for(i=0;i<UIP_CONNS;i++)
		{
			uip_periodic(i);	//handle TCP mission  
			//if data transmission is necessary
			//Tx data is in the uip_buf, length is uip_len	    
			if(uip_len>0)//response data is necessary
			{
				uip_arp_out();
				tapdev_send();
			}
		}

		#if UIP_UDP	//if uIP_UDP enabled 
			//handle every UDP link 
			for(i=0;i<UIP_UDP_CONNS;i++)
			{
				uip_udp_periodic(i);	//handle UDP mission 
				//if data transmission is necessary
				//Tx data is in the uip_buf, length is uip_len	    
				if(uip_len>0)//response data is necessary
				{
					uip_arp_out();
					tapdev_send();
				}
			}
		#endif 

		//check if 10sec pass already, for updating ARP 
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}


//connect PS/2 and RS232 port
//set USART2 Tx/Rx as floating
void Switch2ConnectMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 

	//Rx
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_3;	// USART2 Rx (PA.3)								
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//Tx
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_2;	// USART2 Tx (PA.2)
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	//connect ports
	PORT_CONNECT();	GREEN_ON(); RED_OFF();
	CONNECTION_STATUS = CONNECT_MODE;
	delay(8000*10);
}

//disconnect PS/2 and RS232 port
//set USART2 Tx as AF_PP
//set USART2 Rx as floating
void Switch2DisconnectMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 

	//Rx
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_3;	// USART2 Rx (PA.3)								
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//Tx
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_2;	// USART2 Tx (PA.2)
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	//connect ports
	PORT_DISCONNECT();	GREEN_OFF(); RED_ON();
	CONNECTION_STATUS = DISCONNECT_MODE;
	delay(8000*10);
}
//check data in local server port
//return: CMD type
uint8_t ServerDataCheck(uint8_t* dataBuf)
{
	uint8_t res = 0;

	//find command
	//General Command Lines
	if(strncmp((const char *)str_POFF,(const char *)dataBuf,4)==0)	 res=CMD_POFF;
	else if(strncmp((const char *)str_PON,(const char *)dataBuf,3)==0) res=CMD_PON;
	else if(strncmp((const char *)str_PS,(const char *)dataBuf,2)==0)	 res=CMD_PS;
	else return 0;

	return res;
}

//check data in serial port
//return: CMD type
uint8_t SerialDataCheck(void)
{
	uint8_t res = 0;

	if((USART2_RX_STA>>15)==0)	return 0; //no data 
	else
	{
		//find command
		//General Command Lines
		if(strncmp((const char *)str_POFF,(const char *)USART2_RX_BUF,4)==0)	 res=CMD_POFF;
		else if(strncmp((const char *)str_PON,(const char *)USART2_RX_BUF,3)==0) res=CMD_PON;
		else if(strncmp((const char *)str_PS,(const char *)USART2_RX_BUF,2)==0)	 res=CMD_PS;
		//IP Setup Command Lines
		else if(strncmp((const char *)str_SETDIP,(const char *)USART2_RX_BUF,6)==0)	 	 res=CMD_SETDIP;
		else if(strncmp((const char *)str_SETHIP,(const char *)USART2_RX_BUF,6)==0)	 	 res=CMD_SETHIP;
		else if(strncmp((const char *)str_SETGWAY,(const char *)USART2_RX_BUF,7)==0)	 res=CMD_SETGWAY;
		else if(strncmp((const char *)str_SETMASK,(const char *)USART2_RX_BUF,7)==0)	 res=CMD_SETMASK;
		//Echo Command Lines
		else if(strncmp((const char *)str_ECHODIP,(const char *)USART2_RX_BUF,7)==0)	 res=CMD_ECHODIP;
		else if(strncmp((const char *)str_ECHOHIP,(const char *)USART2_RX_BUF,7)==0)	 res=CMD_ECHOHIP;
		else if(strncmp((const char *)str_ECHOGWAY,(const char *)USART2_RX_BUF,8)==0)	 res=CMD_ECHOGWAY;
		else if(strncmp((const char *)str_ECHOMASK,(const char *)USART2_RX_BUF,8)==0)	 res=CMD_ECHOMASK;
		else if(strncmp((const char *)str_ECHOCPORT,(const char *)USART2_RX_BUF,9)==0)	 res=CMD_ECHOCPORT;
		else if(strncmp((const char *)str_ECHOSPORT,(const char *)USART2_RX_BUF,9)==0)	 res=CMD_ECHOSPORT;

		return res;
	}
	return 0;
}

//get the IPv4 data information from command buffer
//cmd_buf: command buffer
//ip1~ip4: data in IPv4 format
//return: 1 = command error, 0 = success
uint8_t GetIPv4Information(char* cmd_buf, uint8_t* ptrIP1,uint8_t* ptrIP2,uint8_t* ptrIP3,uint8_t* ptrIP4)
{
 	char ip_temp[4][4] = {{'\0','\0','\0','\0'},{'\0','\0','\0','\0'},{'\0','\0','\0','\0'},{'\0','\0','\0','\0'}};
 	uint8_t tempip1,tempip2,tempip3,tempip4;
 	char* ptrBuf = 0x00;
 	uint8_t offset = 0;
 	uint8_t dot_counter = 0;
 	uint8_t digit_counter = 0;

	ptrBuf = cmd_buf;
	while(*ptrBuf!='=')
	{
		offset++; ptrBuf++;
		if(offset>=USART2_MAX_RECV_LEN) return 1; //command error
	}
	ptrBuf++; //point to first digit of IP1
	while(*ptrBuf!='\0')
	{
		if(*ptrBuf=='.')  //find '.'
		{
			ip_temp[dot_counter][digit_counter] = '\0'; //last one fill in '\0' as string end
			dot_counter++;
			digit_counter = 0;
			ptrBuf++;
			if(dot_counter>3) return 1; //command error
		}
		else //IP information
		{
			ip_temp[dot_counter][digit_counter] = *ptrBuf;
			digit_counter++;
			ptrBuf++;
		}
	}
	if (dot_counter<3) return 1; //data corrupt
	tempip1 = atoi(ip_temp[0]); 
	tempip2 = atoi(ip_temp[1]); 
	tempip3 = atoi(ip_temp[2]); 
	tempip4 = atoi(ip_temp[3]); 

	*ptrIP1 = tempip1;
	*ptrIP2 = tempip2;
	*ptrIP3 = tempip3;
	*ptrIP4 = tempip4;	
	return 0;
}

//get the monitored target port from command buffer
//cmd_buf: command buffer
//monport: monitored port
//return: 1 = command error, 0 = success
uint8_t GetMonitorPortInformation(char* cmd_buf, uint16_t* monport)
{
	char str_Port[4] = "";
 	char* ptrBuf = 0x00;
 	uint8_t offset = 0;
 	uint8_t digit_counter = 0;
 	uint16_t temp_port = 0;

	ptrBuf = cmd_buf;
	while(*ptrBuf!='=')
	{
		offset++; ptrBuf++;
		if(offset>=USART2_MAX_RECV_LEN) return 1; //command error
	}
	ptrBuf++; //point to first digit of port number
	while(*ptrBuf!='\0')
	{
		str_Port[digit_counter] = *ptrBuf;
		ptrBuf++;
		digit_counter++;
	}
	temp_port = atoi(str_Port);
	if ((temp_port>9999)||(temp_port<1)) return 1; //data corrupt, port number is 1~9999
	*monport = temp_port;
	return 0;
}
