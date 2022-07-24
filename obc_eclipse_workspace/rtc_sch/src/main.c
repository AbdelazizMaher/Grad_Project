

#include "stm32f4xx.h"
#include "string.h"
#include "stdio.h"

#define true	1
#define false	0
#define _OBC_APP_ID_   1
#define _EPS_APP_ID_   2
#define _ADCS_APP_ID_  3
#define _COMMS_APP_ID_ 4
#define _IAC_APP_ID_   5
#define _GND_APP_ID_   6
#define _DBG_APP_ID_   7
#define _LAST_APP_ID_  8
#define SC_MAX_STORED_SCHEDULES 2
#define MAX_PKT_LEN         210
#define MIN_VALID_QB50_SECS  300 //5min
#define TC 1
#define TM 0
#define TC_SCHEDULING_SERVICE           11

/*services types*/
#define TC_VERIFICATION_SERVICE         1
#define TC_HOUSEKEEPING_SERVICE         3
#define TC_EVENT_SERVICE                5
#define TC_FUNCTION_MANAGEMENT_SERVICE  8
#define TC_TIME_MANAGEMENT_SERVICE      9
#define TC_SCHEDULING_SERVICE           11
#define TC_LARGE_DATA_SERVICE           13
#define TC_MASS_STORAGE_SERVICE         15
#define TC_TEST_SERVICE                 17
#define TC_SU_MNLP_SERVICE              18 /*service number out of ECSS standard, mission specific for mnlp su*/

/*services subtypes*/
#define TM_VR_ACCEPTANCE_SUCCESS        1
#define TM_VR_ACCEPTANCE_FAILURE        2

#define TC_HK_REPORT_PARAMETERS         21
#define TM_HK_PARAMETERS_REPORT         23

#define TM_EV_NORMAL_REPORT         	1
#define TM_EV_ERROR_REPORT         	4

#define TC_FM_PERFORM_FUNCTION          1

#define TC_SC_ENABLE_RELEASE            1
#define TC_SC_DISABLE_RELEASE           2
#define TC_SC_RESET_SCHEDULE            3
#define TC_SC_INSERT_TC                 4
#define TC_SC_DELETE_TC                 5
#define TC_SC_TIME_SHIFT_SPECIFIC       7
#define TC_SC_TIME_SHIFT_SELECTED_OTP   8
#define TC_SC_TIME_SHIFT_ALL            15

#define TM_LD_FIRST_DOWNLINK            1
#define TC_LD_FIRST_UPLINK              9
#define TM_LD_INT_DOWNLINK              2
#define TC_LD_INT_UPLINK                10
#define TM_LD_LAST_DOWNLINK             3
#define TC_LD_LAST_UPLINK               11
#define TC_LD_ACK_DOWNLINK              5
#define TM_LD_ACK_UPLINK                14
#define TC_LD_REPEAT_DOWNLINK           6
#define TM_LD_REPEAT_UPLINK             15
#define TM_LD_REPEATED_DOWNLINK         7
#define TC_LD_REPEATED_UPLINK           12
#define TM_LD_ABORT_SE_DOWNLINK         4
#define TC_LD_ABORT_SE_UPLINK           13
#define TC_LD_ABORT_RE_DOWNLINK         8
#define TM_LD_ABORT_RE_UPLINK           16

#define TC_MS_ENABLE                    1
#define TC_MS_DISABLE                   2
#define TM_MS_CONTENT                   8
#define TC_MS_DOWNLINK                  9
#define TC_MS_DELETE                    11
#define TC_MS_REPORT                    12
#define TM_MS_CATALOGUE_REPORT          13
#define TC_MS_UPLINK                    14
#define TC_MS_FORMAT                    15 /*custom service*/
#define TC_MS_LIST                      16 /*custom service*/
#define TM_MS_CATALOGUE_LIST            17

#define TC_CT_PERFORM_TEST              1
#define TM_CT_REPORT_TEST               2


/*TIME MANAGEMENT SERVICE*/
#define TM_TIME_SET_IN_UTC              1 /*subservice 1*/
#define TM_TIME_SET_IN_QB50             2 /*subservice 2*/
#define TM_REPORT_TIME_IN_UTC           3 /*subservice 3, Telecommand to report time in UTC*/
#define TM_REPORT_TIME_IN_QB50          4 /*subservice 4, Telecommand to report time in QB50*/
#define TM_TIME_REPORT_IN_UTC           5 /*subservice 5, Telemetry response time in UTC*/
#define TM_TIME_REPORT_IN_QB50          6 /*subservice 6, Telemetry response time in QB50*/

/*SCHEDULING SERVICE*/
#define SCHS_ENABLE_RELEASE             1 /*subservice 01, Telecommand to enable the release of telecommands from schedule pool*/
#define SCHS_DISABLE_RELEASE            2 /*subservice 02, Telecommand to disable the release of telecommands from schedule pool*/
#define SCHS_RESET_SCH                  3 /*subservice 03, Telecommand to reset the schedule pool*/
#define SCHS_INSERT_TC_IN_SCH           4 /*subservice 04, Telecommand to insert a tc_tm_pkt in schedule pool*/
#define SCHS_DELETE_TC_FROM_SCH         5 /*subservice 05, Telecommand to delete a tc_tm_pkt from schedule pool*/
#define SCHS_DELETE_TC_FROM_SCH_OTP     6 /*subservice 06, Telecommand to delete tc_tm_pkts from schedule pool over a time period*/
#define SCHS_TIME_SHIFT_SEL_TC          7 /*subservice 07, Telecommand to time shift (+/-) selected active schedule packet*/
#define SCHS_TIME_SHIFT_SEL_TC_OTP      8 /*subservice 08, Telecommand to time shift (+/-) selected active schedule packets over a time period*/
#define SCHS_DETAILED_SCH_REPORT        10 /*subservice 10, Telemerty response to (to TC no:16) report schedules in detailed form*/
#define SCHS_SIMPLE_SCH_REPORT          13 /*subservice 13, Telemerty response to (to TC no:17) report schedules in summary form*/
#define SCHS_TIME_SHIFT_ALL_TCS         15 /*subservice 15, Telecommand to time shift (+/-) all active schedule packets*/
#define SCHS_REPORT_SCH_DETAILED        16 /*subservice 16, Telecommand to report schedules in detailed form*/
#define SCHS_REPORT_SCH_SUMMARY         17 /*subservice 17, Telecommand to report schedules in summary form*/
#define SCHS_LOAD_SCHEDULES             22 /*subservice 22, Telecommand to load schedules from perm storage*/
#define SCHS_SAVE_SCHEDULES             23 /*subservice 23, Telecommand to save schedules on perm storage*/

#define C_ASSERT(e)    ((e) ? (true) : (tst_debugging((uint8_t *)__FILE__, __LINE__, #e)))


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
    SATR_CRC_ERROR             = 9,
    SATR_PKT_ILLEGAL_ACK       = 10,
    SATR_ALREADY_SERVICING     = 11,
    SATR_MS_MAX_FILES          = 12,
    SATR_PKT_INIT              = 13,
    SATR_INV_STORE_ID          = 14,
    SATR_INV_DATA_LEN          = 15,
    /* Scheduling Service Error State Codes
    * from
    */
    SATR_SCHS_FULL             = 16, /*Schedule array is full */
    SATR_SCHS_ID_INVALID       = 17, /*Sub-schedule ID invalid */
    SATR_SCHS_NMR_OF_TC_INVLD  = 18, /*Number of telecommands invalid */
    SATR_SCHS_INTRL_ID_INVLD   = 19, /*Interlock ID invalid */
    SATR_SCHS_ASS_INTRL_ID_INVLD = 20, /*Assess Interlock ID invalid */
    SATR_SCHS_ASS_TP_ID_INVLD  = 21, /*Assessment type id invalid*/
    SATR_SCHS_RLS_TT_ID_INVLD  = 22, /*Release time type ID invalid */
    SATR_SCHS_DST_APID_INVLD   = 23, /*Destination APID in embedded TC is invalid */
    SATR_SCHS_TIM_INVLD        = 24, /*Release time of TC is invalid */
    SATR_QBTIME_INVALID        = 25, /*Time management reported erroneous time*/
    SATR_SCHS_TIM_SPEC_INVLD   = 26, /*Release time of TC is specified in a invalid representation*/
    SATR_SCHS_INTRL_LGC_ERR    = 27, /*The release time of telecommand is in the execution window of its interlocking telecommand.*/
    SATR_SCHS_DISABLED         = 28,
    SATR_SCHS_NOT_IMLP         = 29, /*Not implemented function of scheduling service*/
    /*LAST*/
    SATR_LAST                  = 56
}SAT_returnState;

typedef enum {
    OBC_APP_ID      = _OBC_APP_ID_,
    EPS_APP_ID      = _EPS_APP_ID_,
    ADCS_APP_ID     = _ADCS_APP_ID_,
    COMMS_APP_ID    = _COMMS_APP_ID_,
    IAC_APP_ID      = _IAC_APP_ID_,
    GND_APP_ID      = _GND_APP_ID_,
    DBG_APP_ID      = _DBG_APP_ID_,
    LAST_APP_ID     = _LAST_APP_ID_
}TC_TM_app_id;

typedef enum {
    REPETITIVE      = 4
}SC_event_time_type;

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

typedef struct {

    TC_TM_app_id app_id;

    uint8_t seq_count;

    uint8_t enabled;

    uint8_t sub_schedule_id;

    uint8_t num_of_sch_tc;

    uint8_t intrlck_set_id ;

    uint8_t intrlck_ass_id ;

    uint8_t assmnt_type;

    SC_event_time_type sch_evt;

    uint32_t release_time;

    uint32_t timeout;

    tc_tm_pkt tc_pck;

    uint8_t pos_taken;

}SC_pkt;

typedef struct{
    /*Holds structures, containing Scheduling Telecommands*/
    SC_pkt sc_mem_array[SC_MAX_STORED_SCHEDULES];

    uint8_t innerd_tc_data[SC_MAX_STORED_SCHEDULES][MAX_PKT_LEN];

}Schedule_pkt_pool;

extern Schedule_pkt_pool sch_mem_pool;

typedef struct {
    /*Number of loaded schedules*/
    uint8_t nmbr_of_ld_sched;

    uint8_t schs_service_enabled;

    uint8_t sch_arr_full;

    uint8_t schs_apids_enabled[7];

}Scheduling_service_state;

typedef enum {
    HEALTH_REP      = 1,
    EX_HEALTH_REP   = 2,
    EVENTS_REP      = 3,
    WOD_REP         = 4,
    EXT_WOD_REP     = 5,
    SU_SCI_HDR_REP  = 6,
    ADCS_TLE_REP    = 7,
    EPS_FLS_REP     = 8,
    ECSS_STATS_REP  = 9,
    LAST_STRUCT_ID  = 10
}HK_struct_id;

typedef enum {
    P_OFF       = 0,
    P_ON        = 1,
    P_RESET     = 2,
    SET_VAL     = 3,
    LAST_FUN_ID = 4
}FM_fun_id;

typedef enum {
    OBC_DEV_ID      = 1,
    EPS_DEV_ID      = 2,
    ADCS_DEV_ID     = 3,
    COMMS_DEV_ID    = 4,
    IAC_DEV_ID      = 5,
    SU_DEV_ID       = 6,
    GPS_DEV_ID      = 7,
    OBC_SD_DEV_ID   = 8,
    ADCS_SD_DEV_ID  = 9,
    ADCS_SENSORS    = 10,
    ADCS_GPS        = 11,
    ADCS_MAGNETO    = 12,
    ADCS_SPIN       = 13,
    ADCS_TLE        = 14,
    ADCS_CTRL_GAIN  = 15,
    ADCS_SET_POINT  = 16,
    EPS_WRITE_FLS   = 17,
    SYS_DBG         = 18,
    COMMS_WOD_PAT   = 19,
    LAST_DEV_ID     = 20
}FM_dev_id;

typedef enum {
    EV_inc_pkt           = 1,
    EV_pkt_ack_er        = 2,
    EV_sys_boot          = 3,
    EV_pwr_level         = 4,
    EV_comms_tx_off      = 5,
    EV_sys_timeout       = 6,
    EV_sys_shutdown      = 7,
    EV_assertion         = 8,
    EV_su_error          = 9,
    EV_su_scr_start      = 10,
    EV_pkt_pool_timeout  = 11,
    EV_ms_err            = 12,
    LAST_EV_EVENT        = 13
}EV_event;

/*Mass storage ids*/
typedef enum {
    SU_NOSCRIPT     = 0, /*used when no script has ever been chosen to run, saved on SRAM*/
    SU_SCRIPT_1     = 1,
    SU_SCRIPT_2     = 2,
    SU_SCRIPT_3     = 3,
    SU_SCRIPT_4     = 4,
    SU_SCRIPT_5     = 5,
    SU_SCRIPT_6     = 6,
    SU_SCRIPT_7     = 7,
    SU_LOG          = 8,
    WOD_LOG         = 9,
    EXT_WOD_LOG     = 10,
    EVENT_LOG       = 11,
    FOTOS           = 12,
    SCHS            = 13,
    SRAM            = 14,
    LAST_SID        = 15
}MS_sid;

Scheduling_service_state sc_s_state;
Schedule_pkt_pool sch_mem_pool;

union _cnv {
    double cnvD;
    float cnvF;
    uint32_t cnv32;
    uint16_t cnv16[4];
    uint8_t cnv8[8];
};

#define MAX_YEAR 21

static const uint32_t UTC_QB50_YM[MAX_YEAR][13] = {
 /*2000*/{0 , 0, 2678400, 5184000, 7862400, 10454400, 13132800, 15724800, 18403200, 21081600, 23673600, 26352000, 28944000},
 /*2001*/{0 , 31622400, 34300800, 36720000, 39398400, 41990400, 44668800, 47260800, 49939200, 52617600, 55209600, 57888000, 60480000},
 /*2002*/{0 , 63158400, 65836800, 68256000, 70934400, 73526400, 76204800, 78796800, 81475200, 84153600, 86745600, 89424000, 92016000},
 /*2003*/{0 , 94694400, 97372800, 99792000, 102470400, 105062400, 107740800, 110332800, 113011200, 115689600, 118281600, 120960000, 123552000},
 /*2004*/{0 , 126230400, 128908800, 131414400, 134092800, 136684800, 139363200, 141955200, 144633600, 147312000, 149904000, 152582400, 155174400},
 /*2005*/{0 , 157852800, 160531200, 162950400, 165628800, 168220800, 170899200, 173491200, 176169600, 178848000, 181440000, 184118400, 186710400},
 /*2006*/{0 , 189388800, 192067200, 194486400, 197164800, 199756800, 202435200, 205027200, 207705600, 210384000, 212976000, 215654400, 218246400},
 /*2007*/{0 , 220924800, 223603200, 226022400, 228700800, 231292800, 233971200, 236563200, 239241600, 241920000, 244512000, 247190400, 249782400},
 /*2008*/{0 , 252460800, 255139200, 257644800, 260323200, 262915200, 265593600, 268185600, 270864000, 273542400, 276134400, 278812800, 281404800},
 /*2009*/{0 , 284083200, 286761600, 289180800, 291859200, 294451200, 297129600, 299721600, 302400000, 305078400, 307670400, 310348800, 312940800},
 /*2010*/{0 , 315619200, 318297600, 320716800, 323395200, 325987200, 328665600, 331257600, 333936000, 336614400, 339206400, 341884800, 344476800},
 /*2011*/{0 , 347155200, 349833600, 352252800, 354931200, 357523200, 360201600, 362793600, 365472000, 368150400, 370742400, 373420800, 376012800},
 /*2012*/{0 , 378691200, 381369600, 383875200, 386553600, 389145600, 391824000, 394416000, 397094400, 399772800, 402364800, 405043200, 407635200},
 /*2013*/{0 , 410313600, 412992000, 415411200, 418089600, 420681600, 423360000, 425952000, 428630400, 431308800, 433900800, 436579200, 439171200},
 /*2014*/{0 , 441849600, 444528000, 446947200, 449625600, 452217600, 454896000, 457488000, 460166400, 462844800, 465436800, 468115200, 470707200},
 /*2015*/{0 , 473385600, 476064000, 478483200, 481161600, 483753600, 486432000, 489024000, 491702400, 494380800, 496972800, 499651200, 502243200},
 /*2016*/{0 , 504921600, 507600000, 510105600, 512784000, 515376000, 518054400, 520646400, 523324800, 526003200, 528595200, 531273600, 533865600},
 /*2017*/{0 , 536544000, 539222400, 541641600, 544320000, 546912000, 549590400, 552182400, 554860800, 557539200, 560131200, 562809600, 565401600},
 /*2018*/{0 , 568080000, 570758400, 573177600, 575856000, 578448000, 581126400, 583718400, 586396800, 589075200, 591667200, 594345600, 596937600},
 /*2019*/{0 , 599616000, 602294400, 604713600, 607392000, 609984000, 612662400, 615254400, 617932800, 620611200, 623203200, 625881600, 628473600},
 /*2020*/{0 , 631152000, 633830400, 636336000, 639014400, 641606400, 644284800, 646876800, 649555200, 652233600, 654825600, 657504000, 660096000},
                                                   };

static const uint32_t UTC_QB50_D[32] =
        { 0, 86400, 172800, 259200, 345600, 432000, 518400, 604800, 691200, 777600, 864000, 950400, 1036800, 1123200, 1209600,
          1296000, 1382400, 1468800, 1555200, 1641600, 1728000, 1814400, 1900800, 1987200, 2073600, 2160000, 2246400, 2332800,
          2419200, 2505600, 2592000, 2678400
        };

const uint32_t UTC_QB50_H[25] =
        { 0, 3600, 7200, 10800, 14400, 18000, 21600, 25200, 28800, 32400, 36000, 39600, 43200, 46800, 50400, 54000, 57600,
          61200, 64800, 68400, 72000, 75600, 79200, 82800, 86400
        };

struct time_utc {
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};


RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate ;

uint32_t qb_time;
uint16_t msb;
uint16_t lsb;
static uint16_t rep=0;
uint8_t assertion_last_file = 0;
uint16_t assertion_last_line = 0;
static void RCC_Config(void);
static void GPIO_Config(void);
static void USART_Config(void);
void print_time_date();
uint32_t return_time_QB50();
void cnv_UTC_QB50(struct time_utc utc, uint32_t *qb);
void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec);
void HAL_sys_getDate(uint8_t *weekday, uint8_t *mon, uint8_t *date, uint8_t *year);
SAT_returnState route_pkt(tc_tm_pkt *pkt);
SAT_returnState scheduling_service_init();
SAT_returnState cross_schedules();
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
	 RCC_Config();
	 GPIO_Config();
	 USART_Config();

	 /* Enable the PWR APB1 Clock Interface */
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	 /* Allow access to BKP Domain */
	 PWR_BackupAccessCmd(ENABLE);

	 RCC_LSICmd(ENABLE);

	   /* Wait till LSI is ready */
	   while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	   {
	   }

	   /* Select the RTC Clock Source */
	   RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	 /* Enable the LSE OSC */
	// RCC_LSEConfig(RCC_LSE_ON);
	   /* Wait till LSE is ready */
	 //while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	 //{
	 //}
	 /* Select the RTC Clock Source */
     //RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
     RCC_RTCCLKCmd(ENABLE);
     /* Wait for RTC APB registers synchronization */
     RTC_WaitForSynchro();

     RTC_InitTypeDef RTC_InitStructure;
     RTC_StructInit(&RTC_InitStructure);

     scheduling_service_init();

   for(;;)
   {
	   cross_schedules();

   }

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

void print_time_date()
{
   char showtime[50];


   /* Display time Format : hh:mm:ss */
   sprintf((char*)showtime,"%02d:%02d:%02d  ", sTime.RTC_Hours, sTime.RTC_Minutes, sTime.RTC_Seconds);
   UART2_BUF_O_Write_String_To_Buffer("Time: ");
   UART2_BUF_O_Write_String_To_Buffer(showtime);
   UART2_BUF_O_Write_String_To_Buffer("\r\n");
   UART2_BUF_O_Send_All_Data();

   memset(showtime,0,sizeof(showtime));
   sprintf((char*)showtime,"%02d-%2d-%2d\r\n",sDate.RTC_Month, sDate.RTC_Date, 2000 + sDate.RTC_Year);
   UART2_BUF_O_Write_String_To_Buffer("Date: ");
   UART2_BUF_O_Write_String_To_Buffer(showtime);
   UART2_BUF_O_Send_All_Data();
}


uint32_t return_time_QB50(){

    struct time_utc utc;
    uint32_t qb_secs;
    HAL_sys_getTime(&utc.hour, &utc.min, &utc.sec);
    HAL_sys_getDate(&utc.weekday, &utc.month, &utc.day, &utc.year);
    cnv_UTC_QB50(utc, &qb_secs);
    return qb_secs;
}

void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec) {

	//RTC_TimeTypeDef sTime;

	RTC_GetTime(RTC_Format_BIN , &sTime);

	*hours = sTime.RTC_Hours ;
	*mins = sTime.RTC_Minutes ;
	*sec = sTime.RTC_Seconds ;
}

void HAL_sys_getDate(uint8_t *weekday, uint8_t *mon, uint8_t *date, uint8_t *year) {

 	//RTC_DateTypeDef sDate ;
	RTC_GetDate(RTC_Format_BIN , &sDate);

	*weekday = sDate.RTC_WeekDay;
	*mon = sDate.RTC_Month;
	*date = sDate.RTC_Date;
	*year = sDate.RTC_Year;
}

void cnv_UTC_QB50(struct time_utc utc, uint32_t *qb){

    *qb = (UTC_QB50_YM[utc.year][utc.month] +
          UTC_QB50_D[utc.day]               +
          UTC_QB50_H[utc.hour]              +
          (utc.min*60) + utc.sec) - UTC_QB50_D[1];
}

static uint8_t SCH_se =0;
SAT_returnState cross_schedules() {

    if( SCH_se == SC_MAX_STORED_SCHEDULES)
		{
			SCH_se=0;
		}
       if (sch_mem_pool.sc_mem_array[SCH_se].pos_taken == true && /*if a valid schedule exists*/
           sch_mem_pool.sc_mem_array[SCH_se].enabled &&
           sc_s_state.schs_apids_enabled[(sch_mem_pool.sc_mem_array[SCH_se].app_id) - 1] == true){ /*if scheduling enabled for this APID */

           switch(sch_mem_pool.sc_mem_array[SCH_se].sch_evt){
               case REPETITIVE: ;
                   uint32_t qb_time = return_time_QB50();
                   print_time_date();
	                while ( USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);
	                UART2_BUF_O_Write_String_To_Buffer("qb_time: ");
	                UART2_BUF_O_Write_Number10_To_Buffer(qb_time);
	                UART2_BUF_O_Write_String_To_Buffer("\r\n");
	                UART2_BUF_O_Send_All_Data();
	                if((!C_ASSERT(qb_time >= MIN_VALID_QB50_SECS)) == true ) {
                       return SATR_QBTIME_INVALID; }
                    if(sch_mem_pool.sc_mem_array[SCH_se].release_time <= qb_time){ /*time to execute*/
                        route_pkt(&(sch_mem_pool.sc_mem_array[SCH_se].tc_pck));
                        if(!(sch_mem_pool.sc_mem_array[SCH_se].timeout <=0)){ /*to save the else, and go for rescheduling*/
                            sch_mem_pool.sc_mem_array[SCH_se].release_time =
                                (sch_mem_pool.sc_mem_array[SCH_se].release_time + sch_mem_pool.sc_mem_array[SCH_se].timeout);
                            sch_mem_pool.sc_mem_array[SCH_se].pos_taken = true;
                            sch_mem_pool.sc_mem_array[SCH_se].enabled = true;
                           SCH_se++;
                        }/*timeout field is positive */
                        sch_mem_pool.sc_mem_array[SCH_se].pos_taken = false;
                        sch_mem_pool.sc_mem_array[SCH_se].enabled = false;
                        sc_s_state.nmbr_of_ld_sched--;
                        sc_s_state.sch_arr_full = false;
						SCH_se++;
                    }
             }
        }
    /*go to check next schedule*/
    return SATR_OK;
}

SAT_returnState scheduling_service_init(){

    /* Initialize schedules memory.
     * Assign proper memory allocation to inner TC of ScheduleTC for its data payload.
     */
    for(uint8_t s=0;s<SC_MAX_STORED_SCHEDULES;s++){
        sch_mem_pool.sc_mem_array[s].tc_pck.data
                         = sch_mem_pool.innerd_tc_data[s];
    }
    sch_mem_pool.sc_mem_array[0].pos_taken = true; //doesn't change
    sch_mem_pool.sc_mem_array[0].app_id = COMMS_APP_ID;// change
    sch_mem_pool.sc_mem_array[0].seq_count =0;// change
    sch_mem_pool.sc_mem_array[0].enabled = true;//doesn't change
    sch_mem_pool.sc_mem_array[0].sub_schedule_id = 1;//doesn't change
    sch_mem_pool.sc_mem_array[0].num_of_sch_tc = 1;//doesn't change
    sch_mem_pool.sc_mem_array[0].intrlck_set_id = 0;//doesn't change
    sch_mem_pool.sc_mem_array[0].intrlck_ass_id = 0;//doesn't change
    sch_mem_pool.sc_mem_array[0].assmnt_type = 1;//doesn't change
    sch_mem_pool.sc_mem_array[0].sch_evt = REPETITIVE;//doesn't change
    sch_mem_pool.sc_mem_array[0].timeout = 0;//doesn't change
    sch_mem_pool.sc_mem_array[0].tc_pck.seq_flags = 3;// doesn't change
    sch_mem_pool.sc_mem_array[0].tc_pck.ack = 0;//doesn't change
    sch_mem_pool.sc_mem_array[0].tc_pck.verification_state = SATR_PKT_ILLEGAL_APPID;// change



        /* Marks every schedule as invalid, so its position
         * can be taken by a request to the Schedule packet pool.
         */
         sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
         sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
         sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
         sch_mem_pool.sc_mem_array[0].tc_pck.seq_count = 0;// change
         sch_mem_pool.sc_mem_array[0].tc_pck.len =2;// change
         sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_SCHEDULING_SERVICE;// change
         sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = SCHS_REPORT_SCH_DETAILED;// change
         sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

         sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = COMMS_APP_ID;
         sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;

    //TC_VERIFICATION_SERVICE


        sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
        sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
        sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
        sch_mem_pool.sc_mem_array[0].tc_pck.len =2;// change
        sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_VERIFICATION_SERVICE;// change
        sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TM_VR_ACCEPTANCE_SUCCESS;// change
        sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

        sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = 0;
        sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;

        //SYSVIEW_PRINT("V %d", pkt->app_id);


    //TM_VR_ACCEPTANCE_FAILURE

            sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
            sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
            sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
            sch_mem_pool.sc_mem_array[0].tc_pck.len =2;// change
            sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_VERIFICATION_SERVICE;// change
            sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TM_VR_ACCEPTANCE_FAILURE;// change
            sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

            sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = 0;
            sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;

            //SYSVIEW_PRINT("V %d", pkt->app_id);


    //TC_HOUSEKEEPING_SERVICE

                sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.len =2;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_HOUSEKEEPING_SERVICE;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TC_HK_REPORT_PARAMETERS;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = HEALTH_REP;
                sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;



    //TM_HK_PARAMETERS_REPORT

                    sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                    sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                    sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                    sch_mem_pool.sc_mem_array[0].tc_pck.len =2;// change
                    sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_HOUSEKEEPING_SERVICE;// change
                    sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TM_HK_PARAMETERS_REPORT;// change
                    sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                    sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = HEALTH_REP;
                    sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;


    //TC_FUNCTION_MANAGEMENT_SERVICE

                        sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                        sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.len =3;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_FUNCTION_MANAGEMENT_SERVICE;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TC_FM_PERFORM_FUNCTION;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                        sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = P_ON;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = COMMS_DEV_ID;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[2] = 0;


    //TC_EVENT_SERVICE


                        sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                        sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.len =3;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_EVENT_SERVICE;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TC_FM_PERFORM_FUNCTION;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                        sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = EV_inc_pkt;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[2] = 0;


    //TC_TIME_MANAGEMENT_SERVICE

                        sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                        sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.len =7;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_TIME_MANAGEMENT_SERVICE;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TM_TIME_SET_IN_UTC;// change
                        sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                        struct time_utc utc;
                        HAL_sys_getTime(&utc.hour, &utc.min, &utc.sec);
                        HAL_sys_getDate(&utc.weekday, &utc.month, &utc.day, &utc.year);

                        sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = utc.weekday;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = utc.day;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[2] = utc.month;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[3] = utc.year;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[4] = utc.hour;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[5] = utc.min;
                        sch_mem_pool.sc_mem_array[0].tc_pck.data[6] = utc.sec;

    //TC_MASS_STORAGE_SERVICE

                            sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                            sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                            sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                            sch_mem_pool.sc_mem_array[0].tc_pck.len =4;// change
                            sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_MASS_STORAGE_SERVICE;// change
                            sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TC_MS_ENABLE;// change
                            sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                            sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = WOD_LOG;
                            sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;
                            sch_mem_pool.sc_mem_array[0].tc_pck.data[2] = 0;
                            sch_mem_pool.sc_mem_array[0].tc_pck.data[3] = 0;


    //TC_TEST_SERVICE

                                sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                                sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  OBC_APP_ID;// change
                                sch_mem_pool.sc_mem_array[0].tc_pck.type = TM;// change
                                sch_mem_pool.sc_mem_array[0].tc_pck.len =4;// change
                                sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_TEST_SERVICE;// change
                                sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TM_CT_REPORT_TEST;// change
                                sch_mem_pool.sc_mem_array[0].tc_pck.dest_id = COMMS_APP_ID;// change

                                sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = 0;
                                sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;
                                sch_mem_pool.sc_mem_array[0].tc_pck.data[2] = 0;
                                sch_mem_pool.sc_mem_array[0].tc_pck.data[3] = 0;




    sc_s_state.nmbr_of_ld_sched = 2;
    sc_s_state.sch_arr_full = true;

    /*Enable scheduling release for every APID*/
    for(uint8_t s=0;s<LAST_APP_ID-1;s++){
        sc_s_state.schs_apids_enabled[s] = true;
    }

    return SATR_OK;
}

SAT_returnState route_pkt(tc_tm_pkt *pkt)
{
	rep++;
	   UART2_BUF_O_Write_String_To_Buffer("rep: ");
	   UART2_BUF_O_Write_Number10_To_Buffer(rep);
	   UART2_BUF_O_Write_String_To_Buffer("\r\n");
	   UART2_BUF_O_Send_All_Data();
	return 0;
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

uint8_t tst_debugging(uint8_t* file, uint32_t l, char *e) {

  //assertion_last_file = fi;
  assertion_last_line = l;

   UART2_BUF_O_Write_String_To_Buffer("assert_failed\n");
   UART2_BUF_O_Write_String_To_Buffer((char*) file);
   UART2_BUF_O_Write_String_To_Buffer("  Line:  ");
   UART2_BUF_O_Write_Number03_To_Buffer(l);
   UART2_BUF_O_Write_String_To_Buffer("\n");
   UART2_BUF_O_Send_All_Data();

  return false;
}
