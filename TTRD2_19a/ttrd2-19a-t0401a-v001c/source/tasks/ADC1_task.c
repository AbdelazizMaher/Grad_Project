#include "ADC1_task.h"

#include "../tasks/ttrd2-05a-t0401a-v001a_adc_task.h"
#include <string.h>


//#include "obc.h"


void ADC1_vbat_Init(void)
{
   ADC_CommonInitTypeDef ADC_CommonInitStructure;
 
   // Apply reset values
   ADC_CommonStructInit(&ADC_CommonInitStructure);
 
   // Enable ADC clock 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
 
   // Set up ADC in independent mode
   ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
   ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
   ADC_CommonInit(&ADC_CommonInitStructure);
 
   
   // Enable ADC1 
   ADC_Cmd(ADC1, ENABLE); 
	 
	 // Enable VRBAT
	 ADC_VBATCmd(ENABLE) ;

   TIMEOUT_T3_USEC_Init();	 
}

uint32_t ADC1_vbat_Update(void)
{
   ADC1_vbat_Init();
	
    obc_data.vbat = (uint16_t) ADC1_Read_Channel(ADC_Channel_Vbat, ADC_SampleTime_3Cycles);
	  ADC_VBATCmd(DISABLE);
 
	return RETURN_NORMAL_STATE;
}
 