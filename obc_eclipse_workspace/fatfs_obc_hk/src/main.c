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
#define WOD_MAX_BUFFER 224
#define SYSTEM_APP_ID _OBC_APP_ID_
#define TC_HOUSEKEEPING_SERVICE         3
#define TC_ACK_NO           0x00
#define TC 1
#define TM 0
#define UART_BUF_SIZE 4096
#define MAX_PKT_SIZE  2063
#define TC_HK_REPORT_PARAMETERS         21
#define TM_HK_PARAMETERS_REPORT         23
#define MS_WOD_LOG         "/WOD_LOG"
#define MS_EXT_WOD_LOG     "/EXT_WOD"
#define MS_EVENT_LOG       "/EV_LOG"
#define MS_FOTOS           "/FOTOS"
#define MS_SCHS            "/SCHS"
#define MS_MAX_FILES            5000
#define MS_MAX_PATH             40

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

static tc_tm_pkt hk_pkt;
static uint8_t hk_pkt_data[264];
#define POOL_PKT_TOTAL_SIZE  20

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

struct adcs_data{
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
    int16_t rolldot;
    int16_t pitchdot;
    int16_t yawdot;
    int16_t x_eci;
    int16_t y_eci;
    int16_t z_eci;
};

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

    struct adcs_data attitude_data;

    struct uart_data comms_uart;
    struct uart_data adcs_uart;
    struct uart_data eps_uart;

    uint16_t vbat;
    uint32_t adc_time;
    uint8_t adc_flag;

};

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


struct _obc_data obc_data = { .comms_uart.init_time = 0,
                              .adcs_uart.init_time = 0,
                              .eps_uart.init_time = 0,
                              .vbat = 0,
                              .adc_time = 0,
                              .adc_flag = false };

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

SAT_returnState mass_storage_init();
SAT_returnState wod_log();
void clear_wod();
SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid);
SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt);
SAT_returnState wod_log_load(uint8_t *buf);
SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size);
uint16_t get_new_fileId(MS_sid sid);
void hk_INIT();
SAT_returnState mass_storage_dirCheck();
SAT_returnState crt_pkt(tc_tm_pkt *pkt, TC_TM_app_id app_id, uint8_t type, uint8_t ack, uint8_t ser_type, uint8_t ser_subtype, TC_TM_app_id dest_id);
int main(void)
{
	MX_FATFS_Init();

	mass_storage_init();
    wod_log();
    clear_wod();
    hk_INIT();
    hk_crt_pkt_TM(&hk_pkt, COMMS_APP_ID, WOD_REP);
   // route_pkt(&hk_pkt);

	for(;;);
}


SAT_returnState mass_storage_init() {

    FRESULT res = 0;

    MS_data.enabled = false;
    if((res = f_mount(&MS_data.Fs, MS_SD_PATH, 0)) != FR_OK) { MS_ERR(res); }

    res = f_mkdir(MS_WOD_LOG);

    MS_data.enabled = true;
    if((res = mass_storage_dirCheck()) != SATR_OK)         { return res; }

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

void clear_wod() {
    sat_status.batt_curr = 0;
    sat_status.batt_volt = 0;
    sat_status.bus_3v3_curr = 0;
    sat_status.bus_5v_curr = 0;
    sat_status.temp_eps = 0;
    sat_status.temp_batt = 0;
    sat_status.temp_comms = 0;
}

SAT_returnState hk_crt_pkt_TM(tc_tm_pkt *pkt, const TC_TM_app_id app_id, const HK_struct_id sid) {

    hk_report_parameters(sid, pkt);

    crt_pkt(pkt, (TC_TM_app_id)SYSTEM_APP_ID, TM, TC_ACK_NO, TC_HOUSEKEEPING_SERVICE, TM_HK_PARAMETERS_REPORT, app_id);

    return SATR_OK;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {

    pkt->data[0] = (HK_struct_id)sid;

    if(sid == WOD_REP) {

        wod_log_load(&pkt->data[5]);

        uint16_t size = 4+(32*7);
        mass_storage_storeFile(WOD_LOG, 0, &pkt->data[1], &size);
        pkt->len = 1+4+(32*7);
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

SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size) {

    FIL fp;
    FRESULT res;
    FILINFO fno;

    uint16_t byteswritten;
    uint8_t path[MS_MAX_PATH];

    if(sid == WOD_LOG ||
       sid == EXT_WOD_LOG ||
       sid == FOTOS) {
        file = get_new_fileId(sid);
    }

    if(sid == WOD_LOG)          { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_WOD_LOG, file); }
    else if(sid == EXT_WOD_LOG) { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_EXT_WOD_LOG, file); }
    else if(sid == EVENT_LOG)   { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_EVENT_LOG, file); }
    else if(sid == FOTOS)       { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_FOTOS, file); }
    else if(sid == SCHS)        { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_SCHS, file); }
    else { return SATR_ERROR; }

    res = f_open(&fp, (char*)path, FA_OPEN_ALWAYS | FA_WRITE);

    res = f_write(&fp, buf, *size, (void *)&byteswritten);
    f_close(&fp);

    if(res != FR_OK) { MS_ERR(res); }
    else if(byteswritten == 0) { return SATR_ERROR; }

    return SATR_OK;
}

uint16_t get_new_fileId(MS_sid sid) {

    uint16_t *head = 0;
    uint16_t *tail = 0;

    if(sid == WOD_LOG)          { head = obc_data.fs_wod_head; tail = obc_data.fs_wod_tail; }
    else if(sid == EXT_WOD_LOG) { head = obc_data.fs_ext_head; tail = obc_data.fs_ext_tail; }
    else if(sid == EVENT_LOG)   { head = obc_data.fs_ev_head;  tail = obc_data.fs_ev_tail; }
    else if(sid == FOTOS) {
        (*obc_data.fs_fotos)++;
        if(*obc_data.fs_fotos > MS_MAX_FILES) {
            *obc_data.fs_fotos = 1;
        }
        return *obc_data.fs_fotos;
    }

    (*head)++;
    if(*head > MS_MAX_FILES) {
        *head = 1;
    }
    if(*head == *tail) {
        (*tail)++;
        if(*tail > MS_MAX_FILES) {
            *tail = 1;
        }
    }
    return *head;
}

SAT_returnState crt_pkt(tc_tm_pkt *pkt, TC_TM_app_id app_id, uint8_t type, uint8_t ack, uint8_t ser_type, uint8_t ser_subtype, TC_TM_app_id dest_id) {

    pkt->type = type;
    pkt->app_id = app_id;
    pkt->dest_id = dest_id;
    pkt->ser_type = ser_type;
    pkt->ser_subtype = ser_subtype;
    pkt->verification_state = SATR_PKT_INIT;

    return SATR_OK;
}

void hk_INIT() {
   hk_pkt.data = hk_pkt_data;
}

SAT_returnState mass_storage_dirCheck() {

    FRESULT res;
    FILINFO fno;

    if((res = f_stat((char*)MS_WOD_LOG, &fno)) != FR_OK)        { MS_ERR(res); }
    return SATR_OK;
}
