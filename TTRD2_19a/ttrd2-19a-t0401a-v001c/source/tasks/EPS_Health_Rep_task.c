#include "EPS_Health_Rep_task.h"

#include "housekeeping_service.h"
#include "housekeeping.h"



void EPS_Health_Rep_task_Init(void)
{
   hk_INIT();	
}
uint32_t EPS_Health_Rep_task_Update(void)
{
	static uint32_t tick;
	uint32_t now;
	
	now = getCurrentTick();
	if(now < tick || now - tick > 10000)//__TX_INTERVAL_MS
		{
			
			tick = now;
      //EPS_Health_Req_SCH();

    }
	
	return RETURN_NORMAL_STATE;
}