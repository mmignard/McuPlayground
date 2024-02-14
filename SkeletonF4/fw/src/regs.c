/**
  ******************************************************************************
  * @file           : regs.c
  * @brief          : Code to handle registers that do not survive power cycle
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "regs.h"
#include "spi.h"

volatile REG_BLOCK Regs;
extern uint16_t stepCount;
extern uint16_t FirmWareVersion;
extern SPI_HandleTypeDef hspi2;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

uint16_t ReadAdc(uint16_t chan);
void SetSClk(uint16_t shftNum, uint16_t val);
void SetRClk(uint16_t shftNum, uint16_t val);
void SetDat(uint16_t shftNum, uint16_t val);
void PwrEnShift(void);
void TempSelShift(void);

void InitRegs() {
	//Power-on intitializations
	Regs.u16[RegLoadSwCtrl0] = 0;
	Regs.u16[RegLoadSwCtrl1] = 0;
	Regs.u16[RegLoadSwCtrl2] = 0;
	Regs.u16[RegLoadSwCtrl3] = 0;
	Regs.u16[RegLoadSwCtrl4] = 0;
	PwrEnShift();
	Regs.u16[RegLoadTest] = 0;
	Regs.u16[RegTempCtrl] = 0;
	TempSelShift();	
	InitRtdAdc();
	//HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
//	hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
//	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	if (HAL_SPI_Init(&hspi2) != HAL_OK)
		Error_Handler();

}	

//UpdateRegs called periodically in main()
uint8_t UpdateRegs() {
    uint8_t err = 0;
    static uint16_t nReg=0;
    
    err = ReadReg(nReg);
    nReg++;
    if (nReg>=RegLast || nReg>=REG_SIZE16) nReg = 0;
    
    return err;
}

uint8_t ReadReg(uint16_t nReg) {
	//This routine called if a read register command received over USB, or periodically to update volatile registers.
	//In most cases registers are always valid, but for some the value needs to be updated.
    uint16_t temp = 0, limSw = 0;
    uint8_t err = 0;
    
    if (nReg>=RegLast || nReg>=REG_SIZE16) return 0; //not an error == just don't read past the end of the register block
    
    switch (nReg) {
        case RegFirmWareVersion:
	        Regs.u16[nReg] = FirmWareVersion;
            break;
		case RegUniqueID:
			Regs.u16[nReg] = *(uint16_t *)(0x1fff7a10UL);
			break;
		case RegTick:
			Regs.u16[nReg] = (uint16_t)(HAL_GetTick() & 0xFFFF);
	        break;
		case RegAdcTemp:
			Regs.u16[nReg] = ReadAdc(16); //temperature sensor is channel 16, 0.76V @25degC + 2.5mV/degC, temp = 25+(nAdc*3.3/4095-0.76)/0.025, 1022 => 27.5degC
			break;
		case RegAdcRef:
			Regs.u16[nReg] = ReadAdc(17); //internal reference voltage (1.2Vnom), should be around 1.2/3.3*4095 = 1500, can use to calculate voltage of 3.3V supply
			break;
		case RegRtdTemp:
			Regs.u16[nReg] = ReadRtdTemp(); //rRtd = nAdc*400/65536 in ohms, temp = (nAdc/16384-1)/3.9038e-3
			break;
		case RegRtdCfg: //read config register for Pt Rtd temperature measurement
			Regs.u16[nReg] = (uint16_t)ReadSpi8(CSN3, 0); //need to modify because switching CPHA
			break;
		case RegRstAdcDat: //Data register for AD7298 that reads resets and LED
			Regs.u16[nReg] = ReadAdc7298(CSN0, Regs.u16[RegRstAdcChan]);
			break;
		case RegSnsAdcDat: //Data register for AD7298 that senses power supply voltage & motor current
			Regs.u16[nReg] = ReadAdc7298(CSN1, Regs.u16[RegSnsAdcChan]);
			break;
		case RegTempPot: //read dummy potentiometer for PDU temperature measurement
			Regs.u16[nReg] = ReadTempPot(); //this read is not working
			break;
		default:
			break;
    }  
    return err;
}
void SetReg(uint16_t nReg, uint16_t value) 
{   
    int16_t sval;
	uint16_t dummyVal;
	//This routine called if a write register command received over USB
    if (nReg>=RegLast || nReg>=REG_SIZE16) return;
    
    switch (nReg) {
		case RegRtdCfg:  //write config register for Pt Rtd temperature measurement
			Regs.u16[nReg] = value;
			WriteSpi16(CSN3, 0, Regs.u16[nReg]);
			break; // 
		case RegRstAdcChan: //channel to read on AD7298 that reads resets and LED
			Regs.u16[nReg] = value;
			break;
		case RegSnsAdcChan: //channel to read on AD7298 that senses power supply voltages & motor current
			Regs.u16[nReg] = value;
			break;
		case RegTempCtrl:  //write shift register to control dummy temperature sensors
			Regs.u16[nReg] = value;
			TempSelShift();
			break;
	    case RegLoadSwCtrl0: 
	    case RegLoadSwCtrl1: 
	    case RegLoadSwCtrl2: 
	    case RegLoadSwCtrl3: 
		    Regs.u16[nReg] = value;
		    break;
	    case RegLoadSwCtrl4: //write shift register to control power switches, only update switches on write to Ctrl4
		    Regs.u16[nReg] = value;
		    PwrEnShift();
		    break;
	    case RegLoadTest: //turn on one of the local dummy loads
		    Regs.u16[nReg] = value;
		    dummyVal = (0 == (value & 1)) ? 0 : 1;
		    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, dummyVal);
		    dummyVal = (0 == (value & 2)) ? 0 : 1;
		    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, dummyVal);
		    break;
		case RegExtraBits: //bit defs: 0=BKEN, 1=MPPR, 2=PPS0, 3=PPS1, 4=INHLOC
			Regs.u16[nReg] = value;
			dummyVal = (0 == (value & 1)) ? 0 : 1;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
			dummyVal = (0 == (value & 2)) ? 0 : 1;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
			dummyVal = (0 == (value & 4)) ? 0 : 1;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
			dummyVal = (0 == (value & 8)) ? 0 : 1;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
			dummyVal = (0 == (value & 16)) ? 0 : 1;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
			break;
		case RegTempPot : //write digital temperature potentiometer for dummy thermistor
		    Regs.u16[nReg] = value;
			SetTempPot(value);
			break;
		default :
			break;
    }
    
    ReadReg(nReg);
}

//Currently only the temperature sensor and the internal voltge regulator are accessed through the MCU ADC peripheral
uint16_t ReadAdc(uint16_t chan)
{
	ADC_ChannelConfTypeDef sConfig = { 0 };
	if (16 == chan)
		sConfig.Channel = ADC_CHANNEL_16;
	else if (17 == chan)
		sConfig.Channel = ADC_CHANNEL_17;
	else
		sConfig.Channel = chan;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);	
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	uint16_t retVal = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return (retVal);
}

void SetSClk(uint16_t shftNum, uint16_t val)
{
	uint16_t pin = (0 == shftNum) ? GPIO_PIN_11 : GPIO_PIN_8;
	HAL_GPIO_WritePin(GPIOC, pin, val);
}

void SetRClk(uint16_t shftNum, uint16_t val)
{
	uint16_t pin = (0 == shftNum) ? GPIO_PIN_10 : GPIO_PIN_6;
	HAL_GPIO_WritePin(GPIOC, pin, val);
}

void SetDat(uint16_t shftNum, uint16_t val)
{
	uint16_t pin = (0 == shftNum) ? GPIO_PIN_12 : GPIO_PIN_9;
	uint16_t pinState = (0 == (val & 0x8000)) ? 0 : 1;
	HAL_GPIO_WritePin(GPIOC, pin, pinState);
}

//this function uses global registers as its input
void TempSelShift(void)
{
	uint16_t dat, j;
	//shift 16 bits of data into the two shift registers
	dat = Regs.u16[RegTempCtrl];
	for (j = 0; j < 16; ++j)
	{
		SetSClk(1, 0);
		SetDat(1, dat);
		dat <<= 1;
		SetSClk(1, 1);
	}
	//latch the data
	SetRClk(1, 0);
	SetRClk(1, 1);
}

//this function uses global registers as its input
void PwrEnShift(void)
{
	uint16_t dat, i, j;
	for (i = 0; i < 4; ++i) //shift data into the first 8 shift registers, 16 bits at a time
	{
		dat = Regs.u16[RegLoadSwCtrl0 + i];
		for (j = 0; j < 16; ++j)
		{
			SetSClk(0, 0);
			SetDat(0, dat);
			dat <<= 1;
			SetSClk(0, 1);
		}
	}
	//shift 8 bits of data into the last shift register
	dat = Regs.u16[RegLoadSwCtrl4];
	for (j = 0; j < 8; ++j)
	{
		SetSClk(0, 0);
		SetDat(0, dat);
		dat <<= 1;
		SetSClk(0, 1);
	}
	SetRClk(0, 0);
	SetRClk(0, 1);
}



