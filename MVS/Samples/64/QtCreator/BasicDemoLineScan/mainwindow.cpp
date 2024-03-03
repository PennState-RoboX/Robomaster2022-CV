#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QStyleFactory>
#include <QTextCodec>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    m_pcMyCamera = NULL;
    m_bGrabbing = false;
    m_hWnd = (void*)ui->DisplayWidget->winId();
    m_nSaveImageBufSize=0;
    m_bOpenDevice=false;
    m_bGrabbing=false;
    m_pSaveImageBuf=NULL;

    pthread_mutex_init(&m_hSaveImageMux,NULL); /*初始化函数*/
}

MainWindow::~MainWindow()
{
    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    delete ui;
}

// ch:按钮使能 | en:Enable control
void MainWindow::EnableControls(bool bIsCameraReady)
{
    ui->OpenButton->setEnabled(m_bOpenDevice ? false : (bIsCameraReady ? true : false));
    ui->CloseButton->setEnabled((m_bOpenDevice && bIsCameraReady) ? true : false);
    ui->StartGrabbingButton->setEnabled((m_bGrabbing && bIsCameraReady) ? false : (m_bOpenDevice ? true : false));
    ui->StopGrabbingButton->setEnabled(m_bGrabbing ? true : false);
    ui->SoftwareOnceButton->setEnabled((m_bGrabbing && m_bTriggerModeCheck) ? true : false);
    ui->SaveBmpButton->setEnabled(m_bGrabbing ? true : false);
    ui->SaveTiffButton->setEnabled(m_bGrabbing ? true : false);
    ui->SavePngButton->setEnabled(m_bGrabbing ? true : false);
    ui->SaveJpgButton->setEnabled(m_bGrabbing ? true : false);
    ui->ExposureTimeLineEdit->setEnabled(m_bOpenDevice ? true : false);
    ui->PreampGainLineEdit->setEnabled(m_bOpenDevice ? true : false);
    ui->AcquisitionLineRateLineEdit->setEnabled((m_bOpenDevice && m_bAcquisitionLineRate) ? true : false);
    ui->ResultingLineRateLineEdit->setEnabled(m_bOpenDevice ? true : false);
    ui->SelchangeTriggerselCombo->setEnabled(m_bOpenDevice ? true : false);
    ui->SelchangeTriggerswitchCombo->setEnabled(m_bOpenDevice ? true : false);
    ui->GetParameterButton->setEnabled(m_bOpenDevice ? true : false);
    ui->SetParameterButton->setEnabled(m_bOpenDevice ? true : false);
    ui->SelchangeTriggersourceCombo->setEnabled(m_bOpenDevice ? true : false);
    ui->SelchangePixelformatCombo->setEnabled((m_bOpenDevice &&!m_bGrabbing) ? true : false);
    ui->SelchangeImageCompressionModeCombo->setEnabled((m_bOpenDevice && m_bHBMode &&!m_bGrabbing)? true : false);
    ui->SelchangePreampgainCombo->setEnabled((m_bOpenDevice && m_bPreampGain) ? true : false);
    ui->AcquisitionLineRateEnableCheckBox->setEnabled((m_bOpenDevice &&m_bAcquisitionLineRate)? true : false);
    ui->ResultingLineRateLineEdit->setEnabled(false);

    if (!m_bOpenDevice)
    {
        ui->AcquisitionLineRateEnableCheckBox->setChecked(false);
        ui->ExposureTimeLineEdit->setText(QString::number(0,10));
        ui->PreampGainLineEdit->setText(QString::number(0,10));
        ui->AcquisitionLineRateLineEdit->setText(QString::number(0,10));
        ui->ResultingLineRateLineEdit->setText(QString::number(0,10));

    }
}

//ch:获取触发选项  | en:Get Trigger Selector
int MainWindow::GetTriggerSelector()
{
    MVCC_ENUMVALUE stEnumTriggerSelectorValue = { 0 };
    MVCC_ENUMENTRY stEnumTriggerSelectorEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSelector", &stEnumTriggerSelectorValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->SelchangeTriggerselCombo->clear();
    for (int i = 0; i < stEnumTriggerSelectorValue.nSupportedNum; i++)
    {
        memset(&stEnumTriggerSelectorEntry, 0, sizeof(stEnumTriggerSelectorEntry));
        stEnumTriggerSelectorEntry.nValue = stEnumTriggerSelectorValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TriggerSelector", &stEnumTriggerSelectorEntry);

        ui->SelchangeTriggerselCombo->addItem((QString)stEnumTriggerSelectorEntry.chSymbolic);

    }

    for (int i = 0; i < stEnumTriggerSelectorValue.nSupportedNum; i++)
    {
        if (stEnumTriggerSelectorValue.nCurValue == stEnumTriggerSelectorValue.nSupportValue[i])
        {
            m_nTriggerSelector = i;
            ui->SelchangeTriggerselCombo->setCurrentIndex(m_nTriggerSelector);
        }
    }

    return MV_OK;
}

// ch:获取触发模式 | en:Get Trigger Mode
int MainWindow::GetTriggerMode()
{
    MVCC_ENUMVALUE stEnumTriggerModeValue = { 0 };
    MVCC_ENUMENTRY stEnumTriggerModeEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TriggerMode", &stEnumTriggerModeValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->SelchangeTriggerswitchCombo->clear();
    for (int i = 0; i < stEnumTriggerModeValue.nSupportedNum; i++)
    {
        memset(&stEnumTriggerModeEntry, 0, sizeof(stEnumTriggerModeEntry));
        stEnumTriggerModeEntry.nValue = stEnumTriggerModeValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TriggerMode", &stEnumTriggerModeEntry);
        ui->SelchangeTriggerswitchCombo->addItem((QString)stEnumTriggerModeEntry.chSymbolic);
    }

    for (int i = 0; i < stEnumTriggerModeValue.nSupportedNum; i++)
    {
        if (stEnumTriggerModeValue.nCurValue == stEnumTriggerModeValue.nSupportValue[i])
        {
            m_nTriggerMode = i;
            ui->SelchangeTriggerselCombo->setCurrentIndex(m_nTriggerMode);
        }
    }

    return MV_OK;
}

// ch:获取触发源 | en:Get Trigger Source
int MainWindow::GetTriggerSource()
{
    MVCC_ENUMVALUE stEnumTriggerSourceValue = { 0 };
    MVCC_ENUMENTRY stEnumTriggerSourceEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSource", &stEnumTriggerSourceValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->SelchangeTriggersourceCombo->clear();
    m_mapTriggerSource.clear();

    for (int i = 0; i < stEnumTriggerSourceValue.nSupportedNum; i++)
    {
        memset(&stEnumTriggerSourceEntry, 0, sizeof(stEnumTriggerSourceEntry));
        stEnumTriggerSourceEntry.nValue = stEnumTriggerSourceValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TriggerSource", &stEnumTriggerSourceEntry);

        ui->SelchangeTriggersourceCombo->addItem((QString)stEnumTriggerSourceEntry.chSymbolic);

        m_mapTriggerSource.insert((QString)stEnumTriggerSourceEntry.chSymbolic, stEnumTriggerSourceEntry.nValue);
    }

    for (int i = 0; i < stEnumTriggerSourceValue.nSupportedNum; i++)
    {
        if (stEnumTriggerSourceValue.nCurValue == stEnumTriggerSourceValue.nSupportValue[i])
        {
            m_nTriggerSource = i;
            ui->SelchangeTriggersourceCombo->setCurrentIndex(m_nTriggerSource);
        }
    }

    QString strTriggerSource=ui->SelchangeTriggersourceCombo->currentText();
    QString strTriggerSelector=ui->SelchangeTriggerselCombo->currentText();

    QString strTriggerMode = ui->SelchangeTriggerswitchCombo->currentText();
    if (STR_FRAMEBURSTSTART == strTriggerSelector &&"On" == strTriggerMode && STR_SOFTWARE == strTriggerSource)
    {
        m_bTriggerModeCheck = true;
    }

    EnableControls(true);
    return MV_OK;
}

// ch:获取曝光时间 | en:Get Exposure Time
int MainWindow::GetExposureTime()
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("ExposureTime", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->ExposureTimeLineEdit->setText(QString::number(stFloatValue.fCurValue,'f',4));

    return MV_OK;
}

// ch:获取增益 | en:Get Gain
int MainWindow::GetDigitalShiftGain()
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("DigitalShift", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->PreampGainLineEdit->setText(QString::number(stFloatValue.fCurValue,'f',4));

    return MV_OK;
}

// ch:获取模拟增益  | en:Get PreampGain
int MainWindow::GetPreampGain()
{
    MVCC_ENUMVALUE stEnumPreampGainValue = { 0 };
    MVCC_ENUMENTRY stEnumPreampGainEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("PreampGain", &stEnumPreampGainValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->SelchangePreampgainCombo->clear();
    m_mapPreampGain.clear();
    for (int i = 0; i < stEnumPreampGainValue.nSupportedNum; i++)
    {
        memset(&stEnumPreampGainEntry, 0, sizeof(stEnumPreampGainEntry));
        stEnumPreampGainEntry.nValue = stEnumPreampGainValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("PreampGain", &stEnumPreampGainEntry);

        ui->SelchangePreampgainCombo->addItem((QString)stEnumPreampGainEntry.chSymbolic);

        m_mapPreampGain.insert((QString)stEnumPreampGainEntry.chSymbolic, stEnumPreampGainEntry.nValue);
    }

    for (int i = 0; i < stEnumPreampGainValue.nSupportedNum; i++)
    {
        if (stEnumPreampGainValue.nCurValue == stEnumPreampGainValue.nSupportValue[i])
        {
            m_nPreampGain = i;
            ui->SelchangePreampgainCombo->setCurrentIndex(m_nPreampGain);
        }
    }

    m_bPreampGain = true;

    return MV_OK;
}

int MainWindow::GetAcquisitionLineRateEnable()
{
    bool bAcquisitionLineRateEnable = false;
    int nRet = m_pcMyCamera->GetBoolValue("AcquisitionLineRateEnable", &bAcquisitionLineRateEnable);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if (true == bAcquisitionLineRateEnable)
    {
        ui->AcquisitionLineRateEnableCheckBox->setChecked(true);
    }
    else
    {
        ui->AcquisitionLineRateEnableCheckBox->setChecked(false);
    }

    return MV_OK;
}

// ch:获取行频  | en:Get Acquisition LineRate
int MainWindow::GetAcquisitionLineRate()
{
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("AcquisitionLineRate", &stIntValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->AcquisitionLineRateLineEdit->setText(QString::number(stIntValue.nCurValue,10));

    m_bAcquisitionLineRate = true;

    return MV_OK;
}

int MainWindow::GetResultingLineRate()
{
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("ResultingLineRate", &stIntValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->ResultingLineRateLineEdit->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int MainWindow::GetPixelFormat()
{
    m_mapPixelFormat.clear();
    MVCC_ENUMVALUE stEnumPixelFormatValue = { 0 };
    MVCC_ENUMENTRY stEnumPixelFormatEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("PixelFormat", &stEnumPixelFormatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->SelchangePixelformatCombo->clear();
    for (int i = 0; i < stEnumPixelFormatValue.nSupportedNum; i++)
    {
        memset(&stEnumPixelFormatEntry, 0, sizeof(stEnumPixelFormatEntry));
        stEnumPixelFormatEntry.nValue = stEnumPixelFormatValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("PixelFormat", &stEnumPixelFormatEntry);

        ui->SelchangePixelformatCombo->addItem((QString)stEnumPixelFormatEntry.chSymbolic);

        m_mapPixelFormat.insert((QString)stEnumPixelFormatEntry.chSymbolic, stEnumPixelFormatEntry.nValue);
    }

    for (int i = 0; i < stEnumPixelFormatValue.nSupportedNum; i++)
    {
        if (stEnumPixelFormatValue.nCurValue == stEnumPixelFormatValue.nSupportValue[i])
        {
            m_nPixelFormat = i;
            ui->SelchangePixelformatCombo->setCurrentIndex(m_nPixelFormat);
        }
    }

    return MV_OK;
}

//ch:获取图像压缩模式  | en:Get Image Compression Mode
int MainWindow::GetImageCompressionMode()
{
    MVCC_ENUMVALUE stEnumImageCompressionModeValue = { 0 };
    MVCC_ENUMENTRY stEnumImageCompressionModeEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("ImageCompressionMode", &stEnumImageCompressionModeValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->SelchangeImageCompressionModeCombo->clear();
    for (int i = 0; i < stEnumImageCompressionModeValue.nSupportedNum; i++)
    {
        memset(&stEnumImageCompressionModeEntry, 0, sizeof(stEnumImageCompressionModeEntry));
        stEnumImageCompressionModeEntry.nValue = stEnumImageCompressionModeValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("ImageCompressionMode", &stEnumImageCompressionModeEntry);

        ui->SelchangeImageCompressionModeCombo->addItem((QString)stEnumImageCompressionModeEntry.chSymbolic);
    }

    for (int i = 0; i < stEnumImageCompressionModeValue.nSupportedNum; i++)
    {
        if (stEnumImageCompressionModeValue.nCurValue == stEnumImageCompressionModeValue.nSupportValue[i])
        {
            m_nImageCompressionMode = i;
            ui->SelchangeImageCompressionModeCombo->setCurrentIndex(m_nImageCompressionMode);
        }
    }

    m_bHBMode = true;

    return MV_OK;
}


// ch:显示错误信息 | en:Show error message
void MainWindow::ShowErrorMsg(QString csMessage, unsigned int nErrorNum)
{
    QString errorMsg = csMessage;
    if (nErrorNum != 0)
    {
        QString TempMsg;
        TempMsg.sprintf(": Error = %x: ", nErrorNum);
        errorMsg += TempMsg;
    }

    switch(nErrorNum)
    {
    case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
    case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
    case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
    case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
    case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
    case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
    case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
    case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
    case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
    case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
    case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
    case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
    case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
    case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
    case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
    case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
    case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
    }

    QMessageBox::information(NULL, "PROMPT", errorMsg);
}

void MainWindow::on_EnumButton_clicked()
{
    ui->EnumCombo->clear();
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	ui->EnumCombo->setStyle(QStyleFactory::create("Windows"));
    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        return;
    }

    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }
		char strUserName[256] = {0};
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName) != 0)
            {
                snprintf(strUserName, 256, "[%d]GigE:   %s (%s) (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName,
                         pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber, nIp1, nIp2, nIp3, nIp4);
            }
            else
            {
                snprintf(strUserName, 256, "[%d]GigE:   %s (%s) (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
                         pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber, nIp1, nIp2, nIp3, nIp4);
            }
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
            {
                snprintf(strUserName, 256, "[%d]UsbV3:  %s (%s)", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName,
                         pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
            else
            {
                snprintf(strUserName, 256, "[%d]UsbV3:  %s (%s)", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName,
                         pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
        }
        else
        {
            ShowErrorMsg("Unknown device enumerated", 0);
        }
        ui->EnumCombo->addItem(QString::fromLocal8Bit(strUserName));
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg("No device", 0);
        return;
    }
    ui->EnumCombo->setCurrentIndex(0);

    EnableControls(true);
}

void MainWindow::on_OpenButton_clicked()
{
    int nIndex = ui->EnumCombo->currentIndex();
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg("Please select device", 0);
        return;
    }

    // ch:由设备信息创建设备实例 | en:Device instance created by device information
    if (NULL == m_stDevList.pDeviceInfo[nIndex])
    {
        ShowErrorMsg("Device does not exist", 0);
        return;
    }

    if(m_pcMyCamera == NULL)
    {
        m_pcMyCamera = new CMvCamera;
        if (NULL == m_pcMyCamera)
        {
            return;
        }
    }

    int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        ShowErrorMsg("Open Fail", nRet);
        return;
    }

    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
    if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        unsigned int nPacketSize = 0;
        nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
        if (nRet == MV_OK)
        {
            nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize",nPacketSize);
            if(nRet != MV_OK)
            {
                ShowErrorMsg("Warning: Set Packet Size fail!", nRet);
            }
        }
        else
        {
            ShowErrorMsg("Warning: Get Packet Size fail!", nRet);
        }
    }

    m_bOpenDevice = true;
    EnableControls(true);
    ui->ExposureTimeLineEdit->setEnabled(m_bOpenDevice ? true : false);
    on_GetParameterButton_clicked(); // ch:获取参数 | en:Get Parameter
    
    nRet = GetPixelFormat();
    if (nRet != MV_OK)
    {
        ui->SelchangePixelformatCombo->setEnabled(false);
    }

    nRet = GetImageCompressionMode();
    if (nRet != MV_OK)
    {
        ui->SelchangeImageCompressionModeCombo->setEnabled(false);
    }
    
    EnableControls(true);
}

void MainWindow::on_GetParameterButton_clicked()
{
    int nRet = GetTriggerSelector();
    if (nRet != MV_OK)
    {
        ui->SelchangeTriggerselCombo->setEnabled(false);
    }

    nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        ui->SelchangeTriggerswitchCombo->setEnabled(false);
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        ui->SelchangeTriggersourceCombo->setEnabled(false);
    }

    nRet = GetExposureTime();
    if (nRet != MV_OK)
    {
        ui->ExposureTimeLineEdit->setEnabled(false);
    }

    nRet = GetDigitalShiftGain();
    if (nRet != MV_OK)
    {
        ui->PreampGainLineEdit->setEnabled(false);
    }

    nRet = GetPreampGain();
    if (nRet != MV_OK)
    {
        ui->SelchangePreampgainCombo->setEnabled(false);
    }

    nRet = GetAcquisitionLineRateEnable();
    if (nRet != MV_OK)
    {
        ui->AcquisitionLineRateEnableCheckBox->setEnabled(false);
    }

    nRet = GetAcquisitionLineRate();
    if (nRet != MV_OK)
    {
        ui->AcquisitionLineRateLineEdit->setEnabled(false);
    }

    nRet = GetResultingLineRate();
    if (nRet != MV_OK)
    {
        ui->ResultingLineRateLineEdit->setEnabled(false);
    }
}

void __stdcall MainWindow::ImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{

    if(pUser)
    {
        MainWindow *pMainWindow = (MainWindow*)pUser;
        pMainWindow->ImageCallBackInner(pData, pFrameInfo);
    }
}

void MainWindow::ImageCallBackInner(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo)
{
    //用于保存图片  | en:save image
    pthread_mutex_lock(&m_hSaveImageMux);/*加锁*/
    if (NULL == m_pSaveImageBuf || pFrameInfo->nFrameLen > m_nSaveImageBufSize)
    {
        if (m_pSaveImageBuf)
        {
            free(m_pSaveImageBuf);
            m_pSaveImageBuf = NULL;
        }

        m_pSaveImageBuf = (unsigned char *)malloc(sizeof(unsigned char) * pFrameInfo->nFrameLen);
        if (m_pSaveImageBuf == NULL)
        {
            pthread_mutex_unlock(&m_hSaveImageMux);/*解锁*/
            return;
        }
        m_nSaveImageBufSize = pFrameInfo->nFrameLen;
    }

    memcpy(m_pSaveImageBuf, pData, pFrameInfo->nFrameLen);

    memcpy(&m_stImageInfo, pFrameInfo, sizeof(MV_FRAME_OUT_INFO_EX));
    pthread_mutex_unlock(&m_hSaveImageMux);/*解锁*/

    MV_DISPLAY_FRAME_INFO stDisplayInfo;
    memset(&stDisplayInfo, 0, sizeof(MV_DISPLAY_FRAME_INFO));

    stDisplayInfo.hWnd = m_hWnd;
    stDisplayInfo.pData = pData;
    stDisplayInfo.nDataLen = pFrameInfo->nFrameLen;
    stDisplayInfo.nWidth = pFrameInfo->nWidth;
    stDisplayInfo.nHeight = pFrameInfo->nHeight;
    stDisplayInfo.enPixelType = pFrameInfo->enPixelType;

    m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);
}

void MainWindow::on_StartGrabbingButton_clicked()
{
    if (false == m_bOpenDevice || true == m_bGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }
    m_pcMyCamera->RegisterImageCallBack(ImageCallBack, this);

    int nRet = m_pcMyCamera->StartGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Start grabbing fail", nRet);
        return;
    }
    m_bGrabbing = true;
    EnableControls(true);

    QString QstrTriggerSource = ui->SelchangeTriggersourceCombo->currentText();

    if (STR_SOFTWARE == QstrTriggerSource && m_bTriggerModeCheck == true)
    {
        ui->SoftwareOnceButton->setEnabled(true);
    }
    else
    {
        ui->SoftwareOnceButton->setEnabled(false);
    }
}

// ch:关闭设备 | en:Close Device
int MainWindow::CloseDevice()
{
    if(true == m_bGrabbing)
    {
        int nRet = m_pcMyCamera->StopGrabbing();
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Stop grabbing fail", nRet);
            return nRet;
        }
         m_bGrabbing = false;
    }

    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    m_bGrabbing = false;
    m_bOpenDevice = false;

    if (m_pSaveImageBuf)
    {
        free(m_pSaveImageBuf);
        m_pSaveImageBuf = NULL;
    }
    m_nSaveImageBufSize = 0;

    return MV_OK;
}

void MainWindow::on_CloseButton_clicked()
{
    CloseDevice();
    m_bTriggerModeCheck = false;
    m_bAcquisitionLineRate = false;
    m_bPreampGain = false;
    m_bHBMode = false;
    EnableControls(true);

}

void MainWindow::on_StopGrabbingButton_clicked()
{
    if (false == m_bOpenDevice || false == m_bGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }
    int nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Stop grabbing fail", nRet);
        return;
    }
    m_bGrabbing = false;
    EnableControls(true);
    on_GetParameterButton_clicked(); // ch:获取参数 | en:Get Parameter
}

// ch:设置曝光时间 | en:Set Exposure Time
int MainWindow::SetExposureTime()
{
    m_pcMyCamera->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);

    return m_pcMyCamera->SetFloatValue("ExposureTime", (float)m_dExposureEdit);
}

// ch:数字增益 | en:Digital Shift
int MainWindow::SetDigitalShiftGain()
{
    // ch:设置增益前先把增益使能开关打开，失败无需返回
    //en:Set Gain after Auto Gain is turned off, this failure does not need to return
    m_pcMyCamera->SetBoolValue("DigitalShiftEnable", true);

    return m_pcMyCamera->SetFloatValue("DigitalShift", (float)m_dDigitalShiftGainEdit);
}

// ch:设置行频   | en:set Acquisition LineRate
int MainWindow::SetAcquisitionLineRate()
{
    return m_pcMyCamera->SetIntValue("AcquisitionLineRate", (int)m_nAcquisitionLineRateEdit);
}

void MainWindow::on_SetParameterButton_clicked()
{
    bool bIsSetSucceed = true;
    int nRet = SetExposureTime();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg("Set Exposure Time Fail", nRet);
    }
    nRet = SetDigitalShiftGain();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg("Set Digital Shift Fail", nRet);
    }

    if (true == m_bAcquisitionLineRate)
    {
        nRet = SetAcquisitionLineRate();
        if (nRet != MV_OK)
        {
            bIsSetSucceed = false;
            ShowErrorMsg("Set Acquisition Line Rate Fail", nRet);
        }
    }

    if (true == bIsSetSucceed)
    {
        ShowErrorMsg("Set Parameter Succeed", nRet);
    }
}

void MainWindow::on_SelchangeTriggerselCombo_currentTextChanged(const QString &arg1)
{
    if (STR_FRAMEBURSTSTART == arg1)
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSelector", FRAMEBURSTSTART);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set TriggerSelector FrameBurstStart fail", nRet);
            return;
        }

        QString QStrTriggerMode = ui->SelchangeTriggerswitchCombo->currentText();

        QString QStrTriggerSource = ui->SelchangeTriggersourceCombo->currentText();

        if ("On" == QStrTriggerMode && STR_SOFTWARE == QStrTriggerSource)
        {
            m_bTriggerModeCheck = true;
        }
    }
    else if (arg1 == "LineStart")
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSelector", LINESTART);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set TriggerSelector LineStart fail", nRet);
            return;
        }
        m_bTriggerModeCheck = false;
    }

    int nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        ShowErrorMsg("Get Trigger Source Fail", nRet);
        return;
    }

    EnableControls(true);
}

void MainWindow::on_SelchangeTriggerswitchCombo_currentTextChanged(const QString &arg1)
{
    if ("On" == arg1)
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_ON);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Trigger Mode fail", nRet);
            return;
        }

        QString QStrTriggerSelector = ui->SelchangeTriggerselCombo->currentText();

        QString QStrTriggerSource = ui->SelchangeTriggersourceCombo->currentText();
        if (STR_FRAMEBURSTSTART == QStrTriggerSelector && STR_SOFTWARE == QStrTriggerSource)
        {
            m_bTriggerModeCheck = true;
        }
    }
    else if ("Off" == arg1)
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Trigger Mode fail", nRet);
            return;
        }
        m_bTriggerModeCheck = false;
    }

    EnableControls(true);
}

void MainWindow::on_SelchangeTriggersourceCombo_currentTextChanged(const QString &arg1)
{
    m_bTriggerModeCheck = false;

    if (STR_SOFTWARE == arg1)
    {
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Trigger Source fail", nRet);
            return;
        }

        QString QStrTriggerSelector = ui->SelchangeTriggerselCombo->currentText();

        QString QStrTriggerMode = ui->SelchangeTriggerswitchCombo->currentText();
        if (STR_FRAMEBURSTSTART == QStrTriggerSelector && "On" == QStrTriggerMode)
        {
            m_bTriggerModeCheck = true;
        }
    }


    for (QMap<QString, int>::iterator it = m_mapTriggerSource.begin(); it != m_mapTriggerSource.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set TriggerSource fail", nRet);
                return;
            }
            break;
        }
    }

    EnableControls(true);
}

void MainWindow::on_SoftwareOnceButton_clicked()
{
    if (true != m_bGrabbing)
    {
        return;
    }

    m_pcMyCamera->CommandExecute("TriggerSoftware");
}


void MainWindow::on_SelchangePixelformatCombo_currentTextChanged(const QString &arg1)
{
    for (QMap<QString, int>::iterator it = m_mapPixelFormat.begin(); it != m_mapPixelFormat.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("PixelFormat", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set PixelFormat fail", nRet);
                return;
            }
            break;
        }
    }

    if (true == m_bHBMode)
    {
        int nRet = GetImageCompressionMode();
        if (nRet != MV_OK)
        {
            ShowErrorMsg("Get Image Compression Mode Fail", nRet);
            return;
        }
    }
}

void MainWindow::on_SelchangeImageCompressionModeCombo_currentTextChanged(const QString &arg1)
{
    if ("Off" == arg1)
    {
        int nRet = m_pcMyCamera->SetEnumValue("ImageCompressionMode",IMAGE_COMPRESSION_MODE_OFF);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Image Compression Mode fail", nRet);
            return;
        }
    }
    else if ("HB" == arg1)
    {
        int nRet = m_pcMyCamera->SetEnumValue("ImageCompressionMode", IMAGE_COMPRESSION_MODE_HB);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Image Compression Mode fail", nRet);
            return;
        }
    }
}


void MainWindow::on_ExposureTimeLineEdit_editingFinished()
{
    QString  QStrExposureTime = ui->ExposureTimeLineEdit->text();
    m_dExposureEdit=QStrExposureTime.toFloat();
    //m_pcMyCamera->SetFloatValue("ExposureTime", (float)QStrExposureTime.toDouble());
}

void MainWindow::on_SelchangePreampgainCombo_currentTextChanged(const QString &arg1)
{
    for (QMap<QString, int>::iterator it = m_mapPreampGain.begin(); it != m_mapPreampGain.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("PreampGain", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set PreampGain fail", nRet);
                return;
            }
            break;
        }
    }
}

void MainWindow::on_PreampGainLineEdit_editingFinished()
{
    QString  QStrPreampGain = ui->PreampGainLineEdit->text();
    m_dDigitalShiftGainEdit=QStrPreampGain.toFloat();
}

void MainWindow::on_AcquisitionLineRateLineEdit_editingFinished()
{
    QString  QStrAcquisitionLineRate = ui->AcquisitionLineRateLineEdit->text();
    m_nAcquisitionLineRateEdit=QStrAcquisitionLineRate.toInt();

}

void MainWindow::on_AcquisitionLineRateEnableCheckBox_clicked(bool checked)
{
    if (true ==  checked)
    {
        int nRet = m_pcMyCamera->SetBoolValue("AcquisitionLineRateEnable", true);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Acquisition LineRate Enable fail", nRet);
            return;
        }
    }
    else
    {
        int nRet = m_pcMyCamera->SetBoolValue("AcquisitionLineRateEnable", false);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set Acquisition LineRate Enable fail", nRet);
            return;
        }
    }
}

// ch:保存图片 | en:Save Image
int MainWindow::SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType)
{
    MV_SAVE_IMAGE_TO_FILE_PARAM_EX stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMAGE_TO_FILE_PARAM_EX));

    pthread_mutex_lock(&m_hSaveImageMux);/*加锁*/
    if (m_pSaveImageBuf == NULL || m_stImageInfo.enPixelType == 0)
    {
        pthread_mutex_unlock(&m_hSaveImageMux);/*解锁*/
        return MV_E_NODATA;
    }

    stSaveFileParam.enImageType = enSaveImageType; // ch:需要保存的图像类型 | en:Image format to save
    stSaveFileParam.enPixelType = m_stImageInfo.enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
    stSaveFileParam.nWidth      = m_stImageInfo.nWidth;         // ch:相机对应的宽 | en:Width
    stSaveFileParam.nHeight     = m_stImageInfo.nHeight;          // ch:相机对应的高 | en:Height
    stSaveFileParam.nDataLen    = m_stImageInfo.nFrameLen;
    stSaveFileParam.pData       = m_pSaveImageBuf;
    stSaveFileParam.pcImagePath=(char*)malloc(256);
    memset(stSaveFileParam.pcImagePath,0,256);
    stSaveFileParam.iMethodValue = 0;

    // ch:jpg图像质量范围为(50-99], png图像质量范围为[0-9] | en:jpg image nQuality range is (50-99], png image nQuality range is [0-9]
    if (MV_Image_Bmp == stSaveFileParam.enImageType)
    {
        QString QstrSaveFileParam=QString::asprintf("Image_w%d_h%d_fn%03d.bmp",stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
        memcpy(stSaveFileParam.pcImagePath,QstrSaveFileParam.toLatin1().data(),strlen(QstrSaveFileParam.toLatin1().data()));
    }
    else if (MV_Image_Jpeg == stSaveFileParam.enImageType)
    {
        QString QstrSaveFileParam=QString::asprintf("Image_w%d_h%d_fn%03d.jpg",stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
        memcpy(stSaveFileParam.pcImagePath,QstrSaveFileParam.toLatin1().data(),strlen(QstrSaveFileParam.toLatin1().data()));
        stSaveFileParam.nQuality = 80;
    }
    else if (MV_Image_Tif == stSaveFileParam.enImageType)
    {
        QString QstrSaveFileParam=QString::asprintf("Image_w%d_h%d_fn%03d.tif",stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
        memcpy(stSaveFileParam.pcImagePath,QstrSaveFileParam.toLatin1().data(),strlen(QstrSaveFileParam.toLatin1().data()));
    }
    else if (MV_Image_Png == stSaveFileParam.enImageType)
    {
        stSaveFileParam.nQuality = 8;
        QString QstrSaveFileParam=QString::asprintf("Image_w%d_h%d_fn%03d.png",stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo.nFrameNum);
        memcpy(stSaveFileParam.pcImagePath,QstrSaveFileParam.toLatin1().data(),strlen(QstrSaveFileParam.toLatin1().data()));
    }

    int nRet = m_pcMyCamera->SaveImageToFile(&stSaveFileParam);
    pthread_mutex_unlock(&m_hSaveImageMux);/*解锁*/

    return nRet;
}

void MainWindow::on_SaveBmpButton_clicked()
{
    int nRet = SaveImage(MV_Image_Bmp);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Save bmp fail", nRet);
        return;
    }
    ShowErrorMsg("Save bmp succeed", nRet);
}

void MainWindow::on_SaveJpgButton_clicked()
{
    int nRet = SaveImage(MV_Image_Jpeg);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Save jpg fail", nRet);
        return;
    }
    ShowErrorMsg("Save jpg succeed", nRet);
}

void MainWindow::on_SaveTiffButton_clicked()
{
    int nRet = SaveImage(MV_Image_Tif);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Save tiff fail", nRet);
        return;
    }
    ShowErrorMsg("Save tiff succeed", nRet);
}

void MainWindow::on_SavePngButton_clicked()
{
    int nRet = SaveImage(MV_Image_Png);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Save png fail", nRet);
        return;
    }
    ShowErrorMsg("Save png succeed", nRet);
}
