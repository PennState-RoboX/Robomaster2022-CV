#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "MvCameraControl.h"

#define IMAGE_NAME_LEN 256

bool IsHBPixelFormat(MvGvspPixelType enType)
{
	switch(enType)
	{
	case PixelType_Gvsp_HB_Mono8:
	case PixelType_Gvsp_HB_Mono10:
	case PixelType_Gvsp_HB_Mono10_Packed:
	case PixelType_Gvsp_HB_Mono12:
	case PixelType_Gvsp_HB_Mono12_Packed:
	case PixelType_Gvsp_HB_Mono16:
	case PixelType_Gvsp_HB_RGB8_Packed:
	case PixelType_Gvsp_HB_BGR8_Packed:
	case PixelType_Gvsp_HB_RGBA8_Packed:
	case PixelType_Gvsp_HB_BGRA8_Packed:
	case PixelType_Gvsp_HB_RGB16_Packed:
	case PixelType_Gvsp_HB_BGR16_Packed:
	case PixelType_Gvsp_HB_RGBA16_Packed:
	case PixelType_Gvsp_HB_BGRA16_Packed:
	case PixelType_Gvsp_HB_YUV422_Packed:
	case PixelType_Gvsp_HB_YUV422_YUYV_Packed:
	case PixelType_Gvsp_HB_BayerGR8:
	case PixelType_Gvsp_HB_BayerRG8:
	case PixelType_Gvsp_HB_BayerGB8:
	case PixelType_Gvsp_HB_BayerBG8:
	case PixelType_Gvsp_HB_BayerRBGG8:
	case PixelType_Gvsp_HB_BayerGB10:
	case PixelType_Gvsp_HB_BayerGB10_Packed:
	case PixelType_Gvsp_HB_BayerBG10:
	case PixelType_Gvsp_HB_BayerBG10_Packed:
	case PixelType_Gvsp_HB_BayerRG10:
	case PixelType_Gvsp_HB_BayerRG10_Packed:
	case PixelType_Gvsp_HB_BayerGR10:
	case PixelType_Gvsp_HB_BayerGR10_Packed:
	case PixelType_Gvsp_HB_BayerGB12:
	case PixelType_Gvsp_HB_BayerGB12_Packed:
	case PixelType_Gvsp_HB_BayerBG12:
	case PixelType_Gvsp_HB_BayerBG12_Packed:
	case PixelType_Gvsp_HB_BayerRG12:
	case PixelType_Gvsp_HB_BayerRG12_Packed:
	case PixelType_Gvsp_HB_BayerGR12:
	case PixelType_Gvsp_HB_BayerGR12_Packed:
		return true;
	default:
		return false;
	}
}
// ch:等待用户输入enter键来结束取流或结束程序
//en:wait for user to input enter to stop grabbing or end the sample program
void PressEnterToExit(void)
{
	int c;
	while((c = getchar()) != '\n' && c != EOF);
	fprintf(stderr, "\nPress enter to exit.\n");
	while(getchar() != '\n');
}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if(NULL == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is NULL");
		return false;
	}

	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

		// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
		printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
		printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}

	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName);
		printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
	}

	else
	{
		printf("Not Sopport.\n");
	}

	return true;
}

int main()
{
	int nRet = MV_OK;
	void* handle = NULL;
	unsigned char* pDstBuf = NULL;
	bool isDevOpen = false;
	do 
	{
		// ch:初始化SDK | en:Initialize SDK
		nRet = MV_CC_Initialize();
		if (MV_OK != nRet)
		{
			printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
			break;
		}

		MV_CC_DEVICE_INFO_LIST stDeviceList;
		memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

		//ch:枚举设备 | en:enum device
		nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
		if (MV_OK != nRet)
		{
			printf("MV_CC_EnumDevices fail! nRet [%x]\n", nRet);
			break;
		}
		if (stDeviceList.nDeviceNum > 0)
		{
			for (int i = 0; i < stDeviceList.nDeviceNum; i++)
			{
				printf("[device %d]:\n", i);
				MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
				if(NULL == pDeviceInfo)
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

		if(nIndex >= stDeviceList.nDeviceNum)
		{
			printf("InPut Error!\n");
			break;
		}

		//ch:选择设备并创建句柄 | en:select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet);
			break;
		}

		//ch:打开设备 | en:open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			printf("MV_CC_OpenDevice fail! nRet [%x]\n", nRet);
			break;
		}
		isDevOpen = true;
		// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
		if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
		{
			int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
			if (nPacketSize > 0)
			{
				nRet = MV_CC_SetIntValueEx(handle,"GevSCPSPacketSize",nPacketSize);
				if(nRet != MV_OK)
				{
					printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
				}
			}
			else
			{
				printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
			}
		}

		//ch:设置触发模式为off | en:set trigger mode as off
		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
		if (MV_OK != nRet)
		{
			printf("MV_CC_SetTriggerMode fail! nRet [%x]!\n", nRet);
			break;
		}

		// ch:获取数据包大小 | en:Get payload size
		MVCC_INTVALUE_EX stParam;
		memset(&stParam, 0, sizeof(MVCC_INTVALUE));
		//使用camera新的接口
		nRet = MV_CC_GetIntValueEx(handle, "PayloadSize", &stParam);
		if (MV_OK != nRet)
		{
			printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
			break;
		}
		unsigned int nPayloadSize = stParam.nCurValue;

		// ch:开始取流 | en:Start grab image
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		MV_FRAME_OUT stImageInfo = {0};
		unsigned int nImageNum = 10;
		char chImageName[IMAGE_NAME_LEN] = {0};
		MV_CC_HB_DECODE_PARAM stDecodeParam = {0};

		for (unsigned int i = 0; i < nImageNum; i++)
		{
			nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000);
			if(MV_OK == nRet)
			{
				printf("Get One Frame: Width[%d], Height[%d], FrameNum[%d], PixelFormat[0x%x]\n", 
					stImageInfo.stFrameInfo.nWidth, stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nFrameNum, stImageInfo.stFrameInfo.enPixelType);
				nRet = MV_CC_FreeImageBuffer(handle, &stImageInfo);
				if (MV_OK != nRet)
				{
					printf("Free ImageBuffer fail! nRet [0x%x]\n", nRet);
					break;
				}
			   if (false == IsHBPixelFormat(stImageInfo.stFrameInfo.enPixelType))
			   {
				   printf("Not HB Picture!\n");
				   break;
			   }
				//ch:无损压缩解码 | en:Lossless compression decoding
				stDecodeParam.pSrcBuf = stImageInfo.pBufAddr;
				//memcpy(stDecodeParam.pSrcBuf,stImageInfo.pBufAddr,stImageInfo.stFrameInfo.nFrameLen);
				stDecodeParam.nSrcLen = stImageInfo.stFrameInfo.nFrameLen;

				if (NULL == pDstBuf)
				{
					pDstBuf = (unsigned char *)malloc(sizeof(unsigned char) * (nPayloadSize));
					if (NULL == pDstBuf)
					{
						printf("malloc pDsrData fail!\n");
						break;
					}
				}
				stDecodeParam.pDstBuf = pDstBuf;
				stDecodeParam.nDstBufSize = nPayloadSize;
				if (NULL== stDecodeParam.pSrcBuf)
				{
					printf("stDecodeParam.pSrcBuf is null !\n");
					break;
				}
				else
				{
					nRet = MV_CC_HB_Decode(handle, &stDecodeParam);
					if (nRet != MV_OK)
					{
						printf("Decode fail![0x%x]\n", nRet);
						break;
					}
				}
				FILE* fp = NULL;
				sprintf(chImageName, "Image_w%d_h%d_fn%03d.raw", stDecodeParam.nWidth, stDecodeParam.nHeight, stImageInfo.stFrameInfo.nFrameNum);
				fp = fopen(chImageName, "wb");
				if (NULL == fp)
				{
					printf("Open file failed\n");
					break;
				}
				fwrite(stDecodeParam.pDstBuf, 1, stDecodeParam.nDstBufLen, fp);
				fclose(fp);
				printf("Decode succeed\n");
			}
			else
			{
				printf("Get Image fail! nRet[0x%x]\n", nRet);
			}
		}

		PressEnterToExit();

		// ch:停止取流 | en:Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		// ch:关闭设备 | en:Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			printf("Close Device fail! nRet [0x%x]\n", nRet);
			break;
		}
		isDevOpen = false;
		// ch:销毁句柄 | en:Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
		handle = NULL;
	} while (0);
	//释放内存
	if(pDstBuf)
	{
		free(pDstBuf);
		pDstBuf = NULL;
	}


	if (isDevOpen == true)
	{
		MV_CC_CloseDevice(handle);
	}
	if (handle != NULL)
	{
		MV_CC_DestroyHandle(handle);
		handle = NULL;
	}

	// ch:反初始化SDK | en:Finalize SDK
	MV_CC_Finalize();

	printf("exit.\n");
	
	return 0;
}
