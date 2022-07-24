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
#include "fatfs.h"
#include <string.h>
#include <stdio.h>

#define true	1
#define false	0
#define MS_SD_PATH "0:"
#define _OBC_APP_ID_   1
#define _EPS_APP_ID_   2
#define _ADCS_APP_ID_  3
#define _COMMS_APP_ID_ 4
#define _IAC_APP_ID_   5
#define _GND_APP_ID_   6
#define _DBG_APP_ID_   7
#define _LAST_APP_ID_  8
#define SC_MAX_STORED_SCHEDULES 1
#define MAX_PKT_LEN         210
#define MS_MAX_PATH             40
#define MS_SCHS            "/SCHS"
#define SD_PWR_EN_Pin   GPIO_Pin_12
#define SD_PWR_EN_GPIO_Port GPIOA
#define MIN_VALID_QB50_SECS  300 //5min
#define TC 1
#define TM 0
#define TC_SCHEDULING_SERVICE           11
#define C_ASSERT(e)    ((e) ? (true) : (tst_debugging((uint8_t *)__FILE__, __LINE__, #e)))
#define MS_MAX_SU_FILE_SIZE     2048

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


typedef enum {
    WOD_LOG         = 9,
    EXT_WOD_LOG     = 10,
    EVENT_LOG       = 11,
    FOTOS           = 12,
    SCHS            = 13,
    SRAM            = 14,
    LAST_SID        = 15
}MS_sid;

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
    /*FatFs*/
    SATRF_OK                   = 30, /* (0) Succeeded */
    SATRF_DISK_ERR             = 31, /* (1) A hard error occurred in the low level disk I/O layer */
    SATRF_INT_ERR              = 32, /* (2) Assertion failed */
    SATRF_NOT_READY            = 33, /* (3) The physical drive cannot work */
    SATRF_NO_FILE              = 34, /* (4) Could not find the file */
    SATRF_NO_PATH              = 35, /* (5) Could not find the path */
    SATRF_INVALID_NAME         = 36, /* (6) The path name format is invalid */
    SATRF_DENIED               = 37, /* (7) Access denied due to prohibited access or directory full */
    SATRF_EXIST                = 38, /* (8) Access denied due to prohibited access */
    SATRF_INVALID_OBJECT       = 39, /* (9) The file/directory object is invalid */
    SATRF_WRITE_PROTECTED      = 40, /* (10) The physical drive is write protected */
    SATRF_INVALID_DRIVE        = 41, /* (11) The logical drive number is invalid */
    SATRF_NOT_ENABLED          = 42, /* (12) The volume has no work area */
    SATRF_NO_FILESYSTEM        = 43, /* (13) There is no valid FAT volume */
    SATRF_MKFS_ABORTED         = 44, /* (14) The f_mkfs() aborted due to any parameter error */
    SATRF_TIMEOUT              = 45, /* (15) Could not get a grant to access the volume within defined period */
    SATRF_LOCKED               = 46, /* (16) The operation is rejected according to the file sharing policy */
    SATRF_NOT_ENOUGH_CORE      = 47, /* (17) LFN working buffer could not be allocated */
    SATRF_TOO_MANY_OPEN_FILES  = 48, /* (18) Number of open files > _FS_SHARE */
    SATRF_INVALID_PARAMETER    = 49, /* (19) Given parameter is invalid */
    SATRF_DIR_ERROR            = 50,
    SATR_SD_DISABLED           = 51,
    SATR_QUEUE_FULL            = 52,
    SATR_WRONG_DOWNLINK_OFFSET = 53,
    SATR_VER_ERROR             = 54,
	SATR_FIREWALLED            = 55,
    /*LAST*/
    SATR_LAST                  = 56
}SAT_returnState;

#define MS_ERR(res);    ms_debugging(res, __LINE__); \
                        return res + SATRF_OK;

struct _MS_data {
    FATFS Fs;
    uint8_t enabled;
    uint16_t l;
    SAT_returnState last_err;
};
static struct _MS_data MS_data = { .enabled = true, \
                                   .last_err = FR_OK } ;

void ms_debugging(FRESULT res, uint16_t l) {

    MS_data.last_err = res;
    MS_data.last_err = l;
}
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
    REPETITIVE      = 4
}SC_event_time_type;


typedef struct {
    /*Number of loaded schedules*/
    uint8_t nmbr_of_ld_sched;

    uint8_t schs_service_enabled;

    uint8_t sch_arr_full;

    uint8_t schs_apids_enabled[7];

}Scheduling_service_state;


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

uint8_t sche_tc_buffer[MAX_PKT_LEN+14+1];

SAT_returnState mass_storage_init();
SAT_returnState scheduling_service_init();
void cnv16_8(const uint16_t from, uint8_t *to);
void cnv32_8(const uint32_t from, uint8_t *to);
SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);
SAT_returnState mass_storage_dirCheck();
uint32_t return_time_QB50();
void cnv_UTC_QB50(struct time_utc utc, uint32_t *qb);
void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec);
void HAL_sys_getDate(uint8_t *weekday, uint8_t *mon, uint8_t *date, uint8_t *year);
SAT_returnState route_pkt(tc_tm_pkt *pkt);
SAT_returnState scheduling_service_load_schedules();
SAT_returnState cross_schedules();
uint8_t tst_debugging(uint8_t* file, uint32_t l, char *e);
void cnv8_32(uint8_t *from, uint32_t *to);
void cnv8_16(uint8_t *from, uint16_t *to);
void cnv8_16LE(uint8_t *from, uint16_t *to);
void cnvF_8(const float from, uint8_t *to);
void cnv8_F(uint8_t *from, float *to);
void cnvD_8(const double from, uint8_t *to);
void cnv8_D(uint8_t *from, double *to);
SAT_returnState mass_storage_schedule_load_api(MS_sid sid, uint32_t sch_number, uint8_t *buf);

void HAL_obc_SD_ON();
void GPIO_Inits();

// Used to indicate state of timer
#define COUNTING  (0xABCDABCDU)
#define TIMED_OUT (0x54325432U)

// ------ Public function prototypes (declarations) --------------------------

void     TIMEOUT_T3_USEC_Init(void);
void     TIMEOUT_T3_USEC_Start(void);
uint32_t TIMEOUT_T3_USEC_Get_Timer_State(const uint32_t COUNT);
void Delay();
int main(void)
{
	GPIO_Inits();
	MX_FATFS_Init();
	HAL_obc_SD_ON();
	mass_storage_init();
	scheduling_service_init();

	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	 PWR_BackupAccessCmd(ENABLE);
	 RCC_LSEConfig(RCC_LSE_ON);
	 while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	 {
	 }
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();
    RTC_InitTypeDef RTC_InitStructure;
    RTC_StructInit(&RTC_InitStructure);

  for(;;)
  {
	   cross_schedules();

  }
	for(;;);
}


SAT_returnState mass_storage_init() {

    FRESULT res = 0;

    MS_data.enabled = false;
    if((res = f_mount(&MS_data.Fs, MS_SD_PATH, 0)) != FR_OK) { MS_ERR(res); }

   // res = f_mkdir(MS_SCHS);

    MS_data.enabled = true;
    if((res = mass_storage_dirCheck()) != SATR_OK)         { return res; }

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
        sch_mem_pool.sc_mem_array[s].pos_taken = false;
    }

    sc_s_state.nmbr_of_ld_sched = 0;
    sc_s_state.sch_arr_full = false;

    /*Enable scheduling release for every APID*/
    for(uint8_t s=0;s<LAST_APP_ID-1;s++){
        sc_s_state.schs_apids_enabled[s] = true;
    }

    /* Load Schedules from storage.
     *
     */
    scheduling_service_load_schedules();

    return SATR_OK;
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

SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size) {

    FIL fp;
    FRESULT res;
    FILINFO fno;

    uint16_t byteswritten;
    uint8_t path[MS_MAX_PATH];


    if(sid == SCHS)        { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_SCHS, file); }
    else { return SATR_ERROR; }

    res = f_open(&fp, (char*)path, FA_OPEN_ALWAYS | FA_WRITE);

    res = f_write(&fp, buf, *size, (void *)&byteswritten);
    f_close(&fp);

    if(res != FR_OK) { MS_ERR(res); }
    else if(byteswritten == 0) { return SATR_ERROR; }

    return SATR_OK;
}

SAT_returnState mass_storage_dirCheck() {

    FRESULT res;
    FILINFO fno;

    if((res = f_stat((char*)MS_SCHS, &fno)) != FR_OK)           { MS_ERR(res); }

    return SATR_OK;
}

void HAL_obc_SD_ON() {

  for(uint16_t i = 0; i < 1000; i++){
		GPIO_ResetBits(GPIOA, GPIO_Pin_12);
        TIMEOUT_T3_USEC_Init();
        TIMEOUT_T3_USEC_Start();
        while(COUNTING == TIMEOUT_T3_USEC_Get_Timer_State(1000))
        {

        }
		GPIO_SetBits(GPIOA, GPIO_Pin_12);
        TIMEOUT_T3_USEC_Init();
        TIMEOUT_T3_USEC_Start();
        while(COUNTING == TIMEOUT_T3_USEC_Get_Timer_State(1000));
	}
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

void Delay()
{
    TIMEOUT_T3_USEC_Init();
    TIMEOUT_T3_USEC_Start();
    while(COUNTING == TIMEOUT_T3_USEC_Get_Timer_State(1000))
    {

    }
}

void GPIO_Inits()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/*Configure GPIO pin Output Level */
	GPIO_ResetBits(GPIOA, SD_PWR_EN_Pin);


/*Configure GPIO pins : SD_PWR_EN_Pin */
   GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_InitStruct.GPIO_Pin   = SD_PWR_EN_Pin;
   GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIMEOUT_T3_USEC_Init(void)
   {
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

   // Enable T3 clock
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

   TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

   // Time base configuration
   // From 84 MHz T3 clock to 1 MHz
   TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;

   // Set maximum timer overflow value in microseconds: 65000 = 65 ms.
   TIM_TimeBaseStructure.TIM_Period = 65000 - 1;

   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
   }

void TIMEOUT_T3_USEC_Start(void)
   {
   // Reset the timer
   TIM_SetCounter(TIM3, 0);

   // Start the timer
   TIM_Cmd(TIM3, ENABLE);

   }

uint32_t TIMEOUT_T3_USEC_Get_Timer_State(const uint32_t COUNT)
   {

   if (TIM_GetCounter(TIM3) >= COUNT)
      {
      return TIMED_OUT;
      }
   else
      {
      return COUNTING;
      }
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

SAT_returnState scheduling_service_load_schedules(){

    SAT_returnState state = SATR_ERROR;

    for(uint8_t s=0;s<SC_MAX_STORED_SCHEDULES;s++){

        memset(sche_tc_buffer,0x00,MAX_PKT_LEN+14+1);
        state = mass_storage_schedule_load_api(SCHS, s, sche_tc_buffer);

        if( state == SATR_OK){
            uint16_t f_s=0;
            /*read the tc's data length from the first 2 bytes*/
            cnv8_16LE(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].tc_pck.len);
            f_s+=2;
            /*start loading the sch packet*/
            sch_mem_pool.sc_mem_array[s].app_id = (TC_TM_app_id)sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].seq_count = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].enabled = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].sub_schedule_id = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].num_of_sch_tc = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].intrlck_set_id = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].intrlck_ass_id = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].assmnt_type = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].sch_evt = (SC_event_time_type) sche_tc_buffer[f_s];
            f_s+=1;
            cnv8_32(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].release_time);
            f_s+=4;
            cnv8_32(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].timeout);
            f_s+=4;
            /*TC parsing begins here*/
            sch_mem_pool.sc_mem_array[s].tc_pck.app_id = (TC_TM_app_id) sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.type = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.seq_flags = sche_tc_buffer[f_s];
            f_s+=1;
            cnv8_16LE(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].tc_pck.seq_count);
            f_s+=2;
            cnv8_16LE(&sche_tc_buffer[f_s], &sch_mem_pool.sc_mem_array[s].tc_pck.len);
            f_s+=2;
            sch_mem_pool.sc_mem_array[s].tc_pck.ack = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.ser_type = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.ser_subtype = sche_tc_buffer[f_s];
            f_s+=1;
            sch_mem_pool.sc_mem_array[s].tc_pck.dest_id = (TC_TM_app_id) sche_tc_buffer[f_s];
            f_s+=1;
            /*copy tc payload data*/
            uint16_t i = 0;
            for(;i<sch_mem_pool.sc_mem_array[s].tc_pck.len;i++){
                sch_mem_pool.sc_mem_array[s].tc_pck.data[i] = sche_tc_buffer[f_s];
                f_s+=1;
            }
            sch_mem_pool.sc_mem_array[s].tc_pck.verification_state = (SAT_returnState) sche_tc_buffer[f_s];
            f_s+=1;
            uint8_t l_chk = sche_tc_buffer[f_s];

            uint8_t chk = 0;
            for(uint16_t l=0;l<f_s-1;l++){
                chk = chk ^ sche_tc_buffer[l];
            }

            if( l_chk == chk && chk!= 0x0 ){
                sch_mem_pool.sc_mem_array[s].pos_taken = true;
                sch_mem_pool.sc_mem_array[s].enabled = true;
            }
            else{
                sch_mem_pool.sc_mem_array[s].pos_taken = false;
                sch_mem_pool.sc_mem_array[s].enabled = false;
            }
        }
    }
    return SATR_OK;
}

SAT_returnState route_pkt(tc_tm_pkt *pkt)
{
	SAT_returnState res;
	TC_TM_app_id id;

    if(pkt->type == TC)         { id = pkt->app_id; }
    else if(pkt->type == TM)    { id = pkt->dest_id; }

    if(id == COMMS_APP_ID)
    {
    	return SATR_OK;
    }
    return SATR_OK;
}


SAT_returnState cross_schedules() {
	static uint8_t SCH_se =0;
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
	                if((!C_ASSERT(qb_time >= MIN_VALID_QB50_SECS)) == true ) {
                       return SATR_QBTIME_INVALID; }
                    if(sch_mem_pool.sc_mem_array[SCH_se].release_time <= qb_time){ /*time to execute*/
    	               // UART2_BUF_O_Write_String_To_Buffer("SCHS ROUTING PKT");
    	                //UART2_BUF_O_Send_All_Data();
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

uint8_t tst_debugging(uint8_t* file, uint32_t l, char *e) {

  //assertion_last_file = fi;
  //assertion_last_line = l;


  return false;
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

SAT_returnState mass_storage_schedule_load_api(MS_sid sid, uint32_t sch_number, uint8_t *buf) {

    FIL fp;
    FRESULT res;
    uint8_t path[MS_MAX_PATH];
    uint16_t size = 0;

    if((!C_ASSERT(MS_data.enabled == true)) == true) { return SATR_SD_DISABLED; }
    if((!C_ASSERT( sid == SCHS)) == true) { return SATR_INV_STORE_ID; }
    if((!C_ASSERT( sch_number <= 14 )) == true) { return SATR_INV_STORE_ID; }

    snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_SCHS, sch_number);

    for(uint8_t i = 0; i < 3; i++) {
        res = f_open(&fp, (char*)path, FA_OPEN_EXISTING | FA_READ);
        if(res != FR_OK) {
            Delay();
            continue;
        }
        res = f_read(&fp, buf, MS_MAX_SU_FILE_SIZE, (void *)&size);
        f_close(&fp);
        if(res == FR_OK) {
            break;
        }
        Delay();
    }


    if(res != FR_OK)    { MS_ERR(res); }
    else if(size == 0)  { return SATR_ERROR; }

    return SATR_OK;
}
