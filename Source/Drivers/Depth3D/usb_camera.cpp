#include <stdio.h>
#include "camerads.h"

int camera_init(int width, int height)
{
	int connect_flag = -1;
	CCameraDS m_CamDS;
	int m_iCamCount = CCameraDS::CameraCount();

	if(m_iCamCount == 0)
	{
		return (-1);
	}

	for(int i = 0; i < m_iCamCount; i++)
	{
		char szCamName[1024];

		int retval = m_CamDS.CameraName(i, szCamName, sizeof(szCamName));
        	if (retval > 0)
		{
			if (!strcmp(szCamName, "FX3 UVC"))
			connect_flag = 1;    
			break;
		}
	}

	return connect_flag;
}
