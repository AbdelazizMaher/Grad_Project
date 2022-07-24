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

static void RCC_Config(void);
static void GPIO_Config(void);
static void USART_Config();

#define UART_BUF_SIZE 4096
#define MAX_PKT_SIZE  2063
#define ECSS_HEADER_SIZE        6
#define ECSS_DATA_HEADER_SIZE   4
#define ECSS_CRC_SIZE           2
#define ECSS_DATA_OFFSET        10  /*ECSS_HEADER_SIZE + ECSS_DATA_HEADER_SIZE*/
#define ECSS_VER_NUMBER             0
#define ECSS_DATA_FIELD_HDR_FLG     1
#define ECSS_PUS_VER            1
#define ECSS_SEC_HDR_FIELD_FLG  0
#define TC 1
#define TM 0
#define TC_ACK_NO           0x00
#define TC_TM_SEQ_SPACKET 0x03
#define TC_HOUSEKEEPING_SERVICE         3
#define TC_HK_REPORT_PARAMETERS         21
#define _EPS_APP_ID_   2
#define _OBC_APP_ID_   1
#define _COMMS_APP_ID_ 4
#define SYSTEM_APP_ID _OBC_APP_ID_
#define HLDLC_START_FLAG        0x7E
#define HLDLC_CONTROL_FLAG      0x7D
#define POOL_PKT_SIZE        16
#define MAX_PKT_DATA        198
#define true	1
#define NULL ((void *)0)

typedef enum {
    SATR_PKT_ILLEGAL_APPID     = 0,
    SATR_PKT_INV_LEN           = 1,
    SATR_PKT_INC_CRC           = 2,
    SATR_PKT_ILLEGAL_PKT_TP    = 3,
    SATR_PKT_ILLEGAL_PKT_STP   = 4,
    SATR_PKT_ILLEGAL_APP_DATA  = 5,
    SATR_OK                    = 6,
    SATR_ERROR                 = 7,
    SATR_EOT                   = 8,
    SATR_PKT_ILLEGAL_ACK       = 10,
    SATR_ALREADY_SERVICING     = 11,
    SATR_PKT_INIT              = 13,
    SATR_INV_STORE_ID          = 14,
    SATR_INV_DATA_LEN          = 15
}SAT_returnState;

#define _EPS_APP_ID_   2
typedef enum {
	OBC_APP_ID      = _OBC_APP_ID_,
	COMMS_APP_ID    = _COMMS_APP_ID_,
    EPS_APP_ID      = _EPS_APP_ID_
}TC_TM_app_id;

struct uart_data {
    uint8_t uart_buf[UART_BUF_SIZE];
    uint8_t uart_unpkt_buf[UART_BUF_SIZE];
    uint8_t deframed_buf[MAX_PKT_SIZE];
    uint8_t uart_pkted_buf[UART_BUF_SIZE];
    uint8_t framed_buf[UART_BUF_SIZE];
    uint16_t uart_size;
    uint32_t last_com_time;
    uint32_t init_time;
};
struct _obc_data
{
    struct uart_data eps_uart;
    struct uart_data comms_uart;
};
struct _obc_data obc_data = {
   .eps_uart.init_time = 0,
   .comms_uart.init_time = 0
};



union _cnv {
    double cnvD;
    float cnvF;
    uint32_t cnv32;
    uint16_t cnv16[4];
    uint8_t cnv8[8];
};


struct _pkt_state {
    uint8_t seq_cnt[8];
};

static struct _pkt_state pkt_state;

typedef struct {

    TC_TM_app_id app_id;
    uint8_t type;

    uint8_t seq_flags;
    uint16_t seq_count;

    uint16_t len;

    uint8_t ack;
    uint8_t ser_type;
    uint8_t ser_subtype;

    TC_TM_app_id dest_id;

    uint8_t *data;

    SAT_returnState verification_state;
}tc_tm_pkt;
typedef enum {
    HEALTH_REP      = 1
}HK_struct_id;

static tc_tm_pkt hk_pkt;
static uint8_t hk_pkt_data[264];
#define POOL_PKT_TOTAL_SIZE  20
struct _queue {
    tc_tm_pkt *fifo[POOL_PKT_TOTAL_SIZE];
    uint8_t head;
    uint8_t tail;
};


//static struct _queue queueEPS = { .head = 0, .tail = 0};
static struct _queue queueCOMMS = { .head = 0, .tail = 0};

struct _pkt_pool{
    tc_tm_pkt pkt[POOL_PKT_SIZE];
    uint8_t free[POOL_PKT_SIZE];
    uint32_t time[POOL_PKT_SIZE];
    uint8_t data[POOL_PKT_SIZE][MAX_PKT_DATA];
    uint32_t time_delta[POOL_PKT_SIZE];
};

static struct _pkt_pool pkt_pool;

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

SAT_returnState hk_crt_pkt_TC(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);
SAT_returnState crt_pkt(tc_tm_pkt *pkt, TC_TM_app_id app_id, uint8_t type, uint8_t ack, uint8_t ser_type, uint8_t ser_subtype, TC_TM_app_id dest_id);
SAT_returnState route_pkt(tc_tm_pkt *pkt);
SAT_returnState queuePush(tc_tm_pkt *pkt, TC_TM_app_id app_id);
SAT_returnState export_pkt(TC_TM_app_id app_id, struct uart_data *data);
void stats_outbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt);
SAT_returnState HLDLC_frame(uint8_t *buf_in, uint8_t *buf_out, uint16_t *size);
SAT_returnState pack_pkt(uint8_t *buf, tc_tm_pkt *pkt, uint16_t *size);
void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size);
SAT_returnState checkSum(const uint8_t *data, const uint16_t size, uint8_t *res_crc);
SAT_returnState free_pkt(tc_tm_pkt *pkt);
void MYDMA_Init(DMA_Stream_TypeDef* DMA_Streamx , uint32_t chx , uint32_t padd , uint32_t madd , uint16_t ndtr);
void MYDMA_Enable(USART_TypeDef *USARTx,DMA_Stream_TypeDef* DMA_Streamx, uint16_t ndtr);
SAT_returnState hk_app(tc_tm_pkt *pkt);
tc_tm_pkt * queuePop(TC_TM_app_id app_id);
int main(void)
{
	 RCC_Config();
	 GPIO_Config();
     USART_Config();
	 hk_pkt.data = hk_pkt_data;
	 hk_crt_pkt_TC(&hk_pkt, COMMS_APP_ID, HEALTH_REP);
	 route_pkt(&hk_pkt);
	 export_pkt(COMMS_APP_ID, &obc_data.comms_uart);



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

void USART_Config()
{
	USART_InitTypeDef USART_InitStructure;
	   USART_InitStructure.USART_BaudRate = 9600;
	   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	   USART_InitStructure.USART_StopBits = USART_StopBits_1;
	   USART_InitStructure.USART_Parity = USART_Parity_No;
	   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	   USART_InitStructure.USART_Mode = USART_Mode_Tx;
	   USART_Init(USART2, &USART_InitStructure);
	   USART_Cmd(USART2, ENABLE);
}

SAT_returnState hk_crt_pkt_TC(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid) {

    crt_pkt(pkt, app_id, TC, TC_ACK_NO, TC_HOUSEKEEPING_SERVICE, TC_HK_REPORT_PARAMETERS, (TC_TM_app_id)SYSTEM_APP_ID);

    pkt->data[0] = (char)sid;
    pkt->len = 1;

    return SATR_OK;
}
SAT_returnState crt_pkt(tc_tm_pkt *pkt, TC_TM_app_id app_id, uint8_t type, uint8_t ack, uint8_t ser_type, uint8_t ser_subtype, TC_TM_app_id dest_id) {


    pkt->type = type;
    pkt->app_id = app_id;
    pkt->dest_id = dest_id;
    pkt->ser_type = ser_type;
    pkt->ser_subtype = ser_subtype;


    return SATR_OK;
}
SAT_returnState route_pkt(tc_tm_pkt *pkt) {

    TC_TM_app_id id;
    SAT_returnState res;

    if(pkt->type == TC)         { id = pkt->app_id; }
    else if(pkt->type == TM)    { id = pkt->dest_id; }
    if(id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
            //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
            res = hk_app(pkt);}

    else if(id == COMMS_APP_ID) {
        queuePush(pkt, COMMS_APP_ID);
    }

    return SATR_OK;
}

tc_tm_pkt * queuePop(TC_TM_app_id app_id) {

	  tc_tm_pkt *pkt;
	  if(app_id == COMMS_APP_ID)
     {
       if(queueCOMMS.head == queueCOMMS.tail) { return 0; }
       pkt = queueCOMMS.fifo[queueCOMMS.tail];
       queueCOMMS.tail = (queueCOMMS.tail + 1) % POOL_PKT_TOTAL_SIZE;
		 }

    return pkt;
}

SAT_returnState export_pkt(TC_TM_app_id app_id, struct uart_data *data) {

    tc_tm_pkt *pkt = 0;
    uint16_t size = 0;
    SAT_returnState res = SATR_ERROR;
    if((pkt = queuePop(app_id)) ==  NULL) { return SATR_OK; }
    stats_outbound(pkt->type, pkt->app_id, pkt->dest_id, pkt);

    pack_pkt(data->uart_pkted_buf,  pkt, &size);

    res = HLDLC_frame(data->uart_pkted_buf, data->framed_buf, &size);
    if(res == SATR_ERROR) { return SATR_ERROR; }


    HAL_uart_tx(app_id, data->framed_buf, size);

    free_pkt(pkt);

    return SATR_OK;
}

void stats_outbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt) {

    TC_TM_app_id source = 0;
    TC_TM_app_id dest = 0;

    if(type == TC) {
        source = app_id;
        dest = dest_id;
    }
    else if(type == TM) {
        dest = app_id;
        source = dest_id;
    }
}

SAT_returnState pack_pkt(uint8_t *buf, tc_tm_pkt *pkt, uint16_t *size) {

    union _cnv cnv;
    uint16_t buf_pointer;

     cnv.cnv16[0] = pkt->app_id;

    buf[0] = ( ECSS_VER_NUMBER << 5 | pkt->type << 4 | ECSS_DATA_FIELD_HDR_FLG << 3 | cnv.cnv8[1]);
    buf[1] = cnv.cnv8[0];

    /*if the pkt was created in OBC, it updates the counter*/
    if(pkt->type == TC && pkt->dest_id == SYSTEM_APP_ID)      { pkt->seq_count = pkt_state.seq_cnt[pkt->app_id]++; }
    else if(pkt->type == TM && pkt->app_id == SYSTEM_APP_ID)  { pkt->seq_count = pkt_state.seq_cnt[pkt->dest_id]++; }
    pkt->seq_flags = TC_TM_SEQ_SPACKET;
    cnv.cnv16[0] = pkt->seq_count;
    buf[2] = (pkt->seq_flags << 6 | cnv.cnv8[1]);
    buf[3] = cnv.cnv8[0];

    if(pkt->type == TM) {
        buf[6] = ECSS_PUS_VER << 4 ;
    } else if(pkt->type == TC) {
        buf[6] = ( ECSS_SEC_HDR_FIELD_FLG << 7 | ECSS_PUS_VER << 4 | pkt->ack);
    }

    buf[7] = pkt->ser_type;
    buf[8] = pkt->ser_subtype;
    buf[9] = pkt->dest_id; /*source or destination*/

    buf_pointer = ECSS_DATA_OFFSET;

    for(int i = 0; i < pkt->len; i++) {
        buf[buf_pointer++] = pkt->data[i];
    }

    pkt->len += ECSS_DATA_HEADER_SIZE + ECSS_CRC_SIZE - 1;

    /*check if this is correct*/
    cnv.cnv16[0] = pkt->len;
    buf[4] = cnv.cnv8[1];
    buf[5] = cnv.cnv8[0];

    /*added it for ecss conformity, checksum in the ecss is defined to have 16 bits, we only use 8*/
    buf[buf_pointer++] = 0;
    checkSum(buf, buf_pointer-2, &buf[buf_pointer]);
    *size = buf_pointer + 1;

    return SATR_OK;
}

SAT_returnState HLDLC_frame(uint8_t *buf_in, uint8_t *buf_out, uint16_t *size) {

    uint16_t cnt = 2;

    for(uint16_t i = 0; i < *size; i++) {
        if(i == 0) {
            buf_out[0] = HLDLC_START_FLAG;
            buf_out[1] = buf_in[0];
        } else if(i == (*size) - 1) {
            if(buf_in[i] == HLDLC_START_FLAG) {
                buf_out[cnt++] = HLDLC_CONTROL_FLAG;
                buf_out[cnt++] = 0x5E;
            } else if(buf_in[i] == HLDLC_CONTROL_FLAG) {
                buf_out[cnt++] = HLDLC_CONTROL_FLAG;
                buf_out[cnt++] = 0x5D;
            } else { buf_out[cnt++] = buf_in[i]; }
            buf_out[cnt++] = HLDLC_START_FLAG;
            *size = cnt;
            return SATR_EOT;
        } else if(buf_in[i] == HLDLC_START_FLAG) {
            buf_out[cnt++] = HLDLC_CONTROL_FLAG;
            buf_out[cnt++] = 0x5E;
        } else if(buf_in[i] == HLDLC_CONTROL_FLAG) {
            buf_out[cnt++] = HLDLC_CONTROL_FLAG;
            buf_out[cnt++] = 0x5D;
        } else {
            buf_out[cnt++] = buf_in[i];
        }

    }

    return SATR_ERROR;
}

SAT_returnState checkSum(const uint8_t *data, const uint16_t size, uint8_t *res_crc) {

    *res_crc = 0;
    for(int i=0; i<=size; i++){
        *res_crc = *res_crc ^ data[i];
    }

    return SATR_OK;
}

SAT_returnState free_pkt(tc_tm_pkt *pkt) {

    for(uint8_t i = 0; i <= POOL_PKT_SIZE; i++) {
        if(&pkt_pool.pkt[i] == pkt) {
            pkt_pool.free[i] = true;
            //pkt_pool.time_delta[i]= HAL_sys_GetTick() - pkt_pool.time[i];
            return SATR_OK;
        }
    }

    return SATR_ERROR;
}

void MYDMA_Init(DMA_Stream_TypeDef* DMA_Streamx, uint32_t chx, uint32_t padd, uint32_t madd, uint16_t ndtr)
{

	DMA_InitTypeDef DMA_InitStructure;



	// Judge whether it is DMA1 perhaps DMA2
	if ((u32)DMA_Streamx > (u32)DMA2)
	{

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	}
	else
	{

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	}

	// Uninitialize
	DMA_DeInit(DMA_Streamx);

	// wait for DMA You can configure the
	while(DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
	{

	}


	// To configure DMA Stream


	// Channel selection
	DMA_InitStructure.DMA_Channel = chx;
	//DMA Peripheral address
	DMA_InitStructure.DMA_PeripheralBaseAddr = padd;
	//DMA Memory 0 Address
	DMA_InitStructure.DMA_Memory0BaseAddr = madd;
	// Memory to peripheral mode
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	// Data transmission volume
	DMA_InitStructure.DMA_BufferSize = ndtr;
	// Peripheral non incremental mode
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// Memory incremental mode
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// Peripheral data length 8 position
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	// Memory data length 8 position
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	// Use normal mode
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	// medium priority
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	// close FIFO Pattern
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	// choice FIFO threshold   Has not started FIFO  Whatever you choose
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	// Memory burst single transmission
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	// Peripheral burst single transmission
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	// initialization
	DMA_Init(DMA_Streamx, &DMA_InitStructure);
}

//DMA Can make
void MYDMA_Enable(USART_TypeDef *USARTx,DMA_Stream_TypeDef* DMA_Streamx, uint16_t ndtr)
{

	// close DMA transmission
	DMA_Cmd(DMA_Streamx, DISABLE);

	// Guarantee DMA Can be set
	while(DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
	{

	}

	// Set the amount of data transfer
	DMA_SetCurrDataCounter(DMA_Streamx, ndtr);

	USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);

	// Turn on DMA transmission
	DMA_Cmd(DMA_Streamx, ENABLE);
}

void HAL_uart_tx(TC_TM_app_id app_id, uint8_t *buf, uint16_t size) {

    if(app_id == COMMS_APP_ID) {
    	MYDMA_Init(DMA1_Stream6, DMA_Channel_4, (uint32_t)&(USART2->DR), (uint32_t)buf, size);
    	DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6);
			MYDMA_Enable(USART2,DMA1_Stream6, size);
    }
}

SAT_returnState hk_app(tc_tm_pkt *pkt)
{
	return 0;
}

SAT_returnState queuePush(tc_tm_pkt *pkt, TC_TM_app_id app_id) {

	  if(app_id == COMMS_APP_ID)
     {
		  queueCOMMS.fifo[queueCOMMS.head] = pkt;
		  queueCOMMS.head = (queueCOMMS.head + 1) % POOL_PKT_TOTAL_SIZE;
	 }


    return SATR_OK;
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

