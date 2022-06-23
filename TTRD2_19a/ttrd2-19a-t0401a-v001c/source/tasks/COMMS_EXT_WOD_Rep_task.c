#include "COMMS_EXT_WOD_Rep_task.h"

#include "housekeeping_service.h"
#include "housekeeping.h"



void COMMS_EXT_WOD_Rep_task_Init(void)
{
	
}
uint32_t COMMS_EXT_WOD_Rep_task_Update(void)
{
  static uint32_t tick;
	uint32_t now;
	
	now = getCurrentTick();
	if(now < tick || now - tick > 60000)//__TX_INTERVAL_MS
		{
			
			tick = now;
      COMMS_EXT_WOD_Rep_SCH();
    }			
	
	
	return RETURN_NORMAL_STATE;
}


