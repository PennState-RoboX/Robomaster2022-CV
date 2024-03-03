#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <mutex>
#include <thread>
#include <iostream>
#include "MvCameraControl.h"

using namespace std;

bool g_bExit = false;  //线程结束标记
#define Max_Count 5       //队列 缓冲区个数； 根据实际情况调节
int m_nImageSize = 0;    // 图像大小
//自定义保存的图像信息的结构体
typedef struct _stImageNode_
{

	unsigned char* pData;
	unsigned int nFrameLen;
	
	unsigned short nWidth;
	unsigned short nHeight;
	unsigned int nFrameNum;
}stImageNode;

class ArrayQueue
{
public:
    ArrayQueue()
    {
        this->size = 0;
        this->start = 0;
        this->end = 0;
        this->Queue = NULL;
        this->Qlen = 0;
    }
    

    ~ArrayQueue()
    {
        g_mutex.lock();
		

        for (int i = 0; i< Qlen; i++)
        {
            Queue[i].nFrameNum = 0;
            Queue[i].nHeight = 0;
            Queue[i].nWidth = 0;
            Queue[i].nFrameLen = 0;  
            if(Queue[i].pData != NULL)
            {
                free(Queue[i].pData);
                Queue[i].pData = NULL;
            }                    

            printf(" free ArrayQueue [%d] !\r\n",i);
        }

        delete []Queue;
        Queue = NULL;

        size = 0;
        start = 0;
        end = 0;

        g_mutex.unlock();

    }

    // 队列初始化
    int Init(int nBufCount, int DefaultImagelen)
    {
        int nRet = 0 ;

        this->Queue = new (std::nothrow)stImageNode[nBufCount];
        if (this->Queue == NULL)
		{
			return MV_E_RESOURCE;
		}
        this->Qlen = nBufCount;


        for (int i = 0; i< nBufCount; i++)
        {
            Queue[i].nFrameNum = 0;
            Queue[i].nHeight = 0;
            Queue[i].nWidth = 0;
            Queue[i].nFrameLen = 0;          
            Queue[i].pData = (unsigned char*)malloc(DefaultImagelen);
            if(NULL ==  Queue[i].pData)
            {
                return MV_E_RESOURCE;
            }
        }

        return 0;

    }

    // 数据放入队列
    int push(int nFrameNum, int nHeight, int nWidth, unsigned char *pData, int nFrameLen)
    {
        g_mutex.lock();

        if (size==Qlen)
        {
            g_mutex.unlock();
            return MV_E_BUFOVER;
        }


        size++;
        Queue[end].nFrameNum = nFrameNum;
        Queue[end].nHeight = nHeight;
        Queue[end].nWidth = nWidth;
        Queue[end].nFrameLen = nFrameLen;

        if (NULL !=  Queue[end].pData  && NULL != pData)
        {
            memcpy(Queue[end].pData, pData, nFrameLen);
        }

        end = end == Qlen - 1 ? 0 : end + 1;
        g_mutex.unlock();
        return 0;
    }

    // 数据从队列中取出
    int  poll(int &nFrameNum, int &nHeight, int &nWidth, unsigned char *pData, int &nFrameLen)
    {

        g_mutex.lock();
		

        if (size == 0)
        {
            g_mutex.unlock();
            return  MV_E_NODATA;
        }


        nFrameNum =Queue[start].nFrameNum;
        nHeight =Queue[start].nHeight;
        nWidth =Queue[start].nWidth;
        nFrameLen =Queue[start].nFrameLen;

        if (NULL !=  pData && NULL != Queue[start].pData)
        {
            memcpy( pData,Queue[start].pData, nFrameLen);
        }

        size--;
        start = start == Qlen - 1 ? 0 : start + 1;

        g_mutex.unlock();


        return 0;
    }


private:
    stImageNode *Queue;
    int size;
    int start;
    int end;
    int Qlen;

	std::mutex g_mutex; //互斥锁
};


ArrayQueue * m_queue = NULL;      //线程通信队列


// 等待用户输入enter键来结束取流或结束程序
// wait for user to input enter to stop grabbing or end the sample program
void PressEnterToExit(void)
{
    int c;
    while ( (c = getchar()) != '\n' && c != EOF );
    fprintf( stderr, "\nPress enter to exit.\n");
    while( getchar() != '\n');
    g_bExit = true;
    sleep(1);
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
        printf("Not support.\n");
    }

    return true;
}

void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{	
	if (NULL ==  pFrameInfo ||  NULL ==  pData)
	{
        printf("ImageCallBackEx Input Param invalid.\n");
        return;
    }

    int nRet = MV_OK;
    nRet = m_queue->push(pFrameInfo->nFrameNum, pFrameInfo->nWidth, pFrameInfo->nHeight,pData, pFrameInfo->nFrameLen);//固定数组实现队列故是10 9 8 7 6 5 4 3 2 1
    if (MV_OK != nRet)
    {
        printf("Add Image [%d] to list  failed. \r\n", pFrameInfo->nFrameNum);
    }
    else
    {
        printf("Add Image [%d] to list  success. \r\n", pFrameInfo->nFrameNum);
    }

    return;
}

static void* WorkThread(void* pUser)
{
	int nRet = MV_OK;

    int nWidth = 0;
    int nHeight = 0;
    int nFrameLen= 0;
    int nFrameNum = 0;
    unsigned char *pOutData = (unsigned char *) malloc(m_nImageSize);
    if (NULL == pOutData)
    {
        printf("WorkThread malloc size [%d] failed. \r\n",m_nImageSize);
        return  NULL;
    }
    printf("WorkThread Begin . \r\n");


	while(true != g_bExit)
	{

        nWidth = 0;
        nHeight = 0;
        nFrameLen = 0;
        nFrameNum = 0;

        nRet = m_queue->poll(nFrameNum,nHeight,nWidth,pOutData,nFrameLen);
        if (MV_OK != nRet)
        {
            printf("Poll failed, maybe no data. \r\n");
            usleep(1*1000*2);//根据实际情况调节;
            continue;
        }
        else
        {
            printf("Get nWidth [%d] nHeight [%d] nFrameNum [%d]  \r\n",nWidth,nHeight,nFrameNum);

             //根据实际场景需求，对图像进行 处理   
#if 0
            FILE* fp = NULL;
            char szFileName[256] = {0};
            sprintf(szFileName, "Image_%d_width_%d_height_%d_Len_%d.raw",nFrameNum,nWidth,nHeight,nFrameLen);
            fp = fopen(szFileName, "wb+");
            if (fp == NULL)
            {
                return MV_E_RESOURCE;
            }
            fwrite(pOutData, 1, nFrameLen, fp);
            fclose(fp);
            fp = NULL;
#endif
        }
	}

    printf("WorkThread exit . \r\n");

    if (pOutData)
    {
        free(pOutData);
        pOutData = NULL;
    }

	return NULL;
}

int main()
{
    int nRet = MV_OK;
    void* handle = NULL;
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

        // 枚举设备
        // enum device
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

        printf("Please Intput camera index: ");
        unsigned int nIndex = 0;
        scanf("%d", &nIndex);

        if (nIndex >= stDeviceList.nDeviceNum)
        {
            printf("Intput error!\n");
            break;
        }

        // 选择设备并创建句柄
        // select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet);
            break;
        }

        // 打开设备
        // open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_OpenDevice fail! nRet [%x]\n", nRet);
            break;
        }
		
	    int nWidth = 0;
        int nHeight = 0;

        MVCC_INTVALUE_EX stIntEx  = {0}; 
        nRet = MV_CC_GetIntValueEx(handle, "Width", &stIntEx);
        if (MV_OK != nRet)
        {
            printf("Get IntValue fail! nRet [0x%x]\n", nRet);
            break;
        }
        nWidth = stIntEx.nCurValue;

        
        memset(&stIntEx, 0, sizeof(MVCC_INTVALUE_EX));
        nRet = MV_CC_GetIntValueEx(handle, "Height", &stIntEx);
        if (MV_OK != nRet)
        {
            printf("Get IntValue fail! nRet [0x%x]\n", nRet);
            break;
        }
        nHeight = stIntEx.nCurValue;
             
        
        nRet = MV_CC_GetIntValueEx(handle, "PayloadSize", &stIntEx);
		if (MV_OK != nRet)
		{
			printf("Get IntValue fail! nRet [0x%x], use [%d] replace.\n", nRet, nHeight*nWidth*3);
            m_nImageSize =  nHeight*nWidth*3; 
		}
        else
        {
           m_nImageSize =  stIntEx.nCurValue;
        }
		//初始化队列
		m_queue = new (std::nothrow)ArrayQueue();
        if (m_queue==NULL)
		{
			printf("m_queue is null! \n");
			break;
		}
		nRet = m_queue->Init(Max_Count,m_nImageSize);
		if (MV_OK != nRet)
		{
			printf("ArrayQueue init fail! nRet [0x%x]\n", nRet);
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
                    printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
            }
        }

		
        nRet = MV_CC_SetBoolValue(handle, "AcquisitionFrameRateEnable", false);
        if (MV_OK != nRet)
        {
            printf("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
            break;
        }
		
        // 设置触发模式为on
        // set trigger mode as on
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            printf("MV_CC_SetTriggerMode fail! nRet [%x]\n", nRet);
            break;
        }

        // 注册抓图回调
        // register image callback
        nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_RegisterImageCallBackEx fail! nRet [%x]\n", nRet);
            break; 
        }

        pthread_t nThreadID;
        nRet = pthread_create(&nThreadID, NULL ,WorkThread , handle);
        if (nRet != 0)
        {
            printf("thread create failed.ret = %d\n",nRet);
            break;
        }

        // 开始取流
        // start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
            break;
        }

        PressEnterToExit();

        // 停止取流
        // end grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);
            break;
        }

        // 注销抓图回调
        // unregister image callback
        nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
        if (MV_OK != nRet)
        {
            printf("UnRegisterImageCallBackEx fail! nRet [%x]\n", nRet);
            break; 
        }

        // 关闭设备
        // close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_CloseDevice fail! nRet [%x]\n", nRet);
            break;
        }

        // 销毁句柄
        // destroy handle
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_DestroyHandle fail! nRet [%x]\n", nRet);
            break;
        }
        handle = NULL;

    } while (0);

	//释放资源
	if (m_queue)
    {
        delete m_queue;
        m_queue = NULL;
    }
	
	printf("free buffer done\n");

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
