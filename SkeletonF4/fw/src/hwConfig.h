/**
  ******************************************************************************
  * @file           : hwConfig.h
  * @brief          : Code to configure the MCU devices (hardware)
  ******************************************************************************
  */
#ifndef __HWCONFIG_H__
#define __HWCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_conf.h"
#include <usbd_core.h>
#include <usbd_cdc.h>
#include "usbd_cdc_if.h"
#include <usbd_desc.h>


extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void InitUsb(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_NVIC_Init(void);
void MX_SPI2_Init(void);
void MX_TIM2_Init(void);
void MX_TIM4_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
	
#ifdef __cplusplus
}
#endif

#endif // __HWCONFIG_H__