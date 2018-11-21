#ifndef DCAM_DRV_H			/* prevent circular inclusions */
#define DCAM_DRV_H			/* by using protection macros */
#include <stdint.h>

void DCAM_DRV_Init(void *devh);

uint8_t SME_CMD_Config(uint16_t cmd,uint32_t value);

uint8_t SME_Video_En(uint32_t value);
uint8_t SME_IIC2SPI_En(uint32_t value);
uint8_t SME_Projection_Set(uint32_t value);
uint8_t SME_DispDepth_En(uint32_t value);
uint8_t SME_Depth_Sel(uint32_t value);
uint8_t SME_InPhase_Set(uint32_t value);

uint8_t FlashWPSet(uint8_t WP);
uint8_t FlashRead(uint32_t flashaddr, uint8_t *buf, uint16_t len);
uint8_t FlashWrite(uint32_t flashaddr, uint8_t *buf, uint16_t len);
uint8_t FlashErase(uint8_t mode, uint32_t flashaddr);

void WriteREFBin(uint32_t ref_baseaddr, const char * reffilename);
void depth_lut2flash(double reg_list[][3], double depth_ref, int mindis, uint32_t flashaddr);
double calc_n(double minidis, double depth_ref, double reg_list[][3]);

#endif