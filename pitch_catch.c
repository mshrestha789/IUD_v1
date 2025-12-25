#include "stm32f4_discovery.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "usart1.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)
#define DAC_DHR12R2_ADDRESS    ((uint32_t)0x40007414)
#define DAC_DHR8R1_ADDRESS     ((uint32_t)0x40007410)

static uint32_t g_adc_done_flag,g_dac_done_flag2;

/* Private function prototypes -----------------------------------------------*/
void ADC3_CH12_DMA_Config(__IO uint16_t *ADC3ConvertedValue, uint32_t adc_buff_len);
void TIM6_Config(void);
void DAC_Ch2_SineWaveConfig(const uint16_t *tone_burst, uint32_t tone_burst_len);
void dac_init(void);	
void pitch_catch(__IO uint16_t *ADC3ConvertedValue, uint32_t adc_buff_len, const uint16_t *tone_burst, uint32_t tone_burst_len);

	
	
void pitch_catch_init(void)
{
	
	dac_init();
	 /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	
	GPIO_InitTypeDef      GPIO_InitStructure;
	/* Configure ADC3 Channel12 pin as analog input ******************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	NVIC_InitTypeDef ADCdma_str;
	ADCdma_str.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	ADCdma_str.NVIC_IRQChannelPreemptionPriority = 0;
	ADCdma_str.NVIC_IRQChannelSubPriority = 0;
	ADCdma_str.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&ADCdma_str);
}

void pitch_catch(__IO uint16_t *ADC3ConvertedValue, uint32_t adc_buff_len, const uint16_t *tone_burst, uint32_t tone_burst_len)
{
	ADC3_CH12_DMA_Config(ADC3ConvertedValue, adc_buff_len);
	taskDISABLE_INTERRUPTS();
	taskENTER_CRITICAL();
	
	DAC_Ch2_SineWaveConfig(tone_burst, tone_burst_len);
	#if 0
	while(!g_dac_done_flag2)
	{
		vTaskDelay(0);
	}
	g_dac_done_flag2 = 0;
	#endif		
	
	#if 1
	ADC_SoftwareStartConv(ADC3);
	

	while(!g_adc_done_flag)
	{
		vTaskDelay(0);
			
	}
	g_adc_done_flag = 0;
	#endif
	taskEXIT_CRITICAL();
	taskENABLE_INTERRUPTS();
	vTaskDelay(100);
	//taskEXIT_CRITICAL();
	DMA_DeInit(DMA2_Stream0);			
	ADC_DMARequestAfterLastTransferCmd(ADC3, DISABLE);
	/* Enable ADC3 DMA */
	ADC_DMACmd(ADC3, DISABLE);
	/* Enable ADC3 */
	ADC_Cmd(ADC3, DISABLE);
	ADC_DeInit();
	vTaskDelay(100);	
}

void ADC3_CH12_DMA_Config(__IO uint16_t *ADC3ConvertedValue, uint32_t adc_buff_len)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
  
 
	/* DMA2 Stream0 channel0 configuration **************************************/
	DMA_InitStructure.DMA_Channel = DMA_Channel_2;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC3ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = adc_buff_len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0, ENABLE);

  

	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC3 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC3, &ADC_InitStructure);

	/* ADC3 regular channel12 configuration *************************************/
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

	/* Enable ADC3 DMA */
	ADC_DMACmd(ADC3, ENABLE);

	/* Enable ADC3 */
	ADC_Cmd(ADC3, ENABLE);
  
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
}


void DMA2_Stream0_IRQHandler(void)
{

    if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0) != RESET)
    {
        DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
        g_adc_done_flag = 1;
    }
}

void DMA1_Stream6_IRQHandler(void)
{

    if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
        g_dac_done_flag2 = 1;
    }
}
void dac_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* DMA1 clock and GPIOA clock enable (to be used with DAC) */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);

	/* DAC Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	/* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* TIM6 Configuration ------------------------------------------------------*/
	TIM6_Config(); 
	
	NVIC_InitTypeDef DACdma1_str, DACdma2_str;
	DACdma1_str.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	DACdma1_str.NVIC_IRQChannelPreemptionPriority = 0;
	DACdma1_str.NVIC_IRQChannelSubPriority = 0;
	DACdma1_str.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&DACdma1_str);
	DACdma2_str.NVIC_IRQChannel = DMA1_Stream6_IRQn;
	DACdma2_str.NVIC_IRQChannelPreemptionPriority = 0;
	DACdma2_str.NVIC_IRQChannelSubPriority = 0;
	DACdma2_str.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&DACdma2_str);
	
	
	DAC_DeInit();
	
}
/**
  * @brief  TIM6 Configuration
  * @note   TIM6 configuration is based on CPU @168MHz and APB1 @42MHz
  * @note   TIM6 Update event occurs each 37.5MHz/256 = 16.406 KHz    
  * @param  None
  * @retval None
  */
void TIM6_Config(void)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	/* TIM6 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0x02;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	/* TIM6 TRGO selection */
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

	/* TIM6 enable counter */
	TIM_Cmd(TIM6, ENABLE);
}

/**
  * @brief  DAC  Channel2 SineWave Configuration
  * @param  None
  * @retval None
  */
void DAC_Ch2_SineWaveConfig(const uint16_t *tone_burst, uint32_t tone_burst_len)
{
	DMA_InitTypeDef DMA_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;

	/* DAC channel2 Configuration */
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	
	/* DMA1_Stream5 channel7 configuration **************************************/
	DMA_DeInit(DMA1_Stream5);
	DMA_DeInit(DMA1_Stream6);

	DMA_InitStructure.DMA_Channel = DMA_Channel_7;  //DMA channel 7
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R2_ADDRESS; //Address from datasheet
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)tone_burst; // address of data
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = tone_burst_len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //circular for continous data
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);
	/* Enable DMA1_Stream5 */
	DMA_Cmd(DMA1_Stream5, ENABLE);
	/* Enable DAC Channel2 */
	DAC_Cmd(DAC_Channel_2, ENABLE);
	/* Enable DMA for DAC Channel2 */
	DAC_DMACmd(DAC_Channel_2, ENABLE);
	
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);
	/* Enable DMA1_Stream6 */
	DMA_Cmd(DMA1_Stream6, ENABLE);	
	
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
	
}

