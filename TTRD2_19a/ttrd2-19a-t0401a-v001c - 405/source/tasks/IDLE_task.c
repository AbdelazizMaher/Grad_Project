
#include "IDLE_task.h"
#include "queue.h"

void IDLE_Init(void)
{

}

uint32_t IDLE_Update(void)
{
	 // pkt_pool_IDLE(time);
   queue_IDLE(EPS_APP_ID);
   queue_IDLE(COMMS_APP_ID);
   queue_IDLE(ADCS_APP_ID);
	
	return RETURN_NORMAL_STATE;
}	

