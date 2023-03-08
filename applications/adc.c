/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-01     Rick       the first version
 */
#include "adc.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "status.h"
#include "key.h"
#include "moto.h"
#include "flashwork.h"
#include "gateway.h"
#include "board.h"

#define DBG_TAG "adc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

extern uint8_t ValveStatus;

ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

rt_thread_t ntc_work = RT_NULL;

uint8_t NTC_State = 0;
uint32_t adc_value[20];

static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_39CYCLES_5;
  hadc.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_39CYCLES_5;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */
  HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
  HAL_NVIC_DisableIRQ(ADC_IRQn);
  /* USER CODE END ADC_Init 2 */

}

static void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);

    /* DMA1_Channel2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);

}
void ADC1_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_IRQn 0 */

  /* USER CODE END ADC1_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc);
  /* USER CODE BEGIN ADC1_IRQn 1 */

  /* USER CODE END ADC1_IRQn 1 */
}
double ADC_Voltage_Calc(void)
{
    uint32_t voltage_temp = 0;
    double real_voltage = 0;
    for(uint8_t i=0;i<20;i++)
    {
        voltage_temp += adc_value[i];
    }
    real_voltage = voltage_temp * 0.000040283203125 + 0.018;
    return real_voltage;
}
void NTC_State_Save(uint8_t result)
{
    NTC_State = result;
}
uint8_t NTC_State_read(void)
{
    return NTC_State;
}
void NTC_Work_Callback(void *parameter)
{
    LOG_D("NTC With ADC is Init Success\r\n");
    while(1)
    {
        ADC_Voltage_Calc();
        if(ADC_Voltage_Calc()<1.153 && GetNowStatus()!=NTCWarning)
        {
            NTC_State_Save(ValveStatus);
            Warning_Enable_Num(8);
        }
        if(ADC_Voltage_Calc()>=1.168 && GetNowStatus()==NTCWarning)
        {
            WarUpload_GW(1,0,8,0);//NTC报警
            Warning_Disable();
            if(NTC_State_read())
            {
                Moto_Open(NormalOpen);
            }
            else
            {
                Moto_Close(NormalOff);
            }
        }
        rt_thread_mdelay(1000);
    }
}

void ADC_Init(void)
{
    MX_DMA_Init();
    MX_ADC_Init();
    HAL_ADC_Start_DMA(&hadc,(uint32_t*) &adc_value, 20);
    ntc_work = rt_thread_create("ntc_work", NTC_Work_Callback, RT_NULL, 2048, 15, 10);
    if(ntc_work != RT_NULL)rt_thread_startup(ntc_work);
}
