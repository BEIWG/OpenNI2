// UVCExtensionApp.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
//#include <iostream>
//#include <fstream>
#include "DCamDrv.h"
#include "XnOS.h"

#ifdef _MSC_VER
#include <windows.h>
#include "SVCeuctl.h"
#else
#include "libuvc/libuvc.h"
#endif

using namespace std;


#define SME_CMD_VIDEO_EN        (0x01)
#define SME_CMD_IIC2SPI_EN      (0x02)
#define SME_CMD_RPOJ_SET        (0x03)
#define SME_CMD_DEPTH_EN        (0x04)
#define SME_CMD_DEPLUT_EN       (0x05)
#define SME_CMD_INPHASE_SET     (0x06)

#define EU_CS_XFER_CONFIG       (0x01)
#define EU_CS_XFER_DATA         (0x02)

#define EU_CS_FLASH_ERASE		(0x03)
#define EU_CS_FLASH_RWCONFIG    (0x04)
#define EU_CS_FLASH_DATA        (0x05)
#define EU_CS_SME_OP            (0x06)

#ifndef	 _MSC_VER
uvc_device_handle_t *g_devh = NULL;
const uvc_extension_unit_t * g_euctrl = NULL;

void uvc_eu_init(void *devh)
{
	g_devh = (uvc_device_handle_t *)devh;
	g_euctrl = uvc_get_extension_units(g_devh);
}

int uvc_eu_get_ctrl(uint8_t ctrl, void *data, int len)
{
	if (g_euctrl == NULL || g_devh == NULL)
	{
		printf("usb extension units is not fount \r\n");
		return -1;
	}

	return uvc_get_ctrl(g_devh, g_euctrl->bUnitID, ctrl, data, len, UVC_GET_CUR);
}

int uvc_eu_set_ctrl(uint8_t ctrl, void *data, int len)
{
	if (g_euctrl == NULL || g_devh == NULL)
	{
		printf("usb extension units is not fount \r\n");
		return -1;
	}

	return uvc_set_ctrl(g_devh, g_euctrl->bUnitID, ctrl, data, len);
}
#endif

#ifdef	 _MSC_VER

SVCeuctl * g_EUctl = NULL;

void uvc_eu_init(void *devh)
{
	g_EUctl = (SVCeuctl *)devh;
}

int uvc_eu_get_ctrl(uint8_t ctrl, void *data, int len)
{
	if (g_EUctl == NULL)
	{
		printf("usb extension units is not fount \r\n");
		return -1;
	}

	return g_EUctl->eu_get_ctrl(ctrl, data, len);
}

int uvc_eu_set_ctrl(uint8_t ctrl, void *data, int len)
{
	if (g_EUctl == NULL)
	{
		printf("usb extension units is not fount \r\n");
		return -1;
	}

	return g_EUctl->eu_set_ctrl(ctrl, data, len);
}

#endif


void DCAM_DRV_Init(void *devh)
{
	uvc_eu_init(devh);
}
            
uint8_t SME_FlashErase(uint8_t mode, uint32_t flashaddr)
{
	uint8_t data[8];
	uint32_t readCount;
	uint8_t hr = 0;
	int ret_eu = 0;

	data[0] = mode;
	data[1] = (flashaddr >> 0 ) & 0x000000ff;
	data[2] = (flashaddr >> 8 ) & 0x000000ff;
	data[3] = (flashaddr >> 16) & 0x000000ff;
	data[4] = (flashaddr >> 24) & 0x000000ff;
	data[5] = 0x00;

	ret_eu = uvc_eu_set_ctrl(EU_CS_FLASH_ERASE, (void *)data, 6);
  
	if (ret_eu != 6)
	{
		printf("SME_FlashErase is failed!\r\n");
		return 2;
	}
	return hr;
}

uint8_t SME_CMD_Config(uint16_t cmd,uint32_t value)
{
	uint8_t data[8];
	uint8_t hr = 0;
	int ret_eu = 0;
  
	data[0] = (cmd >> 0) & 0x000000ff;
	data[1] = (cmd >> 8) & 0x000000ff;
	data[2] = (value >> 0)  & 0x000000ff;
	data[3] = (value >> 8)  & 0x000000ff;
	data[4] = (value >> 16) & 0x000000ff;
	data[5] = (value >> 24) & 0x000000ff;

	ret_eu = uvc_eu_set_ctrl(EU_CS_SME_OP, (void *)data, 6);
  
	if (ret_eu != 6)
	{
		printf("SME_CMD_Config is failed!\r\n");
		return 2;
	}
	return hr;
}

uint8_t SME_Video_En(uint32_t value)
{
	return SME_CMD_Config(SME_CMD_VIDEO_EN, value);
}

uint8_t SME_IIC2SPI_En(uint32_t value)
{
	return SME_CMD_Config(SME_CMD_IIC2SPI_EN, value);
}

uint8_t SME_Projection_Set(uint32_t value)
{
	return SME_CMD_Config(SME_CMD_RPOJ_SET, value);
}

uint8_t SME_DispDepth_En(uint32_t value)
{
	return SME_CMD_Config(SME_CMD_DEPTH_EN, value);
}

uint8_t SME_Depth_Sel(uint32_t value)
{
	return SME_CMD_Config(SME_CMD_DEPLUT_EN, value);
}

uint8_t SME_InPhase_Set(uint32_t value)
{
	return SME_CMD_Config(SME_CMD_INPHASE_SET, value);
}

uint8_t SME_FlashWRConfig(uint8_t cmd,uint32_t flashaddr, uint16_t len)
{
	uint8_t data[8];
	uint8_t hr = 0;
	int ret_eu = 0;
  
	data[0] = (flashaddr >> 0) & 0x000000ff;
	data[1] = (flashaddr >> 8) & 0x000000ff;
	data[2] = (flashaddr >> 16) & 0x000000ff;
	data[3] = (flashaddr >> 24) & 0x000000ff;

	if (len > 256)
	{
		data[4] = 255;
		printf("SME_FlashWRConfig() len must <= 256  \r\n");
	}
	else
	{
		data[4] = len-1;
	}
	
	data[5] = cmd;

	ret_eu = uvc_eu_set_ctrl(EU_CS_FLASH_RWCONFIG, (void *)data, 6);
  
	if (ret_eu != 6)
	{
    printf("SME_FlashWRConfig is failed!\r\n");
    return 2;
	}
	return hr;
}

uint8_t SME_FlashWriteData(uint8_t *buf, uint16_t len)
{

	uint8_t data[256];
	uint8_t hr = 0;
	int repeat_t = 0;
	int ret_eu = 0;

	if (len > 256)
	{
		len = 256;
		printf("SME_FlashWriteData() len must <= 256  \r\n");
	}

	memset(data,255,256);
	memcpy(data, buf, len);

	if ((len % 64) != 0)
	{
		repeat_t = (len / 64) + 1;
	}
	else
	{
		repeat_t = (len / 64);
	}

	for(int i = 0; i < repeat_t; i++)
	{
		ret_eu |= uvc_eu_set_ctrl(EU_CS_FLASH_DATA, (void *)(data + i * 64), 64);
	}


	if (ret_eu != 64)
	{
		printf("SME_FlashWriteData is failed!\r\n");
		return 2;
	}
	return hr;
}

uint8_t SME_FlashReadData(uint8_t *buf, uint16_t len)
{

	uint8_t data[256];
	uint8_t hr = 0;
	int repeat_t = 0;
	int ret_eu = 0;
    
	memset(data, 0, 256);

	if (len > 256)
	{
		len = 256;
		printf("SME_FlashReadData() len must <= 256  \r\n");
		//return S_FALSE;
	}

	if ((len % 64) != 0)
	{
		repeat_t = (len / 64) + 1;
	}
	else
	{
		repeat_t = (len / 64);
	}

	for (int i = 0; i < repeat_t; i++)
	{
		ret_eu |= uvc_eu_get_ctrl(EU_CS_FLASH_DATA, (void *)(&data[i * 64]), 64);	
		//printf("read from %x \r\n", (void *)(data + i * 64));
	}

	if (ret_eu != 64)
	{
		printf("SME_FlashReadData is failed!\r\n");
		return 2;
	}

	memcpy(buf, data, len);

	return hr;
}

uint8_t FlashOPInit(void)
{
	uint8_t status = 0x00;

	uint8_t cmd;

	cmd = (0x1 << 2) ;//clear inx and buf
	status = SME_FlashWRConfig(cmd, 0x0, 64);

	if (status != 0x00)
		printf("SME_FlashReadData is failed!\r\n");

	return status;
}

// 0x1 :enable write pretect 0x2:disable write pretect
uint8_t FlashWPSet(uint8_t WP)
{
	uint8_t status = 0x00;
	uint8_t cmd;

	cmd = (WP << 4);//clear inx and buf

	status = SME_FlashWRConfig(cmd, 0, 0);

	if (status != 0x00)
		printf("FlashWPSet is failed!\r\n");

	return status;
}

uint8_t FlashRead(uint32_t flashaddr, uint8_t *buf, uint16_t len)
{
	uint8_t status = 0x00;
	uint8_t cmd;

	cmd = (0x0 << 2) | (0x1 << 1);//read
	status = SME_FlashWRConfig(cmd, flashaddr, len);

	status = SME_FlashReadData(buf, len);

	if (status != 0x00)
		printf("FlashRead is failed!\r\n");

	return status;
}

uint8_t FlashWrite(uint32_t flashaddr, uint8_t *buf, uint16_t len)
{
	uint8_t status = 0x00;	
	uint8_t cmd;
	
	status = SME_FlashWriteData(buf, len);

	cmd = (0x0 << 2) | (0x1 << 0);//write
	status = SME_FlashWRConfig(cmd, flashaddr, len);

	if (status != 0x00)
		printf("FlashWrite is failed!\r\n");

	return status;
}

uint8_t FlashErase(uint8_t mode, uint32_t flashaddr)
{
	uint8_t status = 0x00;

	status = SME_FlashErase(mode, flashaddr);

	if (status != 0x00)
		printf("FlashErase is failed!\r\n");

	return status;
}

#define _64K	 (0x10000)
#define buf_len  (256)
#define rd_len   (256)

//void  WriteREFBin(void)
void  WriteREFBin(uint32_t ref_baseaddr,const char * reffilename)
{
	FILE *fp = NULL;
	
	fp = fopen(reffilename, "rb");
	if (!fp)
	{
		printf("WriteREFBin: open file %s failed!\n", reffilename);
		return;
	}
	
	fseek(fp, 0L, SEEK_END);
	size_t srcSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	printf("srcSize = %d \r\n", srcSize);
	uint8_t *refdata = (uint8_t*) malloc(srcSize);
	if (!refdata)
	{
		fclose(fp);
		printf("WriteREFBin: Malloc buffer failed!\n");
		return;
	}
	
	uint8_t tmp_data[256];
	fread(refdata, 1, srcSize, fp);
	fclose(fp);

	int repeat_num;
	int rem_num;

	//disable write pretect
	FlashWPSet(0x2);

	//erase flash for Write ref.bin
	repeat_num = 0;
	repeat_num = (int)(srcSize / _64K) + 1;
	printf("erase_num = %d \r\n", repeat_num);

	for (int i = 0; i < repeat_num; i++)
	{
		FlashErase(2, ref_baseaddr+i*_64K);
		xnOSSleep(300);
	}
	printf("FlashErase is done! \r\n");

	//PP flash
	repeat_num = 0;
	rem_num = 0;
	repeat_num = (int)(srcSize / buf_len);
	rem_num = (srcSize % buf_len);

	printf("write repeat = %d \r\n", repeat_num);
	FlashOPInit();

	for (int i = 0; i < repeat_num; i++)
	{
		FlashWrite(ref_baseaddr + i * buf_len, &refdata[i * buf_len], buf_len);
		printf("+");
	}

	if (rem_num != 0)
	{
		FlashWrite(ref_baseaddr + repeat_num * buf_len, &refdata[repeat_num * buf_len], rem_num);
		printf("+\r\n");
	}
	else
	{
		printf("\r\n");
	}
	printf("FlashWrite is done! \r\n");
	
	
	repeat_num = (int)(srcSize / rd_len);
	rem_num = (srcSize % rd_len);

	printf("repeat_num %d rem_num %d! \r\n", repeat_num, rem_num);


	for (int i = 0; i < repeat_num; i++)
	{
		FlashRead(ref_baseaddr + i * rd_len, tmp_data, rd_len);

		for (int j = 0; j < rd_len; j++)
		{
			if (refdata[i * rd_len + j] != tmp_data[j])
			{
				printf("flash data is error @%x,ref:%x read: %x \r\n", (ref_baseaddr + i * rd_len + j), refdata[i * rd_len + j], tmp_data[j]);
			}
		}

	}

	if (rem_num != 0)
	{

		FlashRead(ref_baseaddr + repeat_num * rd_len, &refdata[repeat_num * rd_len], rem_num);

		for (int j = 0; j < rem_num; j++)
		{
			if (refdata[ref_baseaddr + repeat_num * rd_len + j] != tmp_data[j])
			{
				printf("flash data is error @%x,ref:%x read: %x \r\n", (ref_baseaddr + repeat_num * rd_len + j), refdata[repeat_num * rd_len + j], tmp_data[j]);
			}
		}
    
	}

	printf("Flash verify is done! \r\n");

	//enable write pretect
	FlashWPSet(0x1);
}

double get_ncoef(int x, double y, double minidis, double depth_ref)
{
	//n = (focal length * base length)/pixel width
	double n = 0.0;
	n = (y*(x + minidis * 16)) / (1 - y / depth_ref);
	return n;
}

double calc_n(double minidis,double depth_ref,double reg_list[][3])
{
	// double depth_ref = 1233.0;
	// double minidis = -72.0;
	double sum = 0;
	int i = 0;

	for (i = 0; i < 512; i++)
	{
		if (reg_list[i][0] == 0x00)
		{
			sum += get_ncoef(reg_list[i][1], reg_list[i][2], minidis, depth_ref);
			printf("i: %d n sum %f \n", i, sum);
		}
		else
		{
			break;
		}
	}

	sum = sum / i;
	printf("i: %d n sum %f \n", i, sum);
	return sum;
}

void depth_lut2flash(double reg_list[][3], double depth_ref, int minidis, uint32_t flashaddr)
{
	int x;
	double tmp_dep = 0.0;
	uint32_t depth_data;
	uint32_t disp2dedth_lut[2048];
	int repeat_num = 0;
	int rem_num = 0;
	int srcSize = 0;
	double sum = 0.;
	double n = 0.;
  
	n = calc_n(minidis,depth_ref,reg_list);
  
	for (x = 0; x < 2048; x++)
	{

		tmp_dep = ((n) / ((x + minidis * 16) + ((n) / depth_ref))); //cm
		tmp_dep *= 8.0;

		if (tmp_dep >= 65535)
			tmp_dep = 65535;

		depth_data = (uint32_t)tmp_dep;
		disp2dedth_lut[x] = (x<<16)|depth_data; //

		//if ((x&0x000f)== 0)
			//printf("x:%d = %x \r\n", x, disp2dedth_lut[x]);
	}

	//disable write pretect
	FlashWPSet(0x2);

	FlashErase(1, flashaddr);
	xnOSSleep(300);
	FlashOPInit();
  
	repeat_num = 0;
	rem_num = 0;
	srcSize = 2048 * 4;
	repeat_num = (int)(srcSize / buf_len);
	rem_num = (srcSize % buf_len);

	for (int i = 0; i < repeat_num; i++)
	{
		FlashWrite(flashaddr + i * buf_len, (uint8_t *)&disp2dedth_lut[i * buf_len/4], buf_len);
		printf("+");
	}

	if (rem_num != 0)
	{
		FlashWrite(flashaddr + repeat_num * buf_len, (uint8_t *)&disp2dedth_lut[repeat_num * buf_len/4], rem_num);
		printf("+\r\n");
	}
	else
	{
		printf("\r\n");
	}

	//enable write pretect
	FlashWPSet(0x1);

}
