#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "MvCameraControl.h"

void* g_hInterface = NULL;

// ch:等待按键输入 | en:Wait for key press
bool g_bExit = false;

void PressEnterToExit(void)
{
    int c;
    while ( (c = getchar()) != '\n' && c != EOF );
    fprintf( stderr, "\nPress enter to exit.\n");
    while( getchar() != '\n');
    g_bExit = true;
    sleep(1);
}

// ch:打印采集卡信息 | en:Print interface info
bool PrintInterfaceInfo(MV_INTERFACE_INFO* pstInterfaceInfo)
{
	if (NULL == pstInterfaceInfo)
	{
		printf("The Pointer of pstInterfaceInfo is NULL!\n");
		return false;
	}
	printf("Display name: %s\n",pstInterfaceInfo->chDisplayName);
	printf("Serial number: %s\n",pstInterfaceInfo->chSerialNumber);
	printf("model name: %s\n",pstInterfaceInfo->chModelName);
	printf("\n");

	return true;
}

// ch:设置/获取枚举类型值 | en:Set/Get enum value
void Set_Get_Enum(const char* str)
{
	MVCC_ENUMVALUE stEnumValue = {0};
	MVCC_ENUMENTRY stEnumentryInfo = {0};

	int nRet = MV_CC_GetEnumValue(g_hInterface,str, &stEnumValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str, nRet);
		return;
	}

	stEnumentryInfo.nValue = stEnumValue.nCurValue;
	nRet = MV_CC_GetEnumEntrySymbolic(g_hInterface,str, &stEnumentryInfo);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s = [%s] Success!\n",str,stEnumentryInfo.chSymbolic);
	}

	nRet = MV_CC_SetEnumValue(g_hInterface,str,stEnumValue.nCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%s] Success!\n",str,stEnumentryInfo.chSymbolic);
	}
}

// ch:设置/获取bool类型值 | en:Set/Get bool value
void Set_Get_Bool(const char* str)
{
	bool bValue = false;
	int nRet = MV_CC_GetBoolValue(g_hInterface,str, &bValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%d] Success!\n",str,bValue);
	}

	nRet = MV_CC_SetBoolValue(g_hInterface,str, bValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%d] Success!\n",str,bValue);
	}
}

// ch:设置/获取整数类型值 | en:Set/Get int value
void Set_Get_Int(const char* str)
{
	MVCC_INTVALUE_EX stIntValue;
	int nRet = MV_CC_GetIntValueEx(g_hInterface,str,&stIntValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%d] Success!\n",str,stIntValue.nCurValue);
	}
	
	nRet = MV_CC_SetIntValueEx(g_hInterface,str,stIntValue.nCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%d] Success!\n",str,stIntValue.nCurValue);
	}
}

// ch:设置/获取字符串类型值 | en:Set/Get string value
void Set_Get_String(const char* str)
{
	MVCC_STRINGVALUE StringValue;
	int nRet = MV_CC_GetStringValue(g_hInterface,str, &StringValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%s] Success!\n",str,StringValue.chCurValue);
	}

	nRet = MV_CC_SetStringValue(g_hInterface,str, StringValue.chCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%s] Success!\n",str,StringValue.chCurValue);
	}
}

// ch:设置/获取浮点数类型值 | en:Set/Get float value
void Set_Get_Float(const char* str)
{
	MVCC_FLOATVALUE FloatValue;
	int nRet = MV_CC_GetFloatValue(g_hInterface,str, &FloatValue);
	if (MV_OK != nRet)
	{
		printf("Get %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Get %s =  [%f] Success!\n",str,FloatValue.fCurValue);
	}

	nRet = MV_CC_SetFloatValue(g_hInterface,str, FloatValue.fCurValue);
	if (MV_OK != nRet)
	{
		printf("Set %s Fail! nRet [0x%x]\n", str,nRet);
		return;
	}
	else
	{
		printf("Set %s = [%f] Success!\n",str,FloatValue.fCurValue);
	}
}

//ch:用户输入转为传输层类型 | en:Convert user select to TLayerType
unsigned int UserSelect2TLayerType(unsigned int nSelect)
{
	switch(nSelect)
	{
	case 0:
			return MV_GIGE_INTERFACE;
	case 1:
			return MV_CAMERALINK_INTERFACE;
	case 2:
			return MV_CXP_INTERFACE;
	case 3:
			return MV_XOF_INTERFACE;
	default:
			return -1;
	}
}

int main()
{
	int nRet = MV_OK;

	printf("[0]: GIGE Interface\n");
	printf("[1]: CAMERALINK Interface\n");
	printf("[2]: CXP Interface\n");
	printf("[3]: XOF Interface\n\n");

	do 
	{
		// ch:初始化SDK | en:Initialize SDK
		nRet = MV_CC_Initialize();
		if (MV_OK != nRet)
		{
			printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
			break;
		}

		MV_INTERFACE_INFO_LIST stInterfaceInfoList={0};

		unsigned int nType = 0;
		printf("Please Input Enum Interfaces Type(0-%d):", 3);
		scanf("%d", &nType);

		unsigned int nTLayerType = UserSelect2TLayerType(nType);
		if(-1 == nTLayerType)
		{
			printf("Input error!\n");
			break;
		}

		//ch:枚举采集卡 | en:Enum Interfaces
		nRet = MV_CC_EnumInterfaces(nTLayerType, &stInterfaceInfoList);
		if (MV_OK != nRet)
		{
			printf("Enum Interfaces fail! nRet [0x%x]\n", nRet);
			break;
		}
		
		if (stInterfaceInfoList.nInterfaceNum > 0)
		{
			for (unsigned int i = 0; i < stInterfaceInfoList.nInterfaceNum; i++)
			{
				printf("[Interface %d]:\n", i);
				MV_INTERFACE_INFO* pstInterfaceInfo = stInterfaceInfoList.pInterfaceInfos[i];
				if (NULL == pstInterfaceInfo)
				{
					break;
				} 
				PrintInterfaceInfo(pstInterfaceInfo);            
			}
			printf("Enum Interfaces success!\n\n");
		} 
		else
		{
			printf("Find No Interface!\n");
			break;
		}

		printf("Please Input Interfaces index(0-%d):", stInterfaceInfoList.nInterfaceNum-1);
		unsigned int nIndex = 0;
		scanf("%d", &nIndex);

		if (nIndex >= stInterfaceInfoList.nInterfaceNum)
		{
			printf("Input error!\n");
			break;
		}

		//ch:创建采集卡句柄 | en:Create interface handle
		nRet = MV_CC_CreateInterface(&g_hInterface, stInterfaceInfoList.pInterfaceInfos[nIndex]);
		if (MV_OK == nRet)
		{
			printf("Create Interface success!\n");
		}
		else
		{
			printf("Create Interface Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		//ch:打开采集卡 | en:Open interface
		nRet = MV_CC_OpenInterface(g_hInterface, "这块暂时随便传入一个非空指针即可");
		if (MV_OK == nRet)
		{
			printf("Open Interface success!\n");
		}
		else
		{
			printf("Open Interface fail! nRet [0x%x]\n", nRet);
			break;
		}

		//ch:采集卡属性操作,不同类型卡对各自常用属性进行获取和设置
		//en:Interface feature operation, get/set each common feature for different interface
		switch(nType)
		{
		case 0:
			{
				//ch:MV_GIGE_INTERFACE卡属性获取和设置操作 | en:Get/Set feature for MV_GIGE_INTERFACE
				Set_Get_Enum("StreamSelector");
				Set_Get_Enum("TimerSelector");
				Set_Get_Enum("TimerTriggerSource");
				Set_Get_Enum("TimerTriggerActivation");
				Set_Get_Bool("HBDecompression");
				Set_Get_Int("TimerDuration");
				Set_Get_Int("TimerDelay");
				Set_Get_Int("TimerFrequency");
				break;
			}
		case 1:
			{
				//ch:MV_CAMERALINK_INTERFACE卡属性操作 | en:Get/Set feature for MV_CAMERALINK_INTERFACE
				Set_Get_Enum("StreamSelector");
				Set_Get_Enum("CameraType");
				Set_Get_Enum("StreamPartialImageControl");
				Set_Get_Int("ImageHeight");
				Set_Get_Int("FrameTimeoutTime");
				break;
			}
		case 2:
			{
				//ch:MV_CXP_INTERFACE卡属性操作 | en:Get/Set feature for MV_CXP_INTERFACE
				Set_Get_Enum("StreamSelector");
				Set_Get_String("CurrentStreamDevice");
				Set_Get_Int("StreamEnableStatus");
				Set_Get_Bool("BayerCFAEnable");
				Set_Get_Bool("IspGammaEnable");
				Set_Get_Float("IspGamma");
				break;
			}
		case 3:
			{
				//ch:MV_XOF_INTERFACE卡属性操作 | en:Get/Set feature for MV_XOF_INTERFACE
				Set_Get_Enum("StreamSelector");
				Set_Get_String("CurrentStreamDevice");
				Set_Get_Int("StreamEnableStatus");
				Set_Get_Bool("BayerCFAEnable");
				Set_Get_Bool("IspGammaEnable");
				Set_Get_Float("IspGamma");
				break;
			}
		default:
			{
				printf("Input error!\n");
				break;
			}
		}

		//ch:关闭采集卡 | en:Close interface
		nRet = MV_CC_CloseInterface(g_hInterface);
		if (MV_OK == nRet)
		{
			printf("Close Interface success!\n");
		}
		else
		{
			printf("Close Interface Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		//ch:销毁采集卡句柄 | en:Destroy interface handle
		nRet = MV_CC_DestroyInterface(g_hInterface);
		if (MV_OK == nRet)
		{
			printf("Destroy Interface success!\n");
		}
		else
		{
			printf("Destroy Interface Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
		g_hInterface = NULL;
	} while (0);

	if (g_hInterface != NULL)
	{
		MV_CC_DestroyInterface(g_hInterface);
		g_hInterface = NULL;
	}

	// ch:反初始化SDK | en:Finalize SDK
	MV_CC_Finalize();

    printf("exit\n");
    return 0;
}
