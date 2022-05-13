#include "mass_storage_service.h"

#include "pkt_pool.h"

#include <stdlib.h>
#include <string.h>
#include "fatfs.h"
#include "service_utilities.h"
#include "obc.h"
#include "power_ctrl.h"

#define MS_SD_PATH "0:"

#define MS_WOD_LOG         "/WOD_LOG"
#define MS_EXT_WOD_LOG     "/EXT_WOD"
#define MS_EVENT_LOG       "/EV_LOG"
#define MS_FOTOS           "/FOTOS"
#define MS_SCHS            "/SCHS"

#define MS_MAX_PATH             40 //random num
#define MS_MAX_FNAME            30 //random num
#define MS_MAX_LOG_FILE_SIZE    198 /*MAX_PKT_DATA*/ 
#define MS_MAX_SU_FILE_SIZE     2048 //2k
#define MS_FILE_SECTOR          512
#define MS_STORES               3
#define MS_SU_FSIZE             174
#define MS_MIN_SU_FILE          1   //min is the header.
#define MAX_F_RETRIES           3

#define LOGS_LIST_SIZE          10

#undef __FILE_ID__
#define __FILE_ID__ 7

extern SAT_returnState sram_hard_delete();

#define MS_ERR(res);    ms_debugging(res, __LINE__); \
                        return res + SATRF_OK;

struct _MS_data {
    FATFS Fs;
    uint8_t enabled;
    uint16_t l;
    SAT_returnState last_err;
};

extern SAT_returnState route_pkt(tc_tm_pkt *pkt);

static struct _MS_data MS_data = { .enabled = true, \
                                   .last_err = FR_OK } ;

void ms_debugging(FRESULT res, uint16_t l) {

    MS_data.last_err = res;
    MS_data.last_err = l;

}

void ms_get_state(uint8_t *res, uint8_t *status, uint16_t *l) {

    *res = MS_data.last_err;
    *status = MS_data.enabled;
    *l = MS_data.l;

}

/**
 * @brief      { entry point for incoming packets. }
 *
 * @param      pkt   The pkt
 *
 * @return     { description_of_the_return_value }
 */
SAT_returnState mass_storage_app(tc_tm_pkt *pkt) {

    SAT_returnState res = SATR_ERROR;

    if((!C_ASSERT(pkt != NULL && pkt->data != NULL)) == true) { return SATR_ERROR; }
    if((!C_ASSERT(pkt->ser_type == TC_MASS_STORAGE_SERVICE)) == true) { return SATR_ERROR; }
    if((!C_ASSERT(pkt->ser_subtype == TC_MS_DELETE ||
                 pkt->ser_subtype == TC_MS_REPORT ||
                 pkt->ser_subtype == TC_MS_DOWNLINK ||
                 pkt->ser_subtype == TC_MS_LIST ||
                 pkt->ser_subtype == TC_MS_UPLINK)) == true) { return SATR_ERROR; }

    MS_sid sid = (MS_sid)pkt->data[0];

    if((!C_ASSERT(sid < LAST_SID)) == true) { return SATR_ERROR; }

    if(pkt->ser_subtype == TC_MS_DELETE) {

        uint16_t to;
        MS_mode mode;
        mode = pkt->data[1];
			
        if(mode == HARD_DELETE) {

            if(sid == SRAM) {
              res = sram_hard_delete();
            }else{
                
              res = mass_storage_hard_delete(sid);
            }
        }else{

           cnv8_16(&pkt->data[2], &to);
           res = mass_storage_delete_api(sid, to, mode);
        }

    } else if(pkt->ser_subtype == TC_MS_REPORT) {

        res = mass_storage_report_api(pkt);

    } else if(pkt->ser_subtype == TC_MS_LIST) {
			
        res = mass_storage_list_api(pkt, sid);
        
    } else if(pkt->ser_subtype == TC_MS_DOWNLINK) {

        uint16_t file;

        cnv8_16(&pkt->data[1], &file);
        uint8_t num = pkt->data[3];
        
        res = mass_storage_downlink_api(pkt, sid, file, num);

    } else if(pkt->ser_subtype == TC_MS_UPLINK) {

        uint16_t size = pkt->len - 3;
        uint16_t file = 0;

        cnv8_16(&pkt->data[1], &file);

        res = mass_storage_storeFile(sid, 0,&pkt->data[3], &size);

    } else { 
        res = SATR_ERROR;
    }

    pkt->verification_state = res;

    return res; 
}

/*delete handles deletion of mass storage. sid denotes the store id.*/
/*if to is 0: it deletes every file of the sid else it deletes every file which time is lower then the time denoted in to*/
SAT_returnState mass_storage_hard_delete(MS_sid sid) {

    FRESULT res;
    FILINFO fno;
    DIR dir;
    uint8_t *fn;
    uint8_t path[MS_MAX_PATH];
    uint8_t temp_path[MS_MAX_PATH];
    uint16_t i;

    if((!C_ASSERT(MS_data.enabled == true)) == true) { return SATR_SD_DISABLED; }
    if((!C_ASSERT(sid == WOD_LOG || \
                 sid == EVENT_LOG || \
                 sid == FOTOS)) == true) { return SATR_ERROR; }

    if(sid == WOD_LOG)          { strncpy((char*)path, MS_WOD_LOG, MS_MAX_PATH); }
    else if(sid == EXT_WOD_LOG) { strncpy((char*)path, MS_EXT_WOD_LOG, MS_MAX_PATH); }
    else if(sid == EVENT_LOG)   { strncpy((char*)path, MS_EVENT_LOG, MS_MAX_PATH); }
    else if(sid == FOTOS)       { strncpy((char*)path, MS_FOTOS, MS_MAX_PATH); }

    if((res = f_opendir(&dir, (char*)path)) != FR_OK) { MS_ERR(res); }  //add more error checking

    for (i = 0; i < (MS_MAX_FILES * 2); i++) {

        if((res = f_readdir(&dir, &fno)) != FR_OK) {
            f_closedir(&dir);
            MS_ERR(res); 
        }  /* Break on error */
        else if(fno.fname[0] == 0) { break; }  /* Break on end of dir */
        else if (fno.fname[0] == '.') { continue; }             /* Ignore dot entry */

        fn = (uint8_t*)fno.fname;

        uint32_t ret = strtol((char*)fn, NULL, 10);

        sprintf(temp_path,"%s/%s", path, (char*)fn);

        if((res = f_stat((char*)temp_path, &fno)) != FR_OK) {
            f_closedir(&dir);
            MS_ERR(res);
        }

        if((res = f_unlink((char*)temp_path)) != FR_OK)     { 
            f_closedir(&dir); 
            MS_ERR(res);
        }
    }
    f_closedir(&dir);

    if(i == MS_MAX_FILES - 1) { return SATR_MS_MAX_FILES; }
    
    return SATR_OK;
}

extern uint8_t uart_temp[];

SAT_returnState mass_storage_downlinkFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size) {

    FIL fp;
    FRESULT res;
    uint16_t byteswritten;
    uint8_t path[MS_MAX_PATH];


    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(buf != NULL && size != NULL) == true)  { return SATR_ERROR; }
    if(!C_ASSERT(sid < SRAM) == true)               { return SATR_ERROR; }

    if(sid >= SU_LOG && sid <= EVENT_LOG) { file = get_filePos(sid, file); }

    /*cp dir belonging to sid*/
    if(sid == WOD_LOG)     { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_WOD_LOG, file); }
    else if(sid == EXT_WOD_LOG) { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_EXT_WOD_LOG, file); }
    else if(sid == EVENT_LOG)   { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_EVENT_LOG, file); }
    else if(sid == FOTOS)       { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_FOTOS, file); }
    else if(sid == SCHS)        { snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_SCHS, file); }
    else { return SATR_ERROR; }

    *size = MAX_PKT_EXT_DATA;

    if((res = f_open(&fp, (char*)path, FA_OPEN_EXISTING | FA_READ)) != FR_OK) { MS_ERR(res); } 

    res = f_read(&fp, buf, *size, (void *)&byteswritten);
    f_close(&fp);

    if(res != FR_OK) { MS_ERR(res); } 
    else if(byteswritten == 0) { MS_ERR(res); } 
    *size = byteswritten;

    return SATR_OK;
}

SAT_returnState mass_storage_storeFile(MS_sid sid, uint32_t file, uint8_t *buf, uint16_t *size) {

    FIL fp;
    FRESULT res;
    FILINFO fno;

    uint16_t byteswritten;
    uint8_t path[MS_MAX_PATH];


    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(buf != NULL && size != NULL) == true)  { return SATR_ERROR; }
    //if(!C_ASSERT(*size > 0 && *size < _MAX_SS) == true) { return SATR_ERROR; }
    if(!C_ASSERT(sid <= SRAM) == true)              { return SATR_ERROR; }

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

SAT_returnState mass_storage_list_api(tc_tm_pkt *pkt, MS_sid sid) {

    SAT_returnState res = SATR_ERROR;
    uint16_t size = 0;
    tc_tm_pkt *temp_pkt = 0;

    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }
    if(!C_ASSERT(sid != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }

    TC_TM_app_id app_id = (TC_TM_app_id)pkt->dest_id; //check if this is ok

    mass_storage_crtPkt(&temp_pkt, app_id, MAX_PKT_EXT_DATA);

    uint16_t iter = 0;

    cnv8_16(&pkt->data[1], &iter);

    temp_pkt->data[0] = sid;

    /*in case the store is empty it puts the file as zero to indicate that*/
    temp_pkt->data[1] = 0;
    temp_pkt->data[2] = 0;

    res = mass_storage_list(sid, &temp_pkt->data[1], &size, &iter);

    if(!(res == SATR_OK || res == SATR_EOT || res == SATR_MS_MAX_FILES)) { free_pkt(temp_pkt); return res; }

    if(size == 0) { size = 2; }
    size++;

    mass_storage_updatePkt(temp_pkt, size, TM_MS_CATALOGUE_LIST);
    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState mass_storage_list(MS_sid sid, uint8_t *buf, uint16_t *size, uint16_t *iter) {

    DIR dir;
    FILINFO fno;
    FRESULT res = 0;
    uint16_t ret;
    uint8_t *fn;
    uint8_t start_flag = 0;
    uint8_t path[MS_MAX_PATH];
    uint8_t temp_path[MS_MAX_PATH];
    uint16_t i;

    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(buf != NULL && size != NULL && iter != NULL) == true)                            { return SATR_ERROR; }
    if(!C_ASSERT(*size == 0) == true)                                                             { return SATR_ERROR; }
    if(!C_ASSERT(sid == WOD_LOG || sid == EXT_WOD_LOG || sid == EVENT_LOG || sid == FOTOS) == true)    { return SATR_ERROR; }

    if(sid == WOD_LOG)          { strncpy((char*)path, MS_WOD_LOG, MS_MAX_PATH); }
    else if(sid == EXT_WOD_LOG) { strncpy((char*)path, MS_EXT_WOD_LOG, MS_MAX_PATH); }
    else if(sid == EVENT_LOG)   { strncpy((char*)path, MS_EVENT_LOG, MS_MAX_PATH); }
    else if(sid == FOTOS)       { strncpy((char*)path, MS_FOTOS, MS_MAX_PATH); }

    if(*iter == 0) { start_flag = 1; }
    else { start_flag = 0; }

    /*first filename should be the file for the next iteration, 0 if its not reached max pkt len*/
    *size += sizeof(uint16_t);

    if((res = f_opendir(&dir, (char*)path)) != FR_OK) { MS_ERR(res); }
    for (i = 0; i < MS_MAX_FILES*2; i++) {

        if((res = f_readdir(&dir, &fno)) != FR_OK) { f_closedir(&dir); MS_ERR(res); }  /* Break on error */
        else if(fno.fname[0] == 0)    { f_closedir(&dir); return SATR_EOT; }  /* Break on end of dir */
        else if (fno.fname[0] == '.') { continue; }             /* Ignore dot entry */

        fn = (uint8_t*)fno.fname; /*NOT _USE_LFN*/

        ret = strtol((char*)fn, NULL, 10);
        if(start_flag == 0 && *iter == ret) { start_flag = 1; }
        if(start_flag == 1) {

            if((*size + 3 + LOGS_LIST_SIZE) >= MAX_PKT_EXT_DATA) {

                /*Here we put the next filename for the iteration*/
                cnv16_8(ret, buf);
                *size += sizeof(uint16_t);

                f_closedir(&dir);
                return SATR_OK;
            }

            sprintf(temp_path,"%s/%s", path, (char*)fn);
            if((res = f_stat(temp_path, &fno)) != FR_OK) {
                fno.fsize = 0xFFFFFFFF;
                fno.fdate = 0xFFFF;
                fno.ftime = 0xFFFF;
            }

            cnv16_8(ret, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv32_8(fno.fsize, &buf[(*size)]);
            *size += sizeof(uint32_t);

            cnv16_8(fno.fdate, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv16_8(fno.ftime, &buf[(*size)]);
            *size += sizeof(uint16_t);
 
        } 

    }
    f_closedir(&dir);
 
    //if(!C_ASSERT(*size != 0) == true) { return SATR_ERROR; }
    if(i == MS_MAX_FILES - 1) { return SATR_MS_MAX_FILES; }

    return SATR_OK;
}


SAT_returnState mass_storage_schedule_load_api(MS_sid sid, uint32_t sch_number, uint8_t *buf) {

    FIL fp;
    FRESULT res;
    uint8_t path[MS_MAX_PATH];
    uint16_t size = 0;

    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT( sid == SCHS) == true) { return SATR_INV_STORE_ID; }
    if(!C_ASSERT( sch_number <= 14 ) == true) { return SATR_INV_STORE_ID; }
    
    snprintf((char*)path, MS_MAX_PATH, "%s//%d", MS_SCHS, sch_number);
    
        res = f_open(&fp, (char*)path, FA_OPEN_EXISTING | FA_READ); //!= FR_OK) { MS_ERR(res);}       
        res = f_read(&fp, buf, MS_MAX_SU_FILE_SIZE, (void *)&size);
        f_close(&fp);
   
    if(res != FR_OK)    { MS_ERR(res); }
    else if(size == 0)  { return SATR_ERROR; } 
    
    return SATR_OK;
}

SAT_returnState mass_storage_init() {

    FRESULT res = 0;

    MS_data.enabled = false;
    if((res = f_mount(&MS_data.Fs, MS_SD_PATH, 0)) != FR_OK) { MS_ERR(res); }

    MS_data.enabled = true;

    /*Directory sanity check*/
    if((res = mass_storage_dirCheck()) != SATR_OK)         { return res; }

    return SATR_OK;
}

SAT_returnState mass_storage_updatePkt(tc_tm_pkt *pkt, const uint16_t size, const uint8_t subtype) {

    pkt->ser_subtype = subtype;
    pkt->len = size;

    return SATR_OK;
}

SAT_returnState mass_storage_crtPkt(tc_tm_pkt **pkt, const uint16_t dest_id, const uint16_t size) {

    *pkt = get_pkt(size);
    if(!C_ASSERT(*pkt != NULL) == true) { return SATR_ERROR; }
    crt_pkt(*pkt, OBC_APP_ID, TM, TC_ACK_NO, TC_MASS_STORAGE_SERVICE, 0, dest_id); //what dest_id ?

    return SATR_OK;
}

SAT_returnState mass_storage_dirCheck() {

    FRESULT res;
    FILINFO fno;

    if((res = f_stat((char*)MS_WOD_LOG, &fno)) != FR_OK)        { MS_ERR(res); }
    if((res = f_stat((char*)MS_EXT_WOD_LOG, &fno)) != FR_OK)    { MS_ERR(res); }
    if((res = f_stat((char*)MS_EVENT_LOG, &fno)) != FR_OK)      { MS_ERR(res); }
    if((res = f_stat((char*)MS_FOTOS, &fno)) != FR_OK)          { MS_ERR(res); }
    if((res = f_stat((char*)MS_SCHS, &fno)) != FR_OK)           { MS_ERR(res); }

    return SATR_OK;
}

/**
 * @brief      gets mass storage device state
 *
 * @param      state  true/false
 */
void mass_storage_getState(uint8_t *state) {
    *state = MS_data.enabled;
}

SAT_returnState mass_storage_downlink_api(tc_tm_pkt *pkt, MS_sid sid, uint16_t file, uint8_t num) {

    uint16_t size;
    SAT_returnState res = SATR_ERROR;
    FRESULT fres;
    TC_TM_app_id app_id;
    tc_tm_pkt *temp_pkt = 0;

    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }

    app_id = (TC_TM_app_id)pkt->dest_id; //check if this is ok

    if(!C_ASSERT(sid < SRAM) == true) { return SATR_INV_STORE_ID; }
    if(!C_ASSERT(num > 0) == true)        { return SATR_ERROR; }
    if(!C_ASSERT((sid <= SU_SCRIPT_7 && num == 1) ||
                 (sid == SU_LOG && num < 10) ||
                 (sid == WOD_LOG && num < 10) ||
                 (sid == EXT_WOD_LOG && num < 10) ||
                 (sid == SCHS && num == 1) ||
                 (sid == EVENT_LOG && num < 4)) == true)    { return SATR_ERROR; }

    mass_storage_crtPkt(&temp_pkt, app_id, MAX_PKT_EXT_DATA);

    temp_pkt->data[0] = sid;

    size = 1;

    for(uint8_t i = 0; i < num; i++) {
        uint16_t fsize = 0;
        uint16_t tfile = file + i;

        res = mass_storage_downlinkFile(sid, tfile, &temp_pkt->data[size + 2], &fsize);

        if(res == SATRF_NO_FILE) { continue; }
        else if(res != SATR_OK) { free_pkt(temp_pkt); return res; }

        if(!C_ASSERT((sid <= SU_SCRIPT_7 && fsize < MAX_PKT_EXT_DATA) ||
                 (sid == SU_LOG && fsize == MS_SU_LOG_SIZE) ||
                 (sid == WOD_LOG && fsize == MS_WOD_LOG_SIZE) ||
                 (sid == EXT_WOD_LOG && fsize < MAX_PKT_EXT_DATA) ||
                 (sid == SCHS && fsize < MAX_PKT_EXT_DATA) ||
                 (sid == FOTOS && num < MAX_PKT_EXT_DATA)) == true) {

            free_pkt(temp_pkt);
            return SATR_WRONG_DOWNLINK_OFFSET;
        }

        cnv16_8(tfile, &temp_pkt->data[size]);
        size += fsize + 2;
        if(!C_ASSERT(size <= MAX_PKT_EXT_DATA) == true) { free_pkt(temp_pkt); return SATR_ERROR; }

    }
    mass_storage_updatePkt(temp_pkt, size, TM_MS_CONTENT);
    route_pkt(temp_pkt);

    return SATR_OK;
}

/*delete handles deletion of mass storage. sid denotes the store id.*/
/*if to is 0: it deletes every file of the sid else it deletes every file which time is lower then the time denoted in to*/
SAT_returnState mass_storage_delete_api(MS_sid sid, uint16_t to, MS_mode mode) {

    uint16_t *head = 0;
    uint16_t *tail = 0;

    if(!C_ASSERT(sid == WOD_LOG || \
                 sid == EXT_WOD_LOG || \
                 sid == EVENT_LOG || \
                 sid == FOTOS) == true) { return 0; }
    if(!C_ASSERT(to < MS_MAX_FILES) == true)  { return 0; }

    if(sid == WOD_LOG)          { head = obc_data.fs_wod_head; tail = obc_data.fs_wod_tail; }
    else if(sid == EXT_WOD_LOG) { head = obc_data.fs_ext_head; tail = obc_data.fs_ext_tail; }
    else if(sid == EVENT_LOG)   { head = obc_data.fs_ev_head;  tail = obc_data.fs_ev_tail; }

    if(mode == DELETE_ALL) {
        *head = 1;
        *tail = 1;
    } else {

        uint16_t files_num = *head - *tail;
        if(!C_ASSERT(to <= files_num) == true)  { return 0; }

        *tail += to;

        if(*tail > MS_MAX_FILES) { tail -= MS_MAX_FILES; }
    }
    return SATR_OK;
}

SAT_returnState mass_storage_report_api(tc_tm_pkt *pkt) {

    SAT_returnState res = SATR_ERROR;
    uint16_t size = 0;
    tc_tm_pkt *temp_pkt = 0;

    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(pkt != NULL && pkt->data != NULL) == true) { return SATR_ERROR; }

    TC_TM_app_id app_id = (TC_TM_app_id)pkt->dest_id; //check if this is ok

    mass_storage_crtPkt(&temp_pkt, app_id, PKT_NORMAL);

    if(!((res = get_fs_stat(temp_pkt->data, &size)) == SATR_EOT)) {
        free_pkt(temp_pkt);
        return res;
    }

    mass_storage_updatePkt(temp_pkt, size, TM_MS_CATALOGUE_REPORT);
    route_pkt(temp_pkt);

    return SATR_OK;
}

SAT_returnState get_fs_stat(uint8_t *buf, uint16_t *size) {

    FILINFO fno_head;
    FILINFO fno_tail;

    uint16_t head = 0;
    uint16_t tail = 0;

    uint8_t f_head[MS_MAX_PATH];
    uint8_t f_tail[MS_MAX_PATH];

    FILINFO fno;
    FRESULT res;
    uint8_t path[MS_MAX_PATH];

    if(!C_ASSERT(MS_data.enabled == true) == true) { return SATR_SD_DISABLED; }
    if(!C_ASSERT(buf != NULL && size != NULL) == true)  { return SATR_ERROR; }
    if(!C_ASSERT(*size == 0) == true)                   { return SATR_ERROR; }

    for(uint8_t sid = WOD_LOG; sid <= EVENT_LOG; sid++) {

        if(sid == WOD_LOG) {

            head = *obc_data.fs_wod_head;
            tail = *obc_data.fs_wod_tail + 1;
            if(tail > MS_MAX_FILES) {
                tail = 1;
            }
            snprintf((char*)f_head, MS_MAX_PATH, "%s//%d", MS_WOD_LOG, head); 
            snprintf((char*)f_tail, MS_MAX_PATH, "%s//%d", MS_WOD_LOG, tail);

        } else if(sid == EXT_WOD_LOG) {

            head = *obc_data.fs_ext_head;
            tail = *obc_data.fs_ext_tail + 1;
            if(tail > MS_MAX_FILES) {
                tail = 1;
            }

            snprintf((char*)f_head, MS_MAX_PATH, "%s//%d", MS_EXT_WOD_LOG, head); 
            snprintf((char*)f_tail, MS_MAX_PATH, "%s//%d", MS_EXT_WOD_LOG, tail);

        } else if(sid == EVENT_LOG) {

            head = *obc_data.fs_ev_head;
            tail = *obc_data.fs_ev_tail + 1;
            if(tail > MS_MAX_FILES) {
                tail = 1;
            }
            snprintf((char*)f_head, MS_MAX_PATH, "%s//%d", MS_EVENT_LOG, head); 
            snprintf((char*)f_tail, MS_MAX_PATH, "%s//%d", MS_EVENT_LOG, tail); 
        }
        else { return SATR_ERROR; }


        if(sid <= EVENT_LOG) {

            int16_t files_num = head - tail + 1;
            if(files_num < 0) { files_num = MS_MAX_FILES + files_num; }

            cnv16_8(files_num, &buf[(*size)]);
            *size += sizeof(uint16_t);

            res = f_stat(f_head, &fno_head);

            if(res != FR_OK) {
                fno_head.fsize = 0;
                fno_head.fdate = 0;
                fno_head.ftime = 0;
            }
            res = f_stat(f_tail, &fno_tail);

            if(res != FR_OK) {
                fno_tail.fsize = 0;
                fno_tail.fdate = 0;
                fno_tail.ftime = 0;
            }

            cnv16_8(tail, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv32_8(fno_tail.fsize, &buf[(*size)]);
            *size += sizeof(uint32_t);

            cnv16_8(fno_tail.fdate, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv16_8(fno_tail.ftime, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv16_8(head, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv32_8(fno_head.fsize, &buf[(*size)]);
            *size += sizeof(uint32_t);

            cnv16_8(fno_head.fdate, &buf[(*size)]);
            *size += sizeof(uint16_t);

            cnv16_8(fno_head.ftime, &buf[(*size)]);
            *size += sizeof(uint16_t);
        } else { return SATR_ERROR; }

    }
    return SATR_EOT;
}

uint16_t get_filePos(MS_sid sid, uint16_t rel_pos) {

    uint16_t *head = 0;
    uint16_t *tail = 0;

    if(!C_ASSERT(sid == WOD_LOG || \
                 sid == EXT_WOD_LOG || \
                 sid == EVENT_LOG || \
                 sid == FOTOS) == true) { return 0; }

    if(!C_ASSERT(rel_pos < MS_MAX_FILES) == true) { return 0; }

    if(sid == WOD_LOG)     { head = obc_data.fs_wod_head; tail = obc_data.fs_wod_tail; }
    else if(sid == EXT_WOD_LOG) { head = obc_data.fs_ext_head; tail = obc_data.fs_ext_tail; }
    else if(sid == EVENT_LOG)   { head = obc_data.fs_ev_head;  tail = obc_data.fs_ev_tail; }

    uint16_t abs_pos = 0;

    abs_pos = *tail + rel_pos;

    if(abs_pos > MS_MAX_FILES) { abs_pos -= MS_MAX_FILES; }

    return abs_pos;
}

uint16_t get_new_fileId(MS_sid sid) {

    uint16_t *head = 0;
    uint16_t *tail = 0;

    if(!C_ASSERT(sid == WOD_LOG ||
                 sid == EXT_WOD_LOG ||
                 sid == EVENT_LOG ||
                 sid == FOTOS) == true) { return 0; }

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
