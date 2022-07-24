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


#define COMMS_U4_PORT_RCC		 	   RCC_AHB1Periph_GPIOA
#define COMMS_U4_UART_RCC			   RCC_APB1Periph_UART4
#define COMMS_U4_PORT        	       GPIOA
#define COMMS_U4_UARTX                 UART4
#define COMMS_U4_AF		               GPIO_AF_UART4
#define COMMS_U4_TX_PIN      	       GPIO_Pin_0
#define COMMS_U4_RX_PIN      	       GPIO_Pin_1
#define COMMS_U4_TX_PIN_SOURCE	       GPIO_PinSource0
#define COMMS_U4_RX_PIN_SOURCE	       GPIO_PinSource1

// Index of data in the buffer that has not yet been sent (with inv. copy)
static uint32_t Wait_idx_g  = 0;
static uint32_t Wait_idx_ig = ~0;

static uint32_t Sent_idx_g  = 0;
static uint32_t Sent_idx_ig = ~0;

#define TX_BUFFER_SIZE_BYTES 500


static char Tx_buffer_g[TX_BUFFER_SIZE_BYTES];
static char Tx_buffer_ig[TX_BUFFER_SIZE_BYTES];  // Inverted copy

void UART2_BUF_O_Send_Char(const char);
void     UART2_BUF_O_Send_All_Data(void);
void     UART2_BUF_O_Write_String_To_Buffer(const char* const);
void     UART2_BUF_O_Write_Char_To_Buffer(const char);
void     UART2_BUF_O_Write_Number02_To_Buffer(const uint32_t DATA);
void UART2_BUF_O_Write_Number10_To_Buffer(const uint32_t DATA);
void UART2_BUF_O_Write_Number03_To_Buffer(const uint32_t DATA);
static void RCC_Config(void);
static void GPIO_Config(void);
static void USART_Config(void);
int main(void)
{
	 RCC_Config();
	 GPIO_Config();
	 USART_Config();

		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		RCC_APB2PeriphClockCmd(COMMS_U4_UART_RCC, ENABLE);

		// GPIOA clock enable
		RCC_AHB1PeriphClockCmd(COMMS_U4_PORT_RCC, ENABLE);

		GPIO_PinAFConfig(COMMS_U4_PORT, COMMS_U4_RX_PIN_SOURCE, COMMS_U4_AF);
		GPIO_PinAFConfig(COMMS_U4_PORT, COMMS_U4_TX_PIN_SOURCE, COMMS_U4_AF);
		GPIO_InitStructure.GPIO_Pin   = COMMS_U4_TX_PIN|COMMS_U4_RX_PIN;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_Init(COMMS_U4_PORT, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(COMMS_U4_UARTX, &USART_InitStructure);

		USART_Cmd(COMMS_U4_UARTX, ENABLE);


		UART2_BUF_O_Write_String_To_Buffer("nothing man\n");
		UART2_BUF_O_Write_String_To_Buffer("nothing man\n");
		UART2_BUF_O_Send_All_Data();
		UART2_BUF_O_Write_String_To_Buffer("nothing man\n");
		UART2_BUF_O_Send_All_Data();

	for(;;);
}

static void RCC_Config(void)
{
   /* Enable GPIO clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Enable USARTs Clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
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

static void USART_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

void UART2_BUF_O_Write_String_To_Buffer(const char* const STR_PTR)
   {
   uint32_t i = 0;

   while (STR_PTR[i] != '\0')
      {
      UART2_BUF_O_Write_Char_To_Buffer(STR_PTR[i]);
      i++;
      }
   }

void UART2_BUF_O_Write_Char_To_Buffer(const char CHARACTER)
   {
   // Write to the buffer *only* if there is space
   if (Wait_idx_g < TX_BUFFER_SIZE_BYTES)
      {
      Tx_buffer_g[Wait_idx_g] = CHARACTER;
      Tx_buffer_ig[Wait_idx_g] = (char) ~CHARACTER;
      Wait_idx_g++;
      }
   else
      {
      // Write buffer is full
      // No error handling / reporting here (characters may be lost)
      // Adapt as required to meet the needs of your application
      }

   // Update the copy
   Wait_idx_ig = ~Wait_idx_g;
   }

void UART2_BUF_O_Write_Number02_To_Buffer(const uint32_t DATA)
   {
   char Digit[3];

   if (DATA <= 99)
      {
      __disable_irq();
      Digit[2] = '\0';  // Null terminator
      Digit[1] = 48 + (DATA % 10);
      Digit[0] = 48 + ((DATA/10) % 10);
      __enable_irq();
      }

   UART2_BUF_O_Write_String_To_Buffer(Digit);
   }

void UART2_BUF_O_Send_Char(const char CHARACTER)
   {

   while(!(USART_GetFlagStatus(USART2, USART_FLAG_TXE)));

   USART_SendData(USART2, CHARACTER);
   }

void UART2_BUF_O_Send_All_Data(void)
   {

   while (Sent_idx_g < Wait_idx_g)
      {
      UART2_BUF_O_Send_Char(Tx_buffer_g[Sent_idx_g]);

      Sent_idx_g++;
      Sent_idx_ig = ~Sent_idx_g;
      }

   // Reset the indices
   Sent_idx_g = 0;
   Sent_idx_ig = ~0;
   Wait_idx_g = 0;
   Wait_idx_ig = ~0;
   }

void UART2_BUF_O_Write_Number10_To_Buffer(const uint32_t DATA)
   {
   char Digit[11];

   __disable_irq();
   Digit[10] = '\0';  // Null terminator
   Digit[9]  = 48 + (DATA % 10);
   Digit[8]  = 48 + ((DATA/10) % 10);
   Digit[7]  = 48 + ((DATA/100) % 10);
   Digit[6]  = 48 + ((DATA/1000) % 10);
   Digit[5]  = 48 + ((DATA/10000) % 10);
   Digit[4]  = 48 + ((DATA/100000) % 10);
   Digit[3]  = 48 + ((DATA/1000000) % 10);
   Digit[2]  = 48 + ((DATA/10000000) % 10);
   Digit[1]  = 48 + ((DATA/100000000) % 10);
   Digit[0]  = 48 + ((DATA/1000000000) % 10);
   __enable_irq();

   UART2_BUF_O_Write_String_To_Buffer(Digit);
   }

void UART2_BUF_O_Write_Number03_To_Buffer(const uint32_t DATA)
   {
   char Digit[4];

   if (DATA <= 999)
      {
      __disable_irq();
      Digit[3] = '\0';  // Null terminator
      Digit[2] = 48 + (DATA % 10);
      Digit[1] = 48 + ((DATA/10) % 10);
      Digit[0] = 48 + ((DATA/100) % 10);
      __enable_irq();

      UART2_BUF_O_Write_String_To_Buffer(Digit);
      }
   }

