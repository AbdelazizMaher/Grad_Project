

// Module header
#include "HELLOWORLD_TRANSMIT_task.h"
#include "ttrd2-05a-t0401a-v001a_uart2_buff_o_task.h"
#include <string.h>

static void osDelay(uint32_t usec);
static void HAL_sys_delay(uint32_t usec);

void HELLOWORLD_TRANSMIT_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStruct;
     
   // Enable GPIOD clock 
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
     
   // Configure port pin for the LED
   GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed; 
   GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_InitStruct.GPIO_Pin   = UART2_LED_PIN;
   
   GPIO_Init(UART2_LED_PORT, &GPIO_InitStruct); 

   GPIO_InitTypeDef GPIO_InitStructure;
   USART_InitTypeDef USART_InitStructure;

   // USART2 clock enable 
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

   // GPIOA clock enable 
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

   // GPIO config
   GPIO_InitStructure.GPIO_Pin   = UART2_TX_PIN; 
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(UART2_PORT, &GPIO_InitStructure);

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

   // USART2 configuration
   // - BaudRate as specified in function parameter
   // - Word Length = 8 Bits
   // - One Stop Bit
   // - No parity
   // - Hardware flow control disabled (RTS and CTS signals)
   // - Tx (only) enabled
   USART_InitStructure.USART_BaudRate = 230400;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Tx;
   USART_Init(USART2, &USART_InitStructure);

   // Enable UART2
   USART_Cmd(USART2, ENABLE);
	 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

  TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF-1;
  TIM_TimeBaseInitStruct.TIM_Prescaler = 1-1;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
	
	TIM_Cmd( TIM1, ENABLE);	 
	 
}

uint32_t HELLOWORLD_TRANSMIT_Update(void)
{
	//if(GPIO_ReadInputDataBit(GPIOA,BUTTON1_PIN))
	{

		//GPIO_WriteBit(UART2_LED_PORT,UART2_LED_PIN,Bit_SET);
	}	
//	char *user_data = "Hello World\r\n";

//	while(GPIO_ReadInputDataBit(GPIOA,BUTTON1_PIN) == Bit_RESET)
//	{
//	}
//		
//	  for(uint32_t i=0; i < strlen(user_data); i++)
//	   {
//		 while ( USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET)
//		 {
//		 };
//	      USART_SendData(USART2,(uint16_t)user_data[i]);
//     }	
//	   while ( USART_GetFlagStatus(USART2,USART_FLAG_TC) != SET);
//		GPIO_WriteBit(UART2_LED_PORT,UART2_LED_PIN,Bit_SET);
//	//}	
    return RETURN_NORMAL_STATE;	  
}


void osDelay(uint32_t usec)
{
	uint32_t delay = usec * SystemCoreClock/100000;
	TIM_SetCounter(TIM1, 0 );
	while(TIM_GetCounter(TIM1) < delay);

}
 
void HAL_sys_delay(uint32_t usec) {
	osDelay(usec);
}

