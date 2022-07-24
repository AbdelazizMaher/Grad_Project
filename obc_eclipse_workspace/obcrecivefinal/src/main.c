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
static void USART_Config(void);
static void DMA_Config(void);

#define _COMMS_APP_ID_ 4
#define MAX_APP_ID      20
#define MAX_SERVICES    20
#define MAX_SUBTYPES    26
const uint8_t services_verification_TC_TM[MAX_SERVICES][MAX_SUBTYPES][2] = {
/*       0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23      24      25   */
/*              TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC  TM  TC */
/*00*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*01*/{ {0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*TC_VERIFICATION_SERVICE*/
/*02*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*03*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 0}, {1, 0}, {0, 0}, {0, 0} }, /*TC_HOUSEKEEPING_SERVICE*/
/*04*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*05*/{ {0, 0}, {1, 0}, {0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*TC_EVENT_SERVICE*/
/*06*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*07*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*08*/{ {0, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*TC_FUNCTION_MANAGEMENT_SERVICE*/
/*09*/{ {0, 0}, {1, 1}, {1, 1}, {0, 1}, {0, 1}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }  /*TIME MANAGEMENT SERVICE*/,
/*10*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*11*/{ {0, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 0}, {0, 1}, {0, 1}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 1} }, /*TC_SCHEDULING_SERVICE*/
/*12*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*13*/{ {0, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {0, 1}, {0, 1}, {1, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*TC_LARGE_DATA_SERVICE*/
/*14*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*15*/{ {0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 0}, {0, 1}, {0, 0}, {0, 1}, {0, 1}, {1, 0}, {0, 1}, {0, 1}, {0, 1}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*TC_MASS_STORAGE_SERVICE*/
/*16*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/*17*/{ {0, 0}, {0, 1}, {1, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*TC_TEST_SERVICE*/
/*18*/{ {0, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {1, 0}, {0, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }, /*MNLP TEST SERVICE*/
/*19*/{ {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
};
#define UART_BUF_SIZE 4096 /*(POOL_PKT_EXT*2)*/
#define UART_DMA_BUF_SIZE  100
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
#define SYSTEM_APP_ID _OBC_APP_ID_
#define HLDLC_START_FLAG        0x7E
#define HLDLC_CONTROL_FLAG      0x7D
#define POOL_PKT_SIZE        16
#define MAX_PKT_DATA        198
#define true	1
#define false	0
#define NULL ((void *)0)
#define PKT_NORMAL  198   /*MAX_PKT_DATA*/
#define TC_ACK_ACC          0x01

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

struct _pkt_pool{
    tc_tm_pkt pkt[POOL_PKT_SIZE];
    uint8_t free[POOL_PKT_SIZE];
    uint32_t time[POOL_PKT_SIZE];
    uint8_t data[POOL_PKT_SIZE][MAX_PKT_DATA];
    uint32_t time_delta[POOL_PKT_SIZE];
};
struct _pkt_state {
    uint8_t seq_cnt[8];
};

union _cnv {
    double cnvD;
    float cnvF;
    uint32_t cnv32;
    uint16_t cnv16[4];
    uint8_t cnv8[8];
};

static struct _pkt_pool pkt_pool;

uint8_t SUBSYS1[UART_DMA_BUF_SIZE];
uint16_t err;
static uint32_t old_pos=0;
static	uint32_t pos=0;
#define ARRAY_LEN(x)            (sizeof(x) / sizeof((x)[0]))
#define True 1
#define False 0
#define TC_MIN_PKT_SIZE     11
uint8_t flag = False;
uint32_t size = 0;
uint8_t packet_flag = 0;
SAT_returnState import_pkt(TC_TM_app_id app_id, struct uart_data *data);
void UART_DMA_rx_check(TC_TM_app_id app_id, struct uart_data *data);
void UART_PROCESS_DATA(const void* temp, uint32_t len,struct uart_data *data);
SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data);
tc_tm_pkt * get_pkt(const uint16_t size);
SAT_returnState unpack_pkt(const uint8_t *buf, tc_tm_pkt *pkt, const uint16_t size);
void stats_inbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt);
SAT_returnState HLDLC_deframe(uint8_t *buf_in, uint8_t *buf_out, uint16_t *size);
SAT_returnState route_pkt(tc_tm_pkt *pkt);
SAT_returnState checkSum(const uint8_t *data, const uint16_t size, uint8_t *res_crc);
void cnv8_16(uint8_t *from, uint16_t *to);

int main(void)
{

	 RCC_Config();
	 GPIO_Config();
	 USART_Config();
	 DMA_Config();

	 for(;;)
	 {
		 import_pkt(COMMS_APP_ID, &obc_data.comms_uart);
	 }




	for(;;);
}


static void RCC_Config(void)
 {
   /* Enable GPIO clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
   /* Enable USARTs Clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
   /* Enable the DMA periph */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

 }

static void GPIO_Config(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
   GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(GPIOD, &GPIO_InitStructure);

  }

static void USART_Config(void)
{
	USART_InitTypeDef USART_InitStructure;

	   USART_InitStructure.USART_BaudRate = 9600;
	   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	   USART_InitStructure.USART_StopBits = USART_StopBits_1;
	   USART_InitStructure.USART_Parity = USART_Parity_No;
	   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	   USART_InitStructure.USART_Mode = USART_Mode_Rx;
	   USART_Init(USART3, &USART_InitStructure);
	   USART_Cmd(USART3, ENABLE);

}

static void DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	   DMA_DeInit(DMA1_Stream1);
	   DMA_InitStructure.DMA_BufferSize = UART_DMA_BUF_SIZE;
	   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	   DMA_InitStructure.DMA_Channel = DMA_Channel_4 ;
	   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)SUBSYS1;
	   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
	   DMA_Init(DMA1_Stream1, &DMA_InitStructure);
	  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	  DMA_Cmd(DMA1_Stream1,ENABLE);
}

SAT_returnState import_pkt(TC_TM_app_id app_id, struct uart_data *data) {

    tc_tm_pkt *pkt;
    uint16_t size = 0;

    SAT_returnState res;
    SAT_returnState res_deframe;

	  UART_DMA_rx_check(app_id,data);

   res = HAL_uart_rx(app_id, data);
    if( res == SATR_EOT ) {

        size = data->uart_size;
        res_deframe = HLDLC_deframe(data->uart_unpkt_buf, data->deframed_buf, &size);
        if(res_deframe == SATR_EOT) {

            pkt = get_pkt(size);

            if((res = unpack_pkt(data->deframed_buf, pkt, size)) == SATR_OK) {
                stats_inbound(pkt->type, pkt->app_id, pkt->dest_id, pkt);
                route_pkt(pkt); }
         }
    }

    return SATR_OK;
}

void UART_DMA_rx_check(TC_TM_app_id app_id, struct uart_data *data)
{
	uint8_t *pData;
	DMA_Stream_TypeDef* DMAy_Streamx;

    if(app_id == COMMS_APP_ID)
      {
		pData = SUBSYS1;
		DMAy_Streamx = DMA1_Stream1;
	  }
	   	pos = UART_DMA_BUF_SIZE - DMA_GetCurrDataCounter(DMAy_Streamx);
	   	   if (pos != old_pos) {                       /* Check change in received data */
					  if (pos > old_pos) {                    /* Current position is over previous one */

	        	   UART_PROCESS_DATA(&pData[old_pos], pos - old_pos,data);
	                    }
		            else {

                       UART_PROCESS_DATA(&pData[old_pos], ARRAY_LEN(pData) - old_pos,data);
                       if (pos > 0 && (flag == False) ) {
                        UART_PROCESS_DATA(&pData[0], pos,data);
                        }
					}
				}
}

void UART_PROCESS_DATA(const void* temp, uint32_t len,struct uart_data *data)
{
	 const uint8_t* d = temp;
	 uint8_t c = len;
	 uint8_t err =0;
   for ( uint32_t i =0; len > 0; len-- ,++d ,++i )
      {

        if( (c == len) && (*d == HLDLC_START_FLAG) )
					{
						data->uart_buf[i] = *d;
            //uart_timeout_start(huart);
          }
				 else if(*d == HLDLC_START_FLAG)
					 {
             data->uart_buf[i] = *d;
						 size = i+1;
						 if(pos > old_pos)
						 {
						   old_pos = old_pos + size +err;
						   packet_flag =1;
						    UART2_BUF_O_Write_String_To_Buffer("recieved data complete\n");
                            UART2_BUF_O_Send_All_Data();
						 }
						 else if((pos < old_pos) && (old_pos +i < UART_DMA_BUF_SIZE))
						 {
						   old_pos = old_pos +size +err;
               flag = True;
						 }
						 else if(pos > 0)
						 {
               	flag = False;
                old_pos = size +err;
						 }
						 break;
           }
		     else if( c > len )
		       {
						 data->uart_buf[i] = *d;
           }
		     else
		     {
		    	 i--;
		    	 len++;
		    	 err++;
		     }
      }
}

SAT_returnState HAL_uart_rx(TC_TM_app_id app_id, struct uart_data *data) {

	if(packet_flag && size > TC_MIN_PKT_SIZE)
	{
      data->uart_size = size;
      for(uint16_t i = 0; i < data->uart_size; i++) { data->uart_unpkt_buf[i] = data->uart_buf[i]; }
      return SATR_EOT;
	}

    return SATR_OK;
}

tc_tm_pkt * get_pkt(const uint16_t size) {

    if(size <= PKT_NORMAL) {
        for(uint8_t i = 0; i < POOL_PKT_SIZE; i++) {
            if(pkt_pool.free[i] == true) {
                pkt_pool.free[i] = false;
                return &pkt_pool.pkt[i];
            }
        }
    }
    return NULL;
}

SAT_returnState unpack_pkt(const uint8_t *buf, tc_tm_pkt *pkt, const uint16_t size) {

    uint8_t tmp_crc[2];

    uint8_t ver, dfield_hdr, ccsds_sec_hdr, tc_pus;

    tmp_crc[0] = buf[size - 1];
    checkSum(buf, size-2, &tmp_crc[1]); /* -2 for excluding the checksum bytes*/

    ver = buf[0] >> 5;

    pkt->type = (buf[0] >> 4) & 0x01;
    dfield_hdr = (buf[0] >> 3) & 0x01;

    pkt->app_id = (TC_TM_app_id)buf[1];

    pkt->seq_flags = buf[2] >> 6;

    cnv8_16((uint8_t*)&buf[2], &pkt->seq_count);
    pkt->seq_count &= 0x3FFF;

    cnv8_16((uint8_t*)&buf[4], &pkt->len);

    ccsds_sec_hdr = buf[6] >> 7;

    tc_pus = buf[6] >> 4;

    pkt->ack = 0x07 & buf[6];

    pkt->ser_type = buf[7];
    pkt->ser_subtype = buf[8];
    pkt->dest_id = (TC_TM_app_id)buf[9];

    pkt->verification_state = SATR_PKT_INIT;


    for(int i = 0; i < pkt->len; i++) {
        pkt->data[i] = buf[ECSS_DATA_OFFSET+i];
    }

    return SATR_OK;
}

void stats_inbound(uint8_t type, TC_TM_app_id app_id, TC_TM_app_id dest_id, tc_tm_pkt *pkt) {

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

SAT_returnState route_pkt(tc_tm_pkt *pkt) {

    SAT_returnState res;
    TC_TM_app_id id;


    if(pkt->type == TC)         { id = pkt->app_id; }
    else if(pkt->type == TM)    { id = pkt->dest_id; }


     if(id == SYSTEM_APP_ID && pkt->ser_type == TC_HOUSEKEEPING_SERVICE) {
        //C_ASSERT(pkt->ser_subtype == 21 || pkt->ser_subtype == 23) { free_pkt(pkt); return SATR_ERROR; }
     }


    return SATR_OK;
}

SAT_returnState HLDLC_deframe(uint8_t *buf_in, uint8_t *buf_out, uint16_t *size) {

    uint16_t cnt = 0;

    for(uint16_t i = 1; i < *size; i++) {
        if(buf_in[i] == HLDLC_START_FLAG) {
            *size = cnt;
            return SATR_EOT;
        } else if(buf_in[i] == HLDLC_CONTROL_FLAG) {
            i++;
            if(buf_in[i] == 0x5E) { buf_out[cnt++] = 0x7E; }
            else if(buf_in[i] == 0x5D) { buf_out[cnt++] = 0x7D; }
            else { return SATR_ERROR; }
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

void cnv8_16(uint8_t *from, uint16_t *to) {

    union _cnv cnv;

    cnv.cnv8[1] = from[0];
    cnv.cnv8[0] = from[1];
    *to = cnv.cnv16[0];
}
