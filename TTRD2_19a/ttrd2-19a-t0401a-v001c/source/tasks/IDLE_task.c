
#include "IDLE_task.h"
#include "queue.h"
#include "mxconstants.h"

static void GPIO_Inits();

void IDLE_Init(void)
{
	//GPIO_Inits();
}

uint32_t IDLE_Update(void)
{
	 // pkt_pool_IDLE(time);
   queue_IDLE(EPS_APP_ID);
   queue_IDLE(COMMS_APP_ID);
   queue_IDLE(ADCS_APP_ID);
	
	return RETURN_NORMAL_STATE;
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