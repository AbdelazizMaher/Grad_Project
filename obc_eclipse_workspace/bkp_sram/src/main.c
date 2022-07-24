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

#define false 0
#define true 1
#define EV_MAX_BUFFER  1024
#define EV_BUFFER_PART 205
#define WOD_MAX_BUFFER 224
#define MS_MAX_FILES            5000

#define C_ASSERT(e)    ((e) ? (true) : (tst_debugging((uint8_t *)__FILE__, __LINE__, #e)))

struct _obc_data
{
    uint16_t obc_seq_cnt;
    uint16_t *fs_wod_head;
    uint16_t *fs_ext_head;
    uint16_t *fs_ev_head;
    uint16_t *fs_wod_tail;
    uint16_t *fs_ext_tail;
    uint16_t *fs_ev_tail;
    uint16_t *fs_fotos;

    uint16_t *comms_boot_cnt;
    uint16_t *eps_boot_cnt;
    uint32_t *comms_tick;
    uint32_t *eps_tick;

    uint8_t *log;
    uint32_t *log_cnt;
    uint32_t *log_state;
    uint8_t *wod_log;
    uint32_t *wod_cnt;

    uint16_t vbat;
    uint32_t adc_time;
    uint8_t adc_flag;

};

struct _sys_data {
    uint8_t rsrc;
    uint32_t *boot_counter;
};

struct _obc_data obc_data = { .vbat = 0,
                              .adc_time = 0,
                              .adc_flag = false };

static struct _sys_data sys_data;


void HAL_obc_enableBkUpAccess();
uint32_t * HAL_obc_BKPSRAM_BASE();
void bkup_sram_INIT();
uint8_t tst_debugging(uint8_t* file, uint32_t l, char *e);

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
int main(void)
{

	   HAL_obc_enableBkUpAccess();
	   bkup_sram_INIT();

	for(;;);
}

void HAL_obc_enableBkUpAccess() {
 // HAL_PWR_EnableBkUpAccess();
  //HAL_PWREx_EnableBkUpReg();
  //__HAL_RCC_BKPSRAM_CLK_ENABLE();
	PWR_BackupAccessCmd(ENABLE);
	PWR_BackupRegulatorCmd(ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
}

uint32_t * HAL_obc_BKPSRAM_BASE() {
  return (uint32_t *)BKPSRAM_BASE;
}


void bkup_sram_INIT() {

    uint8_t *base_pointer = (uint8_t*)HAL_obc_BKPSRAM_BASE();

    obc_data.log_cnt = (uint32_t*)base_pointer;
    base_pointer += sizeof(uint32_t);

    obc_data.log_state = (uint32_t*)base_pointer;
    base_pointer += sizeof(uint32_t);

    sys_data.boot_counter = (uint32_t*)base_pointer;
    base_pointer += sizeof(uint32_t);

    obc_data.wod_cnt = (uint32_t*)base_pointer;
    base_pointer += sizeof(uint32_t);

    obc_data.comms_boot_cnt = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.eps_boot_cnt = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.comms_tick = (uint32_t*)base_pointer;
    base_pointer += sizeof(uint32_t);

    obc_data.eps_tick = (uint32_t*)base_pointer;
    base_pointer += sizeof(uint32_t);

    obc_data.fs_wod_head = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.fs_ext_head = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.fs_ev_head = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.fs_wod_tail = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.fs_ext_tail = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.fs_ev_tail = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.fs_fotos = (uint16_t*)base_pointer;
    base_pointer += sizeof(uint16_t);

    obc_data.log = (uint8_t *)base_pointer;
    base_pointer += sizeof(uint8_t);

    obc_data.wod_log = (uint8_t *)base_pointer + (EV_MAX_BUFFER);
    base_pointer += sizeof(uint8_t);

    if(!C_ASSERT(*obc_data.log_cnt < EV_MAX_BUFFER) == true)      { *obc_data.log_cnt = 0; }
    if((!C_ASSERT(*obc_data.wod_cnt < EV_MAX_BUFFER)) == true)      { *obc_data.wod_cnt = 0; }
    if((!C_ASSERT(*obc_data.fs_wod_head < MS_MAX_FILES)) == true)   { *obc_data.fs_wod_head = 1; }
    if((!C_ASSERT(*obc_data.fs_ext_head < MS_MAX_FILES)) == true)   { *obc_data.fs_ext_head = 1; }
    if((!C_ASSERT(*obc_data.fs_ev_head < MS_MAX_FILES)) == true)    { *obc_data.fs_ev_head = 1; }
    if((!C_ASSERT(*obc_data.fs_wod_tail < MS_MAX_FILES)) == true)   { *obc_data.fs_wod_tail = 1; }
    if((!C_ASSERT(*obc_data.fs_ext_tail < MS_MAX_FILES)) == true)   { *obc_data.fs_ext_tail = 1; }
    if((!C_ASSERT(*obc_data.fs_ev_tail < MS_MAX_FILES)) == true)    { *obc_data.fs_ev_tail = 1; }
    if((!C_ASSERT(*obc_data.fs_fotos < MS_MAX_FILES)) == true)      { *obc_data.fs_fotos = 1; }

}

uint8_t tst_debugging(uint8_t* file, uint32_t l, char *e) {

  //assertion_last_file = fi;
  //assertion_last_line = l;

 //  UART2_BUF_O_Write_String_To_Buffer("assert_failed\n");
 //  UART2_BUF_O_Write_String_To_Buffer((char*) file);
   //UART2_BUF_O_Write_String_To_Buffer("  Line:  ");
   //UART2_BUF_O_Write_Number03_To_Buffer(l);
   //UART2_BUF_O_Write_String_To_Buffer("\n");
   //UART2_BUF_O_Send_All_Data();

  return false;
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

