
//*************************************************//
/*Project: Voltage Data Logger										*/
/*Author: Ajay Rao                   		   				*/
/*Affiliation: NCSU                   		   			*/
/*Version: 1.0																		*/
/*E-mail:abrao2@ncsu.edu                      		*/
//*************************************************//

#include <MKL25Z4.H>
#include <stdio.h>
#include "gpio_defs.h"
#include "UART.h"
#include "LEDs.h"
uint16_t samples[6000];
uint16_t res;
int i=0,o=0;

void Init_ADC(void) {
	
	SIM->SCGC6 |= (1UL << SIM_SCGC6_ADC0_SHIFT); 
	ADC0->CFG1 = ADC_CFG1_ADLPC_MASK | ADC_CFG1_ADIV(0) | ADC_CFG1_ADICLK(0) | 
	ADC_CFG1_ADLSMP_MASK | ADC_CFG1_MODE(3);
	ADC0->SC2 = ADC_SC2_REFSEL(0);
	ADC0->CFG2 = (1UL <<ADC_CFG2_MUXSEL_SHIFT);
	PMC->REGSC |= PMC_REGSC_BGBE_MASK; 
}
void GPIO_Init()
{
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[0] |= PORT_PCR_MUX(1);          
	PORTB->PCR[1] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[1] |= PORT_PCR_MUX(1); 
	PORTB->PCR[2] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[2] |= PORT_PCR_MUX(1); 
	PORTB->PCR[3] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[3] |= PORT_PCR_MUX(1); 
	PTB->PDDR |= MASK(0) | MASK (1)| MASK(2)| MASK(3);
	PTB->PCOR = MASK(0);
	PTB->PCOR = MASK(1);
	PTB->PCOR = MASK(2);
	PTB->PCOR = MASK(3);
}

int main (void) 
	
{
	uint8_t buffer[80], c, * bp;
	char SampleCount[20], SamplePeriod[20], ChannelNumber[20];
	char x,y,z,s,r;
	int Count, Channel;
	uint32_t Period;
	float temp;
  char State = '0',option;

	
	//Initialize all the submodules.
	Init_UART0(115200);
	GPIO_Init();
	Init_ADC();
	Init_RGB_LEDs();
	
	while(1)
	{
		while(State == '0') //Master CLI State
		{
			printf("*****VOLTAGE LOGGER*****\r\n");
		  Control_RGB_LEDs(0,0,1); // CLI BLUE LED
			PTB->PSOR = 0x00000001;
			PTB->PCOR = 0x0000000E;
			printf("Select the Options\r\n");
			printf("1. Retrieve Data\r\n");
			printf("2. New Samples to be collected\r\n");
			printf("******************************************\r\n");
			scanf("%c",&option);
			if(option=='1')
			{
				State='1';
				break;
			}
			if(option=='2')
			{
				State='2';
				break;
			}
		}
		
		while(State == '1')
		{
			printf("Press S to retrieve data that was sampled\r\n");
			scanf("%c",&s);
			if(s== 'R' | 'r')
			{
				Control_RGB_LEDs(1,0,1); //Sending Data.
				PTB->PSOR = 0x00000008;
				PTB->PCOR = 0x00000007;
				printf("Sending data (%d Samples) \r\n",Count);
				if(Channel != 27)
				{
					for(o=0;o<Count;o++)
					{
					temp = (float)((samples[o]*3.3)/65535);				
					printf("%d: %6.4f\r\n",o+1,temp);
					}
				}
				else{for(o=0;o<Count;o++)
					{
					temp = (float)((1.0/samples[o])*65536);				
					printf("%d: %6.4f\r\n",o+1,temp);
					}
				}
					
				State = '0';
			}
		}
		while(State == '2')
		{
			Control_RGB_LEDs(0,0,1);
			PTB->PSOR = 0x00000001;
			PTB->PCOR = 0x0000000E;
//Enter the Sample Period
			printf("Enter the Sampling Period in the format PX (where X is 1-60000)\r\n");
			scanf("%s",SamplePeriod);
			sscanf(SamplePeriod, "%c %d", &x, &Period);
			if(!(x=='P' && Period>0 && Period<60000))
			{
				printf("Invalid Data Entered\r\n");
				State='2';
				break;
			}
			printf("Sampling Period is %d \r\n", Period);
//Samples Count to be collected
			printf("Enter the Samples Count in the format NX (where X is 1-6000)\r\n");
			scanf("%s",SampleCount);
			sscanf(SampleCount, "%c %d", &y, &Count);
			if(!(y=='N' && Count>0 && Count<6000))
			{
				printf("Invalid Data Entered\r\n");
				State='2';
				break;
			}
			printf("Sampling Count is %d \r\n", Count);
//Analog Source Channel						
			printf("Enter the Analog Source Channel in the format CX\r\n");
			scanf("%s",ChannelNumber);
			sscanf(ChannelNumber, "%c %d", &z, &Channel);
			if(!(z=='C'))
			{
				printf("Invalid Data Entered\r\n");
				State='2';
				break;
			}
			printf("Channel selected is %d \r\n", Channel);
			State='3';
			break;
		}
			while(State=='3')
			{
				printf("Erasing previous samples\r\n");
				for(o=0;o<Count;o++)
					{
						samples[o] = 0;
					}
				printf("Press R to begin recording\r\n");
				scanf("%c",&r);
				if(r == 'R' | 'r')
				{
					Control_RGB_LEDs(0,1,0);
					PTB->PSOR = 0x00000002;
					PTB->PCOR = 0x0000000D;
					for(o=0;o<Count;o++)
					{
						Control_RGB_LEDs(0,1,0);
						PTB->PSOR = 0x00000002;
						PTB->PCOR = 0x0000000D;
						ADC0->SC1[0] = Channel; // start conversion on channel	
						while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
						;
						samples[o] = ADC0->R[0];
						
						PTB->PSOR = 0x00000004;
						PTB->PCOR = 0x0000000B;
						Control_RGB_LEDs(1,1,0);
						Delay(Period);
					}
					}
			State = '0';
				}			
			
		}
	}
	
			
	



