// Module header
#include "HELLOWORLD_TRANSMIT_task.h"
#include <string.h>

#include "ttrd2-05a-t0401a-v001a_uart2_buff_o_task.h"

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
#define TC_SC_ENABLE_RELEASE            1
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

char showtime[50];
SAT_returnState route_pkt(tc_tm_pkt *pkt);
SAT_returnState scheduling_service_init();
SAT_returnState cross_schedules();
uint8_t tst_debugging(uint8_t* file, uint32_t l, char *e);

void HELLOWORLD_TRANSMIT_Init(void)
{
	 RCC_Config();
	
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
	 RCC_LSEConfig(RCC_LSE_ON);
	   /* Wait till LSE is ready */
	// while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	 {
	 }
	 /* Select the RTC Clock Source */
    // RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
     RCC_RTCCLKCmd(ENABLE);
     /* Wait for RTC APB registers synchronization */
     RTC_WaitForSynchro();

     RTC_InitTypeDef RTC_InitStructure;
     RTC_StructInit(&RTC_InitStructure);
	 
     scheduling_service_init();
}

uint32_t HELLOWORLD_TRANSMIT_Update(void)
{	
   cross_schedules();
	   UART2_BUF_O_Write_String_To_Buffer("rep: ");
	   UART2_BUF_O_Write_Number10_To_Buffer(rep);
	   UART2_BUF_O_Write_String_To_Buffer("\r\n");
	   UART2_BUF_O_Send_All_Data();  
	
    return RETURN_NORMAL_STATE;	  
}

static void RCC_Config(void)
{
   /* Enable GPIO clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Enable USARTs Clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}


void print_time_date()
{
    /* Display time Format : hh:mm:ss */
    sprintf((char*)showtime,"%02d:%02d:%02d\r\n ",sTime.RTC_Hours, sTime.RTC_Minutes, sTime.RTC_Seconds);
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

 //	RTC_DateTypeDef sDate ;
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
                   if(!C_ASSERT(qb_time >= MIN_VALID_QB50_SECS) == true ) {
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

        /* Marks every schedule as invalid, so its position
         * can be taken by a request to the Schedule packet pool.
         */
        sch_mem_pool.sc_mem_array[s].pos_taken = true;

         sch_mem_pool.sc_mem_array[s].tc_pck.len=1;
         sch_mem_pool.sc_mem_array[s].app_id = OBC_APP_ID;
         sch_mem_pool.sc_mem_array[s].seq_count =0;
         sch_mem_pool.sc_mem_array[s].enabled = true;
         sch_mem_pool.sc_mem_array[s].sub_schedule_id = 1;
         sch_mem_pool.sc_mem_array[s].num_of_sch_tc = 1;
         sch_mem_pool.sc_mem_array[s].intrlck_set_id = 0;
         sch_mem_pool.sc_mem_array[s].intrlck_ass_id = 0;
         sch_mem_pool.sc_mem_array[s].assmnt_type = 1;
         sch_mem_pool.sc_mem_array[s].sch_evt = REPETITIVE;
         sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
         sch_mem_pool.sc_mem_array[1].release_time = 600;//10min
         sch_mem_pool.sc_mem_array[s].timeout = 0;
         sch_mem_pool.sc_mem_array[s].tc_pck.app_id = OBC_APP_ID ;
         sch_mem_pool.sc_mem_array[s].tc_pck.type = TM;
         sch_mem_pool.sc_mem_array[s].tc_pck.seq_flags =0;
         sch_mem_pool.sc_mem_array[s].tc_pck.seq_count = 0;
         sch_mem_pool.sc_mem_array[s].tc_pck.len =0;
         sch_mem_pool.sc_mem_array[s].tc_pck.ack = 0;
         sch_mem_pool.sc_mem_array[s].tc_pck.ser_type = TC_SCHEDULING_SERVICE;
         sch_mem_pool.sc_mem_array[s].tc_pck.ser_subtype = TC_SC_ENABLE_RELEASE;
         sch_mem_pool.sc_mem_array[s].tc_pck.dest_id = OBC_APP_ID;
         uint16_t i = 0;
          for(;i<sch_mem_pool.sc_mem_array[s].tc_pck.len;i++){
                sch_mem_pool.sc_mem_array[s].tc_pck.data[i] = 0;
              }
         sch_mem_pool.sc_mem_array[s].tc_pck.verification_state = SATR_PKT_ILLEGAL_APPID;
     }

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
	return 0;
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