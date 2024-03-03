#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "MvCameraControl.h"
bool g_bExit = false;

// ch:等待按键输入 | en:Wait for key press
void PressEnterToExit(void)
{
    int c;
    while ( (c = getchar()) != '\n' && c != EOF );
    fprintf( stderr, "\nPress enter to exit.\n");
    while( getchar() != '\n');
    g_bExit = true;
    sleep(1);
}

static  void* WorkThread(void* pUser)
{
	int nRet = MV_OK;
	MV_FRAME_OUT stOutFrame = {0};

	while(true)
	{
		nRet = MV_CC_GetImageBuffer(pUser, &stOutFrame, 1000);
		if (nRet == MV_OK)
		{
			printf("Get Image Buffer: Width[%d], Height[%d], FrameNum[%d],enPixelType[%x]\n",
				stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight, stOutFrame.stFrameInfo.nFrameNum,stOutFrame.stFrameInfo.enPixelType);

			nRet = MV_CC_FreeImageBuffer(pUser, &stOutFrame);
			if(nRet != MV_OK)
			{
				printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
			}
		}
		else
		{
			printf("Get Image fail! nRet [0x%x]\n", nRet);
		}
		if(g_bExit)
		{
			break;
		}
	}

	return 0;
}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (NULL == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is NULL!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

		// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
		printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
	}
	else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chSerialNumber);
		printf("Model Name: %d\n\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chModelName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chSerialNumber);
		printf("Model Name: %d\n\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chModelName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chSerialNumber);
		printf("Model Name: %d\n\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chModelName);
	}
	else
	{
		printf("Not support.\n");
	}

	return true;
}

//ch:用户输入转为传输层类型 | en:Convert user select to TLayerType
unsigned int UserSelect2TLayerType(unsigned int nSelect)
{
	switch(nSelect)
	{
	case 0:
			return MV_GENTL_GIGE_DEVICE;
	case 1:
			return MV_GENTL_CAMERALINK_DEVICE;
	case 2:
			return MV_GENTL_CXP_DEVICE;
	case 3:
			return MV_GENTL_XOF_DEVICE;
	default:
			return -1;
	}
}

int main()
{
	int nRet = MV_OK;
	void* handle = NULL;

	printf("[0]: Enum GIGE Interface Devices\n");
	printf("[1]: Enum CAMERALINK Interface Devices\n");
	printf("[2]: Enum CXP Interface Devices\n");
	printf("[3]: Enum XOF Interface Devices\n\n");

	do 
	{
		// ch:初始化SDK | en:Initialize SDK
		nRet = MV_CC_Initialize();
		if (MV_OK != nRet)
		{
			printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
			break;
		}

		MV_CC_DEVICE_INFO_LIST stDeviceList={0};

		unsigned int nType = 0;
		printf("Please Input Enum Interfaces Type(0-%d):", 3);
		scanf("%d", &nType);

		unsigned int nTLayerType = UserSelect2TLayerType(nType);
		if(-1 == nTLayerType)
		{
			printf("Input error!\n");
			break;
		}

		//ch:枚举采集卡设备 | en:Enum interface devices
		nRet = MV_CC_EnumDevices(nTLayerType,&stDeviceList);
		if (MV_OK != nRet)
		{
			printf("Enum Interfaces Devices fail! nRet [0x%x]\n", nRet);
			break;
		}

		if (stDeviceList.nDeviceNum > 0)
		{
			for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
			{
				printf("[device %d]:\n", i);
				MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
				if (NULL == pDeviceInfo)
				{
					break;
				} 
				PrintDeviceInfo(pDeviceInfo);            
			}  
		} 
		else
		{
			printf("Find No Devices!\n");
			break;
		}

		printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum-1);
		unsigned int nIndex = 0;
		scanf("%d", &nIndex);

		if (nIndex >= stDeviceList.nDeviceNum)
		{
			printf("Input error!\n");
			break;
		}

		// ch:选择设备并创建句柄 | en:Select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			printf("Create Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:打开设备 | en:Open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			printf("Open Device fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
		if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
		{
			int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
			if (nPacketSize > 0)
			{
				nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
				if(nRet != MV_OK)
				{
					printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
				}
			}
			else
			{
				printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
			}
		}

		// ch:设置触发模式为off | en:Set trigger mode as off
		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
		if (MV_OK != nRet)
		{
			printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:开始取流 | en:Start grab image
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:创建取流线程 | en:Create grabbing thread
		pthread_t nThreadID;
        nRet = pthread_create(&nThreadID, NULL ,WorkThread , handle);
        if (nRet != MV_OK)
        {
            printf("thread create failed.ret = %d\n",nRet);
            break;
        }

		printf("Press a key to stop grabbing.\n");
		PressEnterToExit();

		g_bExit = true;
		sleep(1);

		// ch:停止取流 | en:Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:关闭设备 | Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			printf("ClosDevice fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:销毁句柄 | Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
		handle = NULL;
	} while (0);

	if (handle != NULL)
	{
		MV_CC_DestroyHandle(handle);
		handle = NULL;
	}

	// ch:反初始化SDK | en:Finalize SDK
	MV_CC_Finalize();

	printf("exit\n");
    return 0;
}
