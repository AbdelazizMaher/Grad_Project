/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include <string.h>

static void RCC_Config(void);
static void GPIO_Config(void);

static uint32_t old_pos;
static	uint32_t pos;


DMA_InitTypeDef DMA_InitStructure;
USART_InitTypeDef USART_InitStructure;

void UART_PROCESS_DATA(const void* data, size_t len);

#define RXBUFFERSIZE    20
uint8_t RxBuffer[RXBUFFERSIZE];

int main(void)
{
	 RCC_Config();
	 GPIO_Config();

   USART_InitStructure.USART_BaudRate = 115200;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(USART2, &USART_InitStructure);
   USART_Cmd(USART2, ENABLE);

   DMA_DeInit(DMA1_Stream5);
   DMA_InitStructure.DMA_BufferSize = 10;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Channel = DMA_Channel_4 ;
   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)RxBuffer;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
 // DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   //DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA1_Stream5, &DMA_InitStructure);
	 USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	 DMA_Cmd(DMA1_Stream5,ENABLE);

   for(;;)
   {
	   	pos = ARRAY_LEN(RXBUFFERSIZE) - DMA_GetCurrDataCounter(DMA1_Stream5);
	   	   if (pos != old_pos) {                       /* Check change in received data */
	           if (pos > old_pos) {                    /* Current position is over previous one */
	               /*
	                * Processing is done in "linear" mode.
	                *
	                * Application processing is fast with single data block,
	                * length is simply calculated by subtracting pointers
	                *
	                * [   0   ]
	                * [   1   ] <- old_pos |------------------------------------|
	                * [   2   ]            |                                    |
	                * [   3   ]            | Single block (len = pos - old_pos) |
	                * [   4   ]            |                                    |
	                * [   5   ]            |------------------------------------|
	                * [   6   ] <- pos
	                * [   7   ]
	                * [ N - 1 ]
	                */
	        	   UART_PROCESS_DATA(&RxBuffer[old_pos], pos - old_pos);
	           }
	           old_pos = pos;                          /* Save current position as old for next transfers */
	       }
   }
	for(;;);
}


static void RCC_Config(void)
 {
   /* Enable GPIO clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Enable USARTs Clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
   /* Enable the DMA periph */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
 }

static void GPIO_Config(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
 }

void UART_PROCESS_DATA(const void* data, size_t len)
{
	   const uint8_t* d = data;

   for ( ;len > 0;len--, ++d)
      {
		 while ( USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET)
		 {
		 };
	      USART_SendData(USART2,(uint16_t) *d);
      }
	   while ( USART_GetFlagStatus(USART2,USART_FLAG_TC) != SET);
   for ( ;len > 0;len--, ++d)
   {

   }

}

