#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <unistd.h>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pcMyCamera = NULL;
    m_bOpenDevice = false;
    m_bStartGrabbing = false;
    m_bContinueRadioButton = false;
    m_bSoftWareTriggerCheck = false;
    m_bTriggerRadioButton = false;
    m_nTriggerMode = MV_TRIGGER_MODE_OFF;
    m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
    memset(&m_stIFList, 0 , sizeof(m_stIFList));
    m_hWnd = (void*)ui->widgetDisplay->winId();
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::EnableControls(bool bIsCameraReady)
{

    ui->EnumInterface->setEnabled(m_bOpenDevice ? false : true);
    ui->EnumDevice->setEnabled(m_bOpenDevice ? false : (m_stIFList.nInterfaceNum > 0 ? true : false));
    ui->OpenDevice->setEnabled(m_bOpenDevice ? false : (bIsCameraReady ? true : false));
    ui->CloseDevice->setEnabled((m_bOpenDevice && bIsCameraReady) ? true : false);
    ui->StartGrab->setEnabled((m_bStartGrabbing && bIsCameraReady) ? false : (m_bOpenDevice ? true : false));
    ui->StopGrab->setEnabled(m_bStartGrabbing ? true : false);
    ui->SoftCheckBox->setEnabled((m_bOpenDevice ) ? true : false);
    ui->SoftOnceButton->setEnabled((m_bStartGrabbing && m_bSoftWareTriggerCheck && m_bTriggerRadioButton) ? true : false);
    ui->ContinueRadioButton->setEnabled(m_bOpenDevice ? true : false);
    ui->TriggerRadioButton->setEnabled(m_bOpenDevice ? true : false);

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

int MainWindow::SetTriggerMode()
{
    return m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
}

int MainWindow::GetTriggerMode()
{
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerMode", &stEnumValue);
    if (MV_OK != nRet)
    {
       return nRet;
    }
    m_nTriggerMode = stEnumValue.nCurValue;
    if (MV_TRIGGER_MODE_ON ==  m_nTriggerMode)
    {
        m_bTriggerRadioButton = true;
        ui->TriggerRadioButton->setChecked(true);
    }
    else
    {
        m_nTriggerMode = MV_TRIGGER_MODE_OFF;
        m_bTriggerRadioButton = false;
        ui->ContinueRadioButton->setChecked(true);
    }
    EnableControls(true);
    return MV_OK;
}



void MainWindow::on_EnumInterface_clicked()
{

    ui->InterfacecomboBox->clear();
    //QString strMsg;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QString FilePath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("文件对话框！"), "/opt/MVS/lib/64",
                                        QString::fromLocal8Bit("CTI文件(*.cti)"));
    qDebug()<<"FilePath: "<<FilePath << endl;
    int nRet = 0;
    nRet = CMvCamera::EnumInterfacsByGenTL(&m_stIFList, FilePath.toLatin1().data());
    if (nRet != MV_OK)
    {
        ShowErrorMsg("EnumInterfacsByGenTL fail", nRet);
        return;
    }
    if (m_stIFList.nInterfaceNum == 0)
    {
        ShowErrorMsg(("No Device"), 0);
        return;
    }
    for (unsigned int i = 0; i < m_stIFList.nInterfaceNum; i++)
    {
        char strUserName[256];
        MV_GENTL_IF_INFO* pstIFInfo = m_stIFList.pIFInfo[i];
        snprintf(strUserName, 256, "Interface[%d]:%s %s (%s) (%d)", i, pstIFInfo->chTLType, pstIFInfo->chInterfaceID, pstIFInfo->chDisplayName, pstIFInfo->nCtiIndex);
        //m_ctrlInterfaceCombo.AddString((CString)strUserName);
        ui->InterfacecomboBox->addItem((QString)strUserName);
    }

    ui->InterfacecomboBox->setCurrentIndex(0);//默认打开是零
    EnableControls(false);

}

void MainWindow::on_EnumDevice_clicked()
{
    ui->DevicecomboBox->clear();
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    ui->DevicecomboBox->setStyle(QStyleFactory::create("Windows"));
    memset(&m_stDevList, 0, sizeof(m_stDevList));
    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    int nRet = CMvCamera::EnumDevicesByGenTL(m_stIFList.pIFInfo[ui->InterfacecomboBox->currentIndex()], &m_stDevList);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(("EnumDevicesByGenTL fail"), nRet);
        return;
    }
    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_GENTL_DEV_INFO* pstDeviceInfo = m_stDevList.pDeviceInfo[i];

        char strUserName[256];
        if (strcmp("", (char*)pstDeviceInfo->chUserDefinedName) != 0)
        {
            snprintf(strUserName, 256, "Dev[%d]:%s (%s)", i, pstDeviceInfo->chUserDefinedName, pstDeviceInfo->chSerialNumber);
        }
        else
        {
            snprintf(strUserName, 256, "Dev[%d]:%s (%s)", i, pstDeviceInfo->chModelName, pstDeviceInfo->chSerialNumber);
        }
        ui->DevicecomboBox->addItem(QString::fromLocal8Bit(strUserName));
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg("No device", 0);
        return;
    }
    ui->DevicecomboBox->setCurrentIndex(0);
    EnableControls(true);
}

void MainWindow::on_OpenDevice_clicked()
{
    if (true == m_bOpenDevice || NULL != m_pcMyCamera)
    {
        return;
    }
    int nIndex = ui->DevicecomboBox->currentIndex();
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg("Please select device", 0);
        return;
    }
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
    int nRet = m_pcMyCamera->OpenDeviceByGenTL(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        ShowErrorMsg("Open Fail", nRet);
        return;
    }
    m_bOpenDevice = true;

    nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        ShowErrorMsg("Get Trigger Mode Fail", nRet);
    }

    EnableControls(true);
}


void MainWindow::on_CloseDevice_clicked()
{
	if(true == m_bStartGrabbing)//防止不停止取流直接关闭设备
    {
        int nRet = m_pcMyCamera->StopGrabbing();
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Stop grabbing fail", nRet);
            return;
        }
         m_bStartGrabbing = false;
    }
    sleep(1/3);
    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    m_bStartGrabbing = false;
    m_bOpenDevice = false;
    EnableControls(true);
}

void MainWindow::on_StartGrab_clicked()
{
    m_pcMyCamera->RegisterImageCallBack(ImageCallBack, this);

    int nRet = m_pcMyCamera->StartGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Start grabbing fail", nRet);
        return;
    }
    m_bStartGrabbing = true;
    EnableControls(true);

}

void MainWindow::on_StopGrab_clicked()
{
    if (false == m_bOpenDevice || false == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }
    int nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Stop grabbing fail", nRet);
        return;
    }
    m_bStartGrabbing = false;
    EnableControls(true);
}

void MainWindow::on_SoftCheckBox_clicked()
{
    if(true == ui->SoftCheckBox->isChecked())
    {
        m_bSoftWareTriggerCheck = true;
    }
    else
    {
        m_bSoftWareTriggerCheck = false;
    }
    EnableControls(true);
}

void MainWindow::on_SoftOnceButton_clicked()
{
    if (true != m_bStartGrabbing)
    {
        return;
    }

    int nRet = m_pcMyCamera->CommandExecute("TriggerSoftware");
    if (nRet != MV_OK)
    {
        ShowErrorMsg("Trigger Software fail", nRet);
        return;
    }
    EnableControls(true);
}

void MainWindow::on_ContinueRadioButton_clicked()
{
    if(true == ui->ContinueRadioButton->isChecked())
    {
        m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);
        m_bContinueRadioButton = true;
        m_bTriggerRadioButton = false;

    }
    EnableControls(true);
}

void MainWindow::on_TriggerRadioButton_clicked()
{
    if(true == ui->TriggerRadioButton->isChecked())
    {
        m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_ON);
        m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        m_bTriggerRadioButton = true;
        m_bContinueRadioButton = false;

    }
    EnableControls(true);
}
