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
#include "string.h"

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
#define EV_MAX_BUFFER  1024
#define EV_BUFFER_PART 205
#define WOD_MAX_BUFFER 224
#define TM_HK_PARAMETERS_REPORT         23
#define SYS_EXT_WOD_SIZE     196
#define COMMS_EXT_WOD_OFFSET  51
#define SUB_SYS_EXT_WOD_SIZE 146
#define false 0
static uint8_t ext_wod_buffer[SYS_EXT_WOD_SIZE];

uint8_t assertion_last_file = 0;
uint16_t assertion_last_line = 0;

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
    struct uart_data comms_uart;

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
struct _obc_data obc_data = {
   .comms_uart.init_time = 0,
   .vbat = 0,
   .adc_time = 0,
   .adc_flag = false
};

struct _sat_status {
    uint8_t batt_curr;
    uint8_t batt_volt;
    uint8_t bus_3v3_curr;
    uint8_t bus_5v_curr;
    uint8_t temp_eps;
    uint8_t temp_batt;
    uint8_t temp_comms;
};

struct _sat_status sat_status;
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
    HEALTH_REP      = 1,
	WOD_REP         = 4,
	EXT_WOD_REP     = 5
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



struct _sys_data {
    uint8_t rsrc;
    uint32_t *boot_counter;
};

static struct _sys_data sys_data;

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
void clear_wod();
SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt);
SAT_returnState wod_log_load(uint8_t *buf);
SAT_returnState wod_log();
SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);
void HAL_obc_enableBkUpAccess();
uint32_t * HAL_obc_BKPSRAM_BASE();
void bkup_sram_INIT();
void clear_ext_wod();
void set_reset_source(const uint8_t rsrc);
void get_reset_source(uint8_t *rsrc);
void update_boot_counter();
void get_boot_counter(uint32_t *cnt);
void HAL_reset_source(uint8_t *src);
void cnv32_8(const uint32_t from, uint8_t *to);
void cnv16_8(const uint16_t from, uint8_t *to);
void cnv8_D(uint8_t *from, double *to);
void cnvD_8(const double from, uint8_t *to) ;
void cnv8_F(uint8_t *from, float *to);
void cnvF_8(const float from, uint8_t *to);
void cnv8_16(uint8_t *from, uint16_t *to);
void cnv8_16LE(uint8_t *from, uint16_t *to);
void cnv8_32(uint8_t *from, uint32_t *to);
int main(void)
{
	 RCC_Config();
	 GPIO_Config();
     USART_Config();



//     sat_status.batt_curr =1;
//     sat_status.batt_volt =1;
//     sat_status.bus_3v3_curr =1;
//     sat_status.bus_5v_curr=1;
//     sat_status.temp_eps=1;
//     sat_status.temp_batt=1;
//     sat_status.temp_comms=1;

	   HAL_obc_enableBkUpAccess();
	   bkup_sram_INIT();

	   uint8_t rsrc = 0;
	   HAL_reset_source(&rsrc);
	   set_reset_source(rsrc);

	   uint32_t b_cnt = 0;
	   get_boot_counter(&b_cnt);

	   update_boot_counter();

	 hk_pkt.data = hk_pkt_data;
//	 wod_log();
//     clear_wod();
//    hk_crt_pkt_TM(&hk_pkt, COMMS_APP_ID, WOD_REP);
//     route_pkt(&hk_pkt);

	 hk_crt_pkt_TM(&hk_pkt, COMMS_APP_ID, EXT_WOD_REP);
     route_pkt(&hk_pkt);
     clear_ext_wod();

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
	   USART_InitStructure.USART_BaudRate = 115200;
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
void clear_wod() {
    sat_status.batt_curr = 0;
    sat_status.batt_volt = 0;
    sat_status.bus_3v3_curr = 0;
    sat_status.bus_5v_curr = 0;
    sat_status.temp_eps = 0;
    sat_status.temp_batt = 0;
    sat_status.temp_comms = 0;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {

    pkt->data[0] = (HK_struct_id)sid;


    if(sid == WOD_REP) {

        wod_log_load(&pkt->data[5]);

        //uint16_t size = 4+(32*7);
        pkt->len = 1+4+(32*7);

    } else if(sid == EXT_WOD_REP) {

        uint16_t size = 1;

		 //*************************************** obc_REP*****************************************//
		uint8_t      Boot_Cnt_OBC =94;
		pkt->data[size] = Boot_Cnt_OBC;
		size += 1;

		uint16_t      Boot_Cnt_COMMS =285;
		cnv16_8(Boot_Cnt_COMMS, &pkt->data[size]);
		size +=2;

		uint16_t     Boot_Cnt_EPS =49362;
		cnv16_8(Boot_Cnt_EPS, &pkt->data[size]);
		size +=2;

		uint8_t     IAC_State =0;
		pkt->data[size] = IAC_State;
		size += 1;

		uint8_t    obc_Last_Assertion_F =1;
		pkt->data[size] = obc_Last_Assertion_F;
		size += 1;

		uint8_t     obc_Last_Assertion_L =29;
		pkt->data[size] = obc_Last_Assertion_L;
		size += 1;

		uint8_t    MS_Err_line =0;
		pkt->data[size] = MS_Err_line;
		size += 1;

		uint8_t    MS_Last_Err =115;
		pkt->data[size] = MS_Last_Err;
		size += 1;

		uint32_t    obc_QB50 =524649046;
		cnv32_8(obc_QB50, &pkt->data[size]);
		size +=4;

		uint8_t    obc_RST_source =7;
		pkt->data[size] = obc_RST_source;
		size += 1;

		double    RTC_VBAT =3.00512672;
		cnvD_8(RTC_VBAT, &pkt->data[size]);
		size +=8;

		uint8_t     SD_Enabled =1;
		pkt->data[size] = SD_Enabled;
		size += 1;

		double     SU_Init_Func_Run_time =524625.105;
		cnvD_8(SU_Init_Func_Run_time, &pkt->data[size]);
		size +=8;

		uint8_t     SU_Last_Active_script =1;
		pkt->data[size] = SU_Last_Active_script;
		size += 1;

		uint8_t    SU_Script_Sch_Active =1;
		pkt->data[size] = SU_Script_Sch_Active;
		size += 1;

		uint8_t     SU_Service_Sch_Active =0;
		pkt->data[size] = SU_Service_Sch_Active;
		size += 1;

		float    Task_time_HK =27.274;
		cnvF_8(Task_time_HK, &pkt->data[size]);
		size +=4;

		float    Task_time_IDLE =35.57;
		cnvF_8(Task_time_IDLE, &pkt->data[size]);
		size +=4;

		float    Task_time_SCH =27.142;
		cnvF_8(Task_time_SCH, &pkt->data[size]);
		size +=4;

		float    Task_time_SU =0.0;
		cnvF_8(Task_time_SU, &pkt->data[size]);
		size +=4;

		float    Task_time_UART =23942.914;
		cnvF_8(Task_time_UART, &pkt->data[size]);
		size +=4;

		float    obc_Time =23948.706;
		cnvF_8(obc_Time, &pkt->data[size]);
		size +=4;

		uint8_t     tt_perm_exec_on_span_count =1;
		pkt->data[size] = tt_perm_exec_on_span_count;
		size += 1;

		uint8_t    tt_perm_norm_exec_count =20;
		pkt->data[size] = tt_perm_norm_exec_count;
		size += 1;

		//*************************************** ADCS_REP*****************************************//
		uint8_t  Boot_Cnt =74;
		pkt->data[size] = Boot_Cnt;
		size += 1;

		double  ECI_X=633.5;
		cnvD_8(ECI_X, &pkt->data[size]);
		size +=8;

		double  ECI_Y=-6093.0;
		cnvD_8(ECI_Y, &pkt->data[size]);
		size +=8;

		double  ECI_Z=2896.5;
		cnvD_8(ECI_Z, &pkt->data[size]);
		size +=8;

		uint8_t  GPS_Sats=255;
		pkt->data[size] = GPS_Sats;
		size += 1;

		uint8_t  GPS_Status=1;
		pkt->data[size] = GPS_Status;
		size += 1;

		uint32_t  GPS_Time=4294967295;
		cnv32_8(GPS_Time, &pkt->data[size]);
		size +=4;

		uint16_t  GPS_Week=65535;
		cnv16_8(GPS_Week, &pkt->data[size]);
		size +=2;

		double  Gyr_X=-0.01;
		cnvD_8(Gyr_X, &pkt->data[size]);
		size +=8;

		double  Gyr_Y=0.025;
		cnvD_8(Gyr_Y, &pkt->data[size]);
		size +=8;

		double  Gyr_Z=-0.006;
		cnvD_8(Gyr_Z, &pkt->data[size]);
		size +=8;

		uint8_t  Last_Assertion_F=37;
		pkt->data[size] = Last_Assertion_F;
		size += 1;

		uint8_t  Last_Assertion_L=50;
		pkt->data[size] = Last_Assertion_L;
		size += 1;

		uint8_t  MG_Torq_I_Y=0;
		pkt->data[size] = MG_Torq_I_Y;
		size += 1;

		uint8_t  MG_Torq_I_Z=0;
		pkt->data[size] = MG_Torq_I_Z;
		size += 1;

		float  Pitch =58.43;
		cnvF_8(Pitch, &pkt->data[size]);
		size +=4;

		double  Pitch_Dot=0.023;
		cnvD_8(Pitch_Dot, &pkt->data[size]);
		size +=8;

		uint32_t  QB50=524649019;
		cnv32_8(QB50, &pkt->data[size]);
		size +=4;

		uint16_t RM_X=21750;
		cnv16_8(RM_X, &pkt->data[size]);
		size +=2;

		uint8_t      RM_Y =40;
		pkt->data[size] = RM_Y;
		size += 1;

		double        RM_Z=-12890;
		cnvD_8(RM_Z, &pkt->data[size]);
		size +=8;

		uint8_t     RST_source=18;
		pkt->data[size] = RST_source;
		size += 1;

		double       Roll=157.57;
		cnvD_8(Roll, &pkt->data[size]);
		size +=8;

		double      Roll_Dot=-0.209;
		cnvD_8(Roll_Dot, &pkt->data[size]);
		size +=8;

		uint8_t     Spin_RPM=0;
		pkt->data[size] = Spin_RPM;
		size += 1;

		float      Sun_V_0 = 3.49;
		cnvF_8(Sun_V_0, &pkt->data[size]);
		size +=4;

		float      Sun_V_1=3.47;
		cnvF_8(Sun_V_1, &pkt->data[size]);
		size +=4;

		float      Sun_V_2=3.49;
		cnvF_8(Sun_V_2, &pkt->data[size]);
		size +=4;

		float      Sun_V_3=3.49;
		cnvF_8(Sun_V_3, &pkt->data[size]);
		size +=4;

		float     Sun_V_4=3.49;
		cnvF_8(Sun_V_4, &pkt->data[size]);
		size +=4;

		uint8_t     TX_error=5;
		pkt->data[size] = TX_error;
		size += 1;

		float    Temp=34.74;
		cnvF_8(Temp, &pkt->data[size]);
		size +=4;

		double  Time =2113.356;
		cnvD_8(Time, &pkt->data[size]);
		size +=8;

		uint16_t       XM_X=24910;
		cnv16_8(XM_X, &pkt->data[size]);
		size +=2;

		uint16_t       XM_Y=13060;
		cnv16_8(XM_Y, &pkt->data[size]);
		size +=2;

		double    XM_Z=-6230;
		cnvD_8(XM_Z, &pkt->data[size]);
		size +=8;

		float    Yaw=60.19;
		cnvF_8(Yaw, &pkt->data[size]);
		size +=4;

		float    Yaw_Dot=0.197;
		cnvF_8(Yaw_Dot, &pkt->data[size]);
		size +=4;

		//*************************************** comms_REP*****************************************//
		uint8_t     Beacon_pattern =72;
		pkt->data[size] = Beacon_pattern;
		size += 1;

		uint32_t    Flash_read_transmit =371609220;
		cnv32_8(Flash_read_transmit, &pkt->data[size]);
		size +=4;

		uint8_t     Invalid_Dest_Frames_Cnt =0;
		pkt->data[size] = Invalid_Dest_Frames_Cnt;
		size += 1;

		uint8_t     comms_Last_Assertion_F =1;
		pkt->data[size] = comms_Last_Assertion_F;
		size += 1;

		uint8_t     comms_Last_Assertion_L =29;
		pkt->data[size] = comms_Last_Assertion_L;
		size += 1;

		uint16_t     Last_RX_Error =65530;
		cnv16_8(Last_RX_Error, &pkt->data[size]);
		size +=2;

		uint16_t    Last_TX_Error =65532;
		cnv16_8(Last_TX_Error, &pkt->data[size]);
		size +=2;

		uint8_t     comms_RST_source =7;
		pkt->data[size] = comms_RST_source;
		size += 1;

		uint8_t    RX_CRC_Failed =0;
		pkt->data[size] = RX_CRC_Failed;
		size += 1;

		uint8_t    RX_Failed =0;
		pkt->data[size] = RX_Failed;
		size += 1;

		uint8_t    RX_Frames =2;
		pkt->data[size] = RX_Frames;
		size += 1;

		uint8_t   TX_Failed =0;
		pkt->data[size] = TX_Failed;
		size += 1;

		uint16_t    TX_Frames =695;
		cnv16_8(TX_Frames, &pkt->data[size]);
		size +=2;

		double    comms_Time =23926.658;
		cnvD_8(comms_Time, &pkt->data[size]);
		size +=8;

		//*************************************** eps_REP*****************************************//
		uint8_t    Batt_Temp_Health_Status=0;
		pkt->data[size] = Batt_Temp_Health_Status;
		size += 1;

		uint8_t     Deployment_Status=0;
		pkt->data[size] = Deployment_Status;
		size += 1;

		uint8_t    Heater_status=0;
		pkt->data[size] = Heater_status;
		size += 1;

		uint8_t    eps_Last_Assertion_F=0;
		pkt->data[size] = eps_Last_Assertion_F;
		size += 1;

		uint8_t    eps_Last_Assertion_L =0;
		pkt->data[size] = eps_Last_Assertion_L;
		size += 1;

		uint8_t    eps_RST_source =184;
		pkt->data[size] = eps_RST_source;
		size += 1;

		uint8_t    Safety_Battery_Mode =0;
		pkt->data[size] = Safety_Battery_Mode;
		size += 1;

		uint8_t    Safety_Temp_Mode =0;
		pkt->data[size] = Safety_Temp_Mode;
		size += 1;

		uint8_t   Soft_error_status =0;
		pkt->data[size] = Soft_error_status;
		size += 1;

		uint8_t   Switch_ADCS =0;
		pkt->data[size] = Switch_ADCS;
		size += 1;

		uint8_t   Switch_COMMS =0;
		pkt->data[size] = Switch_COMMS;
		size += 1;

		uint8_t    Switch_OBC =0;
		pkt->data[size] = Switch_OBC;
		size += 1;

		uint8_t    Switch_SU =1;
		pkt->data[size] = Switch_SU;
		size += 1;

		uint8_t    Temp_sensor_PWR_SW =0;
		pkt->data[size] = Temp_sensor_PWR_SW;
		size += 1;

		double eps_Time =23226.8;
		cnvD_8(eps_Time, &pkt->data[size]);
		size +=8;

		uint8_t    Xplus_Current =56;
		pkt->data[size] = Xplus_Current;
		size += 1;

		uint8_t    Xplus_Duty =16;
		pkt->data[size] = Xplus_Duty;
		size += 1;

		uint8_t    Xplus_Voltage =99;
		pkt->data[size] = Xplus_Voltage;
		size += 1;

		uint8_t    Xminus_Current =49;
		pkt->data[size] = Xminus_Current;
		size += 1;

		uint8_t    Xminus_Duty =16;
		pkt->data[size] = Xminus_Duty;
		size += 1;

		uint8_t    Xminus_Voltage =57;
		pkt->data[size] = Xminus_Voltage;
		size += 1;

		uint8_t   Yplus_Current =168;
		pkt->data[size] = Yplus_Current;
		size += 1;

		uint8_t    Yplus_Duty =16;
		pkt->data[size] = Yplus_Duty;
		size += 1;

		uint8_t    Yplus_Voltage =210;
		pkt->data[size] = Yplus_Voltage;
		size += 1;

		uint8_t    Y_minus_Current =48;
		pkt->data[size] = Y_minus_Current;
		size += 1;

		uint8_t    Yminus_Duty =16;
		pkt->data[size] = Yminus_Duty;
		size += 1;

		uint8_t    Yminus_Voltage =51;
		pkt->data[size] = Yminus_Voltage;
		size += 1;

        pkt->len = size;

    }

    return SATR_OK;
}

SAT_returnState wod_log_load(uint8_t *buf) {

   uint8_t rev_wod_cnt = *obc_data.wod_cnt;
   if(rev_wod_cnt == 0) { rev_wod_cnt = WOD_MAX_BUFFER; }

   uint16_t buf_cnt = 0;
   for(uint16_t i = 0; i < WOD_MAX_BUFFER; i++) {

        rev_wod_cnt--;
        buf[buf_cnt++] = obc_data.wod_log[rev_wod_cnt];
        if(rev_wod_cnt == 0) { rev_wod_cnt = WOD_MAX_BUFFER; }
   }
   return SATR_OK;
}

SAT_returnState wod_log() {

//check endianess
    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.temp_batt;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.temp_eps;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.temp_comms;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.bus_5v_curr;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.bus_3v3_curr;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.batt_curr;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    obc_data.wod_log[*obc_data.wod_cnt] = sat_status.batt_volt;
    (*obc_data.wod_cnt)++;
    if(*obc_data.wod_cnt >= WOD_MAX_BUFFER) { *obc_data.wod_cnt = 0; }

    return SATR_OK;
}

SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid) {

    hk_report_parameters(sid, pkt);

    crt_pkt(pkt, (TC_TM_app_id)SYSTEM_APP_ID, TM, TC_ACK_NO, TC_HOUSEKEEPING_SERVICE, TM_HK_PARAMETERS_REPORT, app_id);

    return SATR_OK;
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
    for(uint32_t i=0 ; i< EV_MAX_BUFFER ;i++)
    {
    	obc_data.wod_log[i]=5;
    }

    base_pointer += sizeof(uint8_t);

    *obc_data.log_cnt = 0;
    *obc_data.wod_cnt = 0;
    *obc_data.fs_wod_head = 1;
    *obc_data.fs_ext_head = 1;
    *obc_data.fs_ev_head = 1;
    *obc_data.fs_wod_tail = 1;
    *obc_data.fs_ext_tail = 1;
    *obc_data.fs_ev_tail = 1;
    *obc_data.fs_fotos = 1;
}

void clear_ext_wod() {
    memset(ext_wod_buffer, 0, SYS_EXT_WOD_SIZE);
}

void set_reset_source(const uint8_t rsrc) {
    sys_data.rsrc = rsrc;
}

void get_reset_source(uint8_t *rsrc) {
    *rsrc = sys_data.rsrc;
}

void update_boot_counter() {
    (*sys_data.boot_counter)++;
}

void get_boot_counter(uint32_t *cnt) {
    *cnt = *sys_data.boot_counter;
}

void HAL_reset_source(uint8_t *src) {

    *src = RCC_GetFlagStatus(RCC_FLAG_BORRST);
    *src |= (RCC_GetFlagStatus(RCC_FLAG_PINRST) << 1);
    *src |= (RCC_GetFlagStatus(RCC_FLAG_PORRST) << 2);
    *src |= (RCC_GetFlagStatus(RCC_FLAG_SFTRST) << 3);
    *src |= (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) << 4);
    *src |= (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) << 5);
    *src |= (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) << 6);

    RCC_ClearFlag();

}

void cnv32_8(const uint32_t from, uint8_t *to) {

    union _cnv cnv;

    cnv.cnv32 = from;
    to[0] = cnv.cnv8[0];
    to[1] = cnv.cnv8[1];
    to[2] = cnv.cnv8[2];
    to[3] = cnv.cnv8[3];
}

void cnv16_8(const uint16_t from, uint8_t *to) {

    union _cnv cnv;

    cnv.cnv16[0] = from;
    to[0] = cnv.cnv8[0];
    to[1] = cnv.cnv8[1];

}

void cnv8_32(uint8_t *from, uint32_t *to) {

    union _cnv cnv;

    cnv.cnv8[3] = from[3];
    cnv.cnv8[2] = from[2];
    cnv.cnv8[1] = from[1];
    cnv.cnv8[0] = from[0];
    *to = cnv.cnv32;

}

void cnv8_16LE(uint8_t *from, uint16_t *to) {

    union _cnv cnv;

    cnv.cnv8[1] = from[1];
    cnv.cnv8[0] = from[0];
    *to = cnv.cnv16[0];
}

void cnv8_16(uint8_t *from, uint16_t *to) {

    union _cnv cnv;

    cnv.cnv8[1] = from[0];
    cnv.cnv8[0] = from[1];
    *to = cnv.cnv16[0];
}


void cnvF_8(const float from, uint8_t *to) {

    union _cnv cnv;

    cnv.cnvF = from;
    to[0] = cnv.cnv8[0];
    to[1] = cnv.cnv8[1];
    to[2] = cnv.cnv8[2];
    to[3] = cnv.cnv8[3];
}

void cnv8_F(uint8_t *from, float *to) {

    union _cnv cnv;

    cnv.cnv8[3] = from[3];
    cnv.cnv8[2] = from[2];
    cnv.cnv8[1] = from[1];
    cnv.cnv8[0] = from[0];
    *to = cnv.cnvF;

}

void cnvD_8(const double from, uint8_t *to) {

    union _cnv cnv;

    cnv.cnvD = from;
    to[0] = cnv.cnv8[0];
    to[1] = cnv.cnv8[1];
    to[2] = cnv.cnv8[2];
    to[3] = cnv.cnv8[3];
    to[4] = cnv.cnv8[4];
    to[5] = cnv.cnv8[5];
    to[6] = cnv.cnv8[6];
    to[7] = cnv.cnv8[7];
}

void cnv8_D(uint8_t *from, double *to) {

    union _cnv cnv;

    cnv.cnv8[7] = from[7];
    cnv.cnv8[6] = from[6];
    cnv.cnv8[5] = from[5];
    cnv.cnv8[4] = from[4];
    cnv.cnv8[3] = from[3];
    cnv.cnv8[2] = from[2];
    cnv.cnv8[1] = from[1];
    cnv.cnv8[0] = from[0];
    *to = cnv.cnvD;

}
