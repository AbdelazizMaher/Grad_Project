#include "COMMS_exHealth_Rep_task.h"

#include "housekeeping_service.h"
#include "housekeeping.h"


void     COMMS_exHealth_Rep_task_Init(void)
{
	
}
uint32_t COMMS_exHealth_Rep_task_Update(void)
{
  static uint32_t tick;
	uint32_t now;
	
	now = getCurrentTick();
	if(now < tick || now - tick > 93000)//__TX_INTERVAL_MS
		{
			
			tick = now;
      COMMS_exHealth_Req_SCH();
    }		
  
	
	return RETURN_NORMAL_STATE;
}