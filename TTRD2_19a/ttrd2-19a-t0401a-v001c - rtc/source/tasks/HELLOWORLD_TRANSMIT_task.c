// Module header
#include "HELLOWORLD_TRANSMIT_task.h"
#include <string.h>

#include "ttrd2-05a-t0401a-v001a_uart2_buff_o_task.h"

#include "stdio.h"

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
static void RCC_Config(void);
static void GPIO_Config(void);
static void USART_Config(void);
void print_time_date();
uint32_t return_time_QB50();
void cnv_UTC_QB50(struct time_utc utc, uint32_t *qb);
void HAL_sys_getTime(uint8_t *hours, uint8_t *mins, uint8_t *sec);
void HAL_sys_getDate(uint8_t *weekday, uint8_t *mon, uint8_t *date, uint8_t *year);

   char showtime[50];
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

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
	 
	 	memset(&sdatestructureget,0,sizeof(sdatestructureget));
    memset(&stimestructureget,0,sizeof(stimestructureget));
}

uint32_t HELLOWORLD_TRANSMIT_Update(void)
{	
	qb_time= return_time_QB50();
	print_time_date();
	
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
    //USART_SendData(USART2,);
	UART2_BUF_O_Write_String_To_Buffer(showtime);
	UART2_BUF_O_Send_All_Data();
    memset(showtime,0,sizeof(showtime));
    sprintf((char*)showtime,"%02d-%2d-%2d\r\n",sDate.RTC_Month, sDate.RTC_Date, 2000 + sDate.RTC_Year);
    //USART_SendData(USART2,);
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
