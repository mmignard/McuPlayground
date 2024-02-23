/**
  ******************************************************************************
  * @file           : hwConfig.h
  * @brief          : Code to configure the MCU devices (hardware)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "hwConfig.h"

ADC_HandleTypeDef hadc1;
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void InitUsb(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_NVIC_Init(void);
void MX_SPI2_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

uint16_t stepCount = 0;
//void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{
	//PA4 -> CFAULT input from CAN XCVR
	//PA5 -> BKEN (high routes PDU brake power to connector J8)
	//PA6 -> VRPWM, PWM output to fake the radio power
	//PA7,8 -> PPS0 & 1	
	//PB6,7 -> LDSNS0 & 1 sense current state of local dummy loads (comes from comparator output)
	//PB12,13 -> LDTST0 & 1 turn on one of two local dummy loads
	//PB14 -> green LED (low=on)
	//PC0 -> CSN0 for reset & LED ADC SPI bus
	//PC1 -> CSN1 for power supply and motor current ADC SPI bus
	//PC4 -> CSN2 digital pot SPI for dummy thermistor
	//PC5 -> CSN3 Platinum RTD ADC SPI to measure temperature
	//PC6,8,9 -> LATCH1, SHIFT1, QIN1 for shift register for temperature mux control
	//PC10,11,12 -> LATCH0,SHIFT0,QIN0 for shift register for power switch control
	//PC13 -> INHLOC local inhibit that goes to PDU
	//PC14 -> motor pulse per revolution (PDU senses)
	//PC15 -> CAN bus silent control
	
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_RESET);
//	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOC,
		GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
		GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
		GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
		GPIO_PIN_SET);
	GPIO_InitStruct.Pin =	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
							GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
							GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	HAL_StatusTypeDef temp = HAL_OK;
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 144;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 6;
	temp = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(temp != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
* Initializes the Global MSP.
*/
void HAL_MspInit(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
void MX_NVIC_Init(void)
{
	/* OTG_FS_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC1_Init(void)
{
	ADC_ChannelConfTypeDef sConfig = { 0 };
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	*/
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (hadc->Instance == ADC1)
	{
		/* Peripheral clock enable */
		__HAL_RCC_ADC1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**ADC1 GPIO Configuration
		PA0-WKUP     ------> ADC1_IN0
		PA1     ------> ADC1_IN1
		PA2     ------> ADC1_IN2
		PA3     ------> ADC1_IN3
		PA4     ------> ADC1_IN4
		PA5     ------> ADC1_IN5
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
		                      | GPIO_PIN_4 | GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1)
	{
		/* Peripheral clock disable */
		__HAL_RCC_ADC1_CLK_DISABLE();

		/**ADC1 GPIO Configuration
		PA0-WKUP     ------> ADC1_IN0
		PA1     ------> ADC1_IN1
		PA2     ------> ADC1_IN2
		PA3     ------> ADC1_IN3
		PA4     ------> ADC1_IN4
		PA5     ------> ADC1_IN5
		*/
		HAL_GPIO_DeInit(GPIOA,
			GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
		                      |GPIO_PIN_4|GPIO_PIN_5);
	}
}
/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
void MX_SPI2_Init(void)
{
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi2) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (hspi->Instance == SPI2)
	{
		/* Peripheral clock enable */
		__HAL_RCC_SPI2_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		/**SPI2 GPIO Configuration
		PC2     ------> SPI2_MISO
		PC3     ------> SPI2_MOSI
		PC7     ------> SPI2_SCK
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	}
}

/**
* @brief SPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
	if (hspi->Instance == SPI2)
	{
		/* Peripheral clock disable */
		__HAL_RCC_SPI2_CLK_DISABLE();

		/**SPI2 GPIO Configuration
		PC2     ------> SPI2_MISO
		PC3     ------> SPI2_MOSI
		PC7     ------> SPI2_SCK
		*/
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_7);
	}
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM2_Init(void)
{
	//TIM2 is used to decode an incremental encoder to detect antenna deployment motor motion
	TIM_Encoder_InitTypeDef sConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 327;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 511;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 0;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 0;
	if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
	{
		asm("bkpt 255");
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		asm("bkpt 255");
	}
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM4_Init(void)
{
	//TIM4 is used to detect the amount of time from enabling a dummy load until the OCP circuit trips and turns off the power source
	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_IC_InitTypeDef sConfigIC = { 0 };

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 327;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 511;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
	{
		asm("bkpt 255");
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
	{
		asm("bkpt 255");
	}
	if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
	{
		asm("bkpt 255");
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
	{
		asm("bkpt 255");
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
	{
		asm("bkpt 255");
	}
	if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
	{
		asm("bkpt 255");
	}

}

/**
* @brief TIM_Encoder MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_encoder: TIM_Encoder handle pointer
* @retval None
*/
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (htim_encoder->Instance == TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**TIM2 GPIO Configuration
		PA0-WKUP     ------> TIM2_CH1
		PA1     ------> TIM2_CH2
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* TIM2 interrupt Init */
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
		//TIM2->DIER = 0x0003; //enable update interrupt-not sure why this is here, it should be enabled by TIM1_UP_TIM10_IRQn above
		                       //maybe have to use HAL_TIM_Base_Start_IT() in MX_TIM1_Init()
			}
}

/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (htim_base->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**TIM4 GPIO Configuration
		PB6     ------> TIM4_CH1
		PB7     ------> TIM4_CH2
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//		HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
//		HAL_NVIC_EnableIRQ(TIM4_IRQn);
//		TIM4->DIER = 0x0003; //enable update interrupt-not sure why this is here, it should be enabled by TIM1_UP_TIM10_IRQn above
                             //maybe have to use HAL_TIM_Base_Start_IT() in MX_TIM1_Init()
	}
}

/**
* @brief TIM_Encoder MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_encoder: TIM_Encoder handle pointer
* @retval None
*/
void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_encoder)
{
	if (htim_encoder->Instance == TIM2)
	{
		__HAL_RCC_TIM2_CLK_DISABLE();

		/**TIM2 GPIO Configuration
		PA0-WKUP     ------> TIM2_CH1
		PA1     ------> TIM2_CH2
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
	}
}

/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if (htim_base->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_DISABLE();

		/**TIM4 GPIO Configuration
		PB6     ------> TIM4_CH1
		PB7     ------> TIM4_CH2
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	__asm volatile("bkpt #01");
	while (1)
	{
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	 /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
