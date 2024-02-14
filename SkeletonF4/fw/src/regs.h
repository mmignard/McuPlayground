/**
  ******************************************************************************
  * @file           : regs.h
  * @brief          : Code to handle registers that do not survive power cycle
  ******************************************************************************
  */
#ifndef __REGS_H__
#define __REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define REG_SIZE 512
#define REG_SIZE8      (REG_SIZE-4)
#define REG_SIZE16     (REG_SIZE8/2)
#define REG_SIZE32     (REG_SIZE16/2)

typedef struct REG_BLOCK {
	 // 512 bytes - 4 per page
    uint8_t start;
	uint8_t seq;
	uint8_t ver;
	uint8_t chksum;
	union {
		uint8_t u8[REG_SIZE8];
		uint16_t u16[REG_SIZE16];
		int16_t s16[REG_SIZE16];
		uint32_t u32[REG_SIZE32];
		int32_t s32[REG_SIZE32];
		float f32[REG_SIZE32];
	};
} REG_BLOCK;

extern volatile REG_BLOCK Regs;
	
enum SYS_STATUS {
  RegFirmWareVersion = 0,      //0, Version # set at top of main.c
  RegUniqueID,                 //1, a unique ID for each MCU (96 bits, but only returns lowest 16 bits)
  RegTick,                     //2, lower 16 bits of the 1mS system timer
  RegAdcTemp,                  //3, read temperature sensor internal to MCU, 0.76V @25degC + 2.5mV/degC, temp = 25+(nAdc*3.3/4095-0.76)/0.025, 1022 => 27.5degC
  RegAdcRef,                   //4, MCU internal reference voltage (1.2Vnom), should be around 1.2/3.3*4095 = 1500, can use to calculate voltage of 3.3V supply
  RegRtdTemp,                  //5, read Rtd temperature sensor (U17), rRtd = nAdc*400/65536 in ohms, temp = (rRtd/16384-1)/3.9038e-3
  RegRtdCfg, 				   //6, Rtd Config register (8 bits)
  RegRstAdcChan, 			   //7, channel to read on AD7298 that reads resets and LED
  RegRstAdcDat, 			   //8, Data register for AD7298 that reads resets and LED
  RegSnsAdcChan, 			   //9, channel to read on AD7298 that senses power supply voltages & motor current
  RegSnsAdcDat, 			   //10, Data register for AD7298 that senses power supply voltage
  RegTempCtrl, 			       //11, Temperature control register, needs to get converted to SHIFT/LATCH/QIN for 74HC595 shift register--there are 2 of them in series
  RegLoadSwCtrl0, 			   //12, Load switch control register, needs to get converted to SHIFT/LATCH/QIN for 74HC595 shift register--there are 9 of them in series
  RegLoadSwCtrl1, 			   //13, Load switch control register
  RegLoadSwCtrl2, 			   //14, Load switch control register
  RegLoadSwCtrl3, 			   //15, Load switch control register
  RegLoadSwCtrl4, 			   //16, Load switch control register, update all shift registers only when writing this register
  RegLoadTest, 			       //17, bit 0=LDTST0 (255ohm load), bit 1=LDTST1 (127ohm load), bit 2=LDSNS0, bit 3=LDSNS1 (LDSNSx are read only)
  RegLoadTestTime, 			   //18, number of microseconds from either LDTST0 or LDTST1 going high until source was disabled (measures OCP reaction time), 0 means still waiting
  RegExtraBits, 			   //19, bit defs: 0=BKEN, 1=MPPR, 2=PPS0, 3=PPS1, 4=INHLOC
  RegMotorPos, 				   //20, motor position (from quadrature rotary position encoder), can write to any initial value
  RegTempPot, 				   //21, digital temperature potentiometer for dummy thermistor

//control bits needed: BKEN (brake enable), PPS0,1 (pulse per second for PDU timing), MPPR (dummy pulse per revolution for PDU FPGA)
//LDTST0,1 (load test, switches in some dummy loads on the PduTest board)
  RegLast
};

void InitRegs();
uint8_t UpdateRegs();
uint8_t ReadReg(uint16_t nReg);
void SetReg(uint16_t nReg, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif // __REGS_H__