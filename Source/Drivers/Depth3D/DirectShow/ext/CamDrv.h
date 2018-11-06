#ifndef CAM_DRV_H			/* prevent circular inclusions */
#define CAM_DRV_H			/* by using protection macros */

#include "sensor.h"
#include "SVCeuctl.h"
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

void camdrv_init(SVCeuctl * s_EUctl);
uint8_t I2C_ReadB(uint8_t DevID, uint8_t addr, uint8_t* data);
uint8_t I2C_Read21B(uint8_t DevID, uint16_t addr, uint8_t* data);
uint8_t I2C_Read2B(uint8_t DevID, uint16_t addr, uint16_t* data);
uint8_t I2C_Read4B(uint8_t DevID, uint16_t addr, uint32_t* data);
uint8_t I2C_WriteB(uint8_t DevID, uint8_t addr, uint8_t data);
uint8_t I2C_Write21B(uint8_t DevID, uint16_t addr, uint8_t data);
uint8_t I2C_Write2B(uint8_t DevID, uint16_t addr, uint16_t data);
uint8_t I2C_Write4B(uint8_t DevID, uint16_t addr, uint32_t data);

uint8_t SensorAR0144Set(uint16_t addr, uint16_t data);
uint8_t SensorAR0144Get(uint16_t addr, uint16_t* data);
uint8_t Sensor9712Set(uint8_t addr, uint8_t data);
uint8_t Sensor9712Get(uint8_t addr, uint8_t* data);

uint8_t SVDPUSet(uint16_t addr, uint32_t data);
uint8_t SVDPUGet(uint16_t addr, uint32_t *data);
uint8_t SVDPUGetThenSet(uint16_t reg, uint32_t  value, uint32_t  mask);
uint8_t SVDPUSet8b(uint16_t addr, uint8_t data);
uint8_t SVDPUGet8b(uint16_t addr, uint8_t *data);
uint8_t SVDPUGetThenSet8b(uint16_t reg, uint8_t  value, uint8_t  mask);

void platformInit(void);
void platformPowerUp(void);
void SensorAR0144SetSeq(uint16_t reg_list[][3]);
void Sensor9712SetSeq(uint16_t reg_list[][3]);
void Sensor9712GetSeq(uint16_t reg_list[][3]);
void SensorInit(void);

void StreamSwitchToDepth(void);
void StreamSwitchToIR(void);
#endif