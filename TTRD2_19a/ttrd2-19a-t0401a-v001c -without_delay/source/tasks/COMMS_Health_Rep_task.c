#include "COMMS_Health_Rep_task.h"

#include "housekeeping_service.h"
#include "housekeeping.h"



void COMMS_Health_Rep_task_Init(void)
{

}
uint32_t COMMS_Health_Rep_task_Update(void)
{
  COMMS_Health_Req_SCH();
	
	return RETURN_NORMAL_STATE;
}


