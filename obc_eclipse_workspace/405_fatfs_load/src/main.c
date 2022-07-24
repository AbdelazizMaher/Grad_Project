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
#define SC_MAX_STORED_SCHEDULES 15
#define MAX_PKT_LEN         210
#define MS_MAX_PATH             40
#define MS_SCHS            "/SCHS"
#define SD_PWR_EN_Pin   GPIO_Pin_12
#define SD_PWR_EN_GPIO_Port GPIOA

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

    ABSOLUTE        = 0,

    SCHEDULE        = 1,

    SUBSCHEDULE     = 2,

    INTERLOCK       = 3,

    REPETITIVE      = 4,
    LAST_EVENTTIME  = 5

}SC_event_time_type;


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


uint8_t sche_tc_buffer[MAX_PKT_LEN+14+1];

SAT_returnState mass_storage_init();
SAT_returnState scheduling_service_init();
void cnv16_8(const uint16_t from, uint8_t *to);
void cnv32_8(const uint32_t from, uint8_t *to);
SAT_returnState scheduling_service_save_schedules();
SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);
SAT_returnState mass_storage_dirCheck();

void HAL_obc_SD_ON();
void GPIO_Inits();

// Used to indicate state of timer
#define COUNTING  (0xABCDABCDU)
#define TIMED_OUT (0x54325432U)

// ------ Public function prototypes (declarations) --------------------------

void     TIMEOUT_T3_USEC_Init(void);
void     TIMEOUT_T3_USEC_Start(void);
uint32_t TIMEOUT_T3_USEC_Get_Timer_State(const uint32_t COUNT);

int main(void)
{
	GPIO_Inits();
	MX_FATFS_Init();
	HAL_obc_SD_ON();
	mass_storage_init();
	scheduling_service_init();

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
    sch_mem_pool.sc_mem_array[0].tc_pck.data
                     = sch_mem_pool.innerd_tc_data[0];

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
    sch_mem_pool.sc_mem_array[0].tc_pck.verification_state = SATR_PKT_INIT;// change
    //TC_HOUSEKEEPING_SERVICE

                sch_mem_pool.sc_mem_array[0].release_time = 420;//7min
                sch_mem_pool.sc_mem_array[0].tc_pck.app_id =  COMMS_APP_ID;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.type = TC;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.len =1;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.ser_type = TC_HOUSEKEEPING_SERVICE;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.ser_subtype = TC_HK_REPORT_PARAMETERS;// change
                sch_mem_pool.sc_mem_array[0].tc_pck.dest_id =  OBC_APP_ID;// change

                sch_mem_pool.sc_mem_array[0].tc_pck.data[0] = HEALTH_REP;
                //sch_mem_pool.sc_mem_array[0].tc_pck.data[1] = 0;

//    for(uint8_t s=0;s<1;s++){
//        sch_mem_pool.sc_mem_array[s].tc_pck.data
//                         = sch_mem_pool.innerd_tc_data[s];
//
//        /* Marks every schedule as invalid, so its position
//         * can be taken by a request to the Schedule packet pool.
//         */
//        sch_mem_pool.sc_mem_array[s].pos_taken = false;
//        sch_mem_pool.sc_mem_array[s].tc_pck.len=1;
//        sch_mem_pool.sc_mem_array[s].app_id = OBC_APP_ID;
//        sch_mem_pool.sc_mem_array[s].seq_count =0;
//        sch_mem_pool.sc_mem_array[s].enabled;
//        sch_mem_pool.sc_mem_array[s].sub_schedule_id;
//        sch_mem_pool.sc_mem_array[s].num_of_sch_tc;
//        sch_mem_pool.sc_mem_array[s].intrlck_set_id;
//        sch_mem_pool.sc_mem_array[s].intrlck_ass_id;
//        sch_mem_pool.sc_mem_array[s].assmnt_type;
//        sch_mem_pool.sc_mem_array[s].sch_evt;
//        sch_mem_pool.sc_mem_array[s].release_time;
//        sch_mem_pool.sc_mem_array[s].timeout;
//        sch_mem_pool.sc_mem_array[s].tc_pck.app_id;
//        sch_mem_pool.sc_mem_array[s].tc_pck.type;
//        sch_mem_pool.sc_mem_array[s].tc_pck.seq_flags;
//        sch_mem_pool.sc_mem_array[s].tc_pck.seq_count;
//        sch_mem_pool.sc_mem_array[s].tc_pck.len;
//        sch_mem_pool.sc_mem_array[s].tc_pck.ack;
//        sch_mem_pool.sc_mem_array[s].tc_pck.ser_type;
//        sch_mem_pool.sc_mem_array[s].tc_pck.ser_subtype;
//        sch_mem_pool.sc_mem_array[s].tc_pck.dest_id;
//        uint16_t i = 0;
//         for(;i<sch_mem_pool.sc_mem_array[s].tc_pck.len;i++){
//               sch_mem_pool.sc_mem_array[s].tc_pck.data[i] ;
//             }
//        sch_mem_pool.sc_mem_array[s].tc_pck.verification_state;
//    }

//    sc_s_state.nmbr_of_ld_sched = 0;
//    sc_s_state.sch_arr_full = false;

    /*Enable scheduling release for every APID*/
    for(uint8_t s=0;s<LAST_APP_ID-1;s++){
        sc_s_state.schs_apids_enabled[s] = true;
    }

    scheduling_service_save_schedules();
    return SATR_OK;
}

SAT_returnState scheduling_service_save_schedules(){

    /*convert the Schedule packet from Schedule_pkt_pool format to an array of linear bytes*/
    for(uint8_t s=0;s<1;s++){

            memset(sche_tc_buffer,0x00,MAX_PKT_LEN+14+1);
            uint16_t f_s=0;
            /*save the tc's data length in the first 2 bytes*/
            cnv16_8(sch_mem_pool.sc_mem_array[s].tc_pck.len, &sche_tc_buffer[f_s]);
            f_s+=2;
            /*start saving sch packet*/
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].app_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].seq_count;
            f_s+=1;
    //        cnv16_8(sch_mem_pool.sc_mem_array[s].seq_count, &sche_tc_buffer[3]);
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].enabled;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].sub_schedule_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].num_of_sch_tc;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].intrlck_set_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].intrlck_ass_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].assmnt_type;
            f_s+=1;
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].sch_evt;
            f_s+=1;
            cnv32_8(sch_mem_pool.sc_mem_array[s].release_time,&sche_tc_buffer[f_s]); //11
            f_s+=4;
            cnv32_8(sch_mem_pool.sc_mem_array[s].timeout,&sche_tc_buffer[f_s]); //15
            f_s+=4;
            /*TC parsing begins here*/
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].tc_pck.app_id;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.type;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.seq_flags;
            f_s+=1;
            cnv16_8(sch_mem_pool.sc_mem_array[s].tc_pck.seq_count,&sche_tc_buffer[f_s]);
            f_s+=2;
            cnv16_8(sch_mem_pool.sc_mem_array[s].tc_pck.len,&sche_tc_buffer[f_s]);
            f_s+=2;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.ack;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.ser_type;
            f_s+=1;
            sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.ser_subtype;
            f_s+=1;
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].tc_pck.dest_id;
            f_s+=1;
            /*copy tc payload data*/
            uint16_t i = 0;
            for(;i<sch_mem_pool.sc_mem_array[s].tc_pck.len;i++){
                sche_tc_buffer[f_s] = sch_mem_pool.sc_mem_array[s].tc_pck.data[i];
                f_s+=1;
            }
//            f_s+=f_s+i;
            sche_tc_buffer[f_s] = (uint8_t)sch_mem_pool.sc_mem_array[s].tc_pck.verification_state;
            f_s+=1;
            uint8_t chk = 0;
            for(uint16_t l=0;l<f_s-1;l++){
                chk = chk ^ sche_tc_buffer[l];
            }
            sche_tc_buffer[f_s] = chk;
            f_s+=1;
            mass_storage_storeFile(SCHS,s,sche_tc_buffer,&f_s);

    }
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

