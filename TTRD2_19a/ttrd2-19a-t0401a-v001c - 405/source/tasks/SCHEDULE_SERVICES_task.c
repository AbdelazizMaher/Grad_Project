
#include "SCHEDULE_SERVICES_task.h"
#include "scheduling_service.h"
#include "../tasks/ttrd2-05a-t0401a-v001a_uart2_buff_o_task.h"


static void RTC_init(void);

void     SCHEDULE_SERVICES_Init(void)
{
   RTC_init();
	 scheduling_service_init();
}

uint32_t SCHEDULE_SERVICES_Update(void)
{
	static uint32_t sch_tick;
	uint32_t now;
	
	now = getCurrentTick();
	if(now < sch_tick || now - sch_tick > 10000)//__TX_INTERVAL_MS
		{
			
			sch_tick = now;
      cross_schedules();

    }	
	 
	
	return RETURN_NORMAL_STATE;	
}

	
void RTC_init(void)
{
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
	 
	UART2_BUF_O_Write_String_To_Buffer("RTC INIT\n");
	UART2_BUF_O_Send_All_Data();	 
}