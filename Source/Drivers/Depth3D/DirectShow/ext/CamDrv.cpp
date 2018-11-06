// UVCExtensionApp.cpp : Defines the entry point for the console application.
//
//#include "stdafx.h"
#include "CamDrv.h"
#include <stdio.h>
#include <windows.h>

SVCeuctl * m_EUctl = NULL;

void camdrv_init(SVCeuctl * s_EUctl)
{
	m_EUctl = s_EUctl;
}

uint8_t I2C_ReadB(uint8_t DevID, uint8_t addr, uint8_t* data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x000000ff);
	uint8_t status = 0x00;

	status = m_EUctl->XferRecv(DevID, 1, subaddr, (uint8_t*)data, 1);

	if (status != 0x00)
		DEGPRINT(status, "I2C_ReadB()");

	return status;
}

uint8_t I2C_Read21B(uint8_t DevID, uint16_t addr, uint8_t* data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x0000ffff);
	uint8_t status = 0x00;

	status = m_EUctl->XferRecv(DevID, 2, subaddr, (uint8_t*)data, 1);

	if (status != 0x00)
		DEGPRINT(status, "I2C_Read2B()");

	return status;
}

uint8_t I2C_Read2B(uint8_t DevID, uint16_t addr, uint16_t* data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x0000ffff);
	uint8_t status = 0x00;

	status = m_EUctl->XferRecv(DevID, 2, subaddr, (uint8_t*)data,2);

	if (status != 0x00)
		DEGPRINT(status, "I2C_Read2B()");

	return status;
}

uint8_t I2C_Read4B(uint8_t DevID, uint16_t addr, uint32_t* data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x0000ffff);
	uint8_t status = 0x00;

	status = m_EUctl->XferRecv(DevID, 2, subaddr, (uint8_t*)data, 4);

	if (status != 0x00)
		DEGPRINT(status, "I2C_Read4B()");

	return status;
}

uint8_t I2C_WriteB(uint8_t DevID, uint8_t addr, uint8_t data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x000000ff);
	uint8_t status = 0x00;

	status = m_EUctl->XferSend(DevID, 1, subaddr, (uint8_t*)&data, 1);

	if (status != 0x00)
		DEGPRINT(status, "I2C_WriteB()");

	return status;
}

uint8_t I2C_Write21B(uint8_t DevID, uint16_t addr, uint8_t data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x0000ffff);
	uint8_t status = 0x00;

	status = m_EUctl->XferSend(DevID, 2, subaddr, (uint8_t*)&data, 1);

	if (status != 0x00)
		DEGPRINT(status, "I2C_Write2B()");

	return status;
}

uint8_t I2C_Write2B(uint8_t DevID, uint16_t addr, uint16_t data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}

	uint32_t subaddr = (uint32_t)(addr & 0x0000ffff);
	uint8_t status = 0x00;

	status = m_EUctl->XferSend(DevID, 2, subaddr, (uint8_t*)&data, 2);

	if (status != 0x00)
		DEGPRINT(status, "I2C_Write2B()");

	return status;
}

uint8_t I2C_Write4B(uint8_t DevID, uint16_t addr, uint32_t data)
{
	if (m_EUctl == NULL)
	{
		//printf("camdrv is not init \r\n");
		return 0x01;
	}
	uint32_t subaddr = (uint32_t)(addr & 0x0000ffff);
	uint8_t status = 0x00;

	status = m_EUctl->XferSend(DevID, 2, subaddr, (uint8_t*)&data, 4);

	if (status != 0x00)
		DEGPRINT(status, "I2C_Write4B()");

	return status;
}

uint8_t SVDPUSet8b(uint16_t addr, uint8_t data)
{
	return I2C_Write21B(0xac, addr, data);
}

uint8_t SVDPUGet8b(uint16_t addr, uint8_t *data)
{
	return I2C_Read21B(0xac, addr, data);
}

uint8_t SVDPUGetThenSet8b(uint16_t reg, uint8_t  value, uint8_t  mask)
{
	uint8_t t;
	uint8_t Status;
	Status = SVDPUGet8b(reg, &t);

	if (Status != 0x00)
	{
		return Status;
	}

	t &= ~(mask);
	t |= (value)& (mask);

	Status = SVDPUSet8b(reg, t);

	if (Status != 0x00)
	{
		return Status;
	}

	return Status;
}

uint8_t SVDPUSet(uint16_t addr, uint32_t data)
{
	return I2C_Write4B(0xac, addr, data);
}

uint8_t SVDPUGet(uint16_t addr, uint32_t *data)
{
	return I2C_Read4B(0xac, addr, data);
}

uint8_t SVDPUGetThenSet(uint16_t reg, uint32_t  value, uint32_t  mask)
{
	uint32_t t;
	uint8_t Status;
	Status = SVDPUGet(reg, &t);

	if (Status != 0x00)
	{
		return Status;
	}

	t &= ~(mask);
	t |= (value)& (mask);

	Status = SVDPUSet(reg, t);

	if (Status != 0x00)
	{
		return Status;
	}

	return Status;
}

uint8_t Sensor9712Set(uint8_t addr, uint8_t data)
{
	return I2C_WriteB(0x60, addr, data);
}

uint8_t Sensor9712Get(uint8_t addr, uint8_t* data)
{
	return I2C_ReadB(0x60, addr, data);
}

void LBChange(int len, uint8_t * src, uint8_t * dst)
{
	if (len == 1)
	{
		*dst = *src;
	}
	else if (len >= 2)
	{
		dst[0] = src[1];
		dst[1] = src[0];
		dst[2] = src[3];
		dst[3] = src[2];

		//dst[0] = src[0];
		//dst[1] = src[1];
		//dst[2] = src[2];
		//dst[3] = src[3];

	}
}
uint8_t SensorAR0144Set(uint16_t addr, uint16_t data)
{	
	uint16_t tmp_data = 0;
	//LBChange(2, (uint8_t*)&data, (uint8_t*)&tmp_data);
	tmp_data = ((data & 0xff00) >> 8) | ((data & 0x00ff) << 8);
	return I2C_Write2B(0x20, addr, tmp_data);
}

uint8_t SensorAR0144Get(uint16_t addr, uint16_t* data)
{
	uint8_t status = 0x00;
	uint16_t tmp_data = 0;
	
	status = I2C_Read2B(0x20, addr, &tmp_data);
	*data = ((tmp_data & 0xff00) >> 8) | ((tmp_data & 0x00ff) << 8);

	return status;
}

void SensorAR0144SetSeq(uint16_t reg_list[][3])
{

	for (int i = 0; i < 512; i++)
	{
		if (reg_list[i][0] == 0x00)
		{
			SensorAR0144Set(reg_list[i][1], reg_list[i][2]);
		}
		else if (reg_list[i][0] == 0x01)
		{
			//printf("usleep %d us \r\n", reg_list[i][1]);
			Sleep(reg_list[i][1]>>5);
		}
		else
		{
			break;
		}
	}
	//return CY_U3P_SUCCESS;
	
}

void Sensor9712SetSeq(uint16_t reg_list[][3])
{
	for (int i = 0; i < 512; i++)
	{
		if (reg_list[i][0] == 0x00)
		{
			Sensor9712Set((uint8_t)(reg_list[i][1] & 0x00ff), (uint8_t)(reg_list[i][2]&0x00ff));
		}
		else if (reg_list[i][0] == 0x01)
		{
			//printf("usleep %d us \r\n", reg_list[i][1]);
			Sleep(reg_list[i][1]>>5);
		}
		else
		{
			break;
		}
		}
}

void Sensor9712GetSeq(uint16_t reg_list[][3])
{
	uint8_t tmpdata=0;

	for (int i = 0; i < 512; i++)
	{
		if (reg_list[i][0] == 0x00)
		{
			Sensor9712Get((uint8_t)(reg_list[i][1] & 0x00ff), &tmpdata);
			if ((uint8_t)(reg_list[i][2] & 0x00ff) != tmpdata)
			{
				//printf("addr: 0x%x read: 0x%x exp:0x%x\r\n", reg_list[i][1], tmpdata, (reg_list[i][2] & 0x00ff));
			}
		}
		else if (reg_list[i][0] == 0x01)
		{
			//printf("usleep %d us \r\n", reg_list[i][1]);
			Sleep(reg_list[i][1] >> 5);
		}
		else
		{
			break;
		}
	}
}

void platformInit(void)
{
	SVDPUSet8b(SYSCTRL1, 0x00);
	SVDPUGetThenSet8b(SYSCTRL_ID, ((0x0 << 4) | (0x0 << 5)), ((0x1 << 4) | (0x1 << 5)));
	Sleep(10);
}

//#DOVDD2(IO) #DOVDD1(IO)#AVDD2(analog)#AVDD1(analog) #DVDD2(core)#DVDD1(core) #led2 #led1
void platformPowerUp(void)
{
	//SENSOR 0 PowerUp
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 4), (0x1 << 4)); //VAA_PIX first
	Sleep(1);
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 6), (0x1 << 6)); //VDD_IO then
	Sleep(1);
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 2), (0x1 << 2)); //DVDD 
	Sleep(1);

	//SENSOR 1 PowerUp
	/*
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 7), (0x1 << 7)); //VDD_IO first
	Sleep(1);
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 5), (0x1 << 5)); //VAA_PIX then
	Sleep(1);
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 3), (0x1 << 3)); //DVDD 
	Sleep(5);
	*/
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 5), (0x1 << 5)); //VAA_PIX first
	Sleep(1);
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 7), (0x1 << 7)); //VDD_IO then
	Sleep(1);
	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 3), (0x1 << 3)); //DVDD 
	Sleep(5);
	SVDPUGetThenSet8b(SYSCTRL0, ((0x0 << 4) | (0x0 << 5)), ((0x1 << 4) | (0x1 << 5)));
	Sleep(20);
}

void  SensorInit(void)
{
#if 0

#else
	SVDPUSet(PATCONF_ID, 0x04000320);
	SVDPUSet8b(SYSCTRL0, 0x01);
	//sensor reset
	SVDPUGetThenSet8b(SYSCTRL0, ((0x1 << 0) | (0x0 << 4) | (0x0 << 5)), ((0x1 << 0) | (0x1 << 4) | (0x1 << 5)));
	Sleep(100);
	SVDPUGetThenSet8b(SYSCTRL0, ((0x1 << 0) | (0x1 << 4) | (0x1 << 5)), ((0x1 << 0) | (0x1 << 4) | (0x1 << 5)));

	SensorAR0144SetSeq(reg_list_AR0144_SEQ);
	Sleep(1);
	SensorAR0144SetSeq(reg_list_AR0144_1024800);
	
	//select senor1
	SVDPUGetThenSet8b(SYSCTRL0, (0x1 << 2), (0x1 << 2));
	SensorAR0144SetSeq(reg_list_AR0144_SEQ);
	Sleep(1);
	SensorAR0144SetSeq(reg_list_AR0144_1024800_C);
	//Sensor9712SetSeq(reg_list_ov9712_1024800);
	//Sensor9712Set(0xca, 0x24);
	//Sensor9712Set(0x12, 0x02);
	//Sensor9712Set(0x97, 0x88);
	Sleep(1);
	//reset
	SVDPUGetThenSet8b(SYSCTRL0, (0x1 << 0), (0x1 << 0));

	//enable senor0 output
	//SVDPUGetThenSet8b(SYSCTRL0, (0x1 << 1), (0x1 << 1));

	//select senor0
	SVDPUGetThenSet8b(SYSCTRL0, (0x0 << 2), (0x1 << 2));

	//SVDPUGetThenSet8b(SYSCTRL_ID, (0x0 << 2), (0x1 << 2));
	//SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 0), (0x1 << 0)); // 

	SVDPUGetThenSet8b(SYSCTRL1, (0x1 << 0), (0x1 << 0)); //DVDD 

	Sleep(20);
	//Sensor9712GetSeq(reg_list_ov9712_1024800);
#endif
}

void StreamSwitchToDepth(void)
{
    SVDPUGetThenSet(0x200, 0x1 << 13, 0x1 << 13);
    SVDPUSet(0x200 + (12 << 2), (0 << 31) | (1 << 29) | (0 << 24) | ((2510) * (822 - 3)));
    Sleep(100);
    SVDPUSet(0x200 + (12 << 2), (1 << 31) | (1 << 29) | (1 << 24) | ((2510) * (822 - 3)));
}

void StreamSwitchToIR(void)
{
    SVDPUGetThenSet(0x200, 0x0 << 13, 0x1 << 13);
    SVDPUSet(0x200 + (12 << 2), (0 << 31) | (1 << 29) | (0 << 24) | ((2510) * (15 - 3)));
    Sleep(100);
    SVDPUSet(0x200 + (12 << 2), (1 << 31) | (1 << 29) | (1 << 24) | ((2510) * (15 - 3)));
}