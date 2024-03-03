#include "interfacebasicdemo.h"
#include "ui_interfacebasicdemo.h"
#include <QMessageBox>

#define USER_SEL_0 0
#define USER_SEL_1 1
#define USER_SEL_2 2
#define USER_SEL_3 3

InterfaceBasicDemo::InterfaceBasicDemo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InterfaceBasicDemo)
{
    ui->setupUi(this);
    nCurrentIndex=0;
    m_bOpenInterface=false;
    EnableControls(false);
}

InterfaceBasicDemo::~InterfaceBasicDemo()
{
    delete ui;
}

void InterfaceBasicDemo::EnableControls(bool bIsCameraReady)
{
    ui->bnOpenIF->setEnabled(bIsCameraReady ? true : false);
    ui->bnCloseIF->setEnabled((m_bOpenInterface && bIsCameraReady) ? true : false);
    ui->bnConfig->setEnabled((m_bOpenInterface && bIsCameraReady) ? true : false);
}

// ch:显示错误信息 | en:Show error message
void InterfaceBasicDemo::ShowErrorMsg(QString csMessage, unsigned int nErrorNum)
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

void InterfaceBasicDemo::on_bnEnumIF_clicked()
{
    int nRet = MV_OK;
    memset(&m_stIFList, 0, sizeof(MV_INTERFACE_INFO_LIST));
    ui->cbIFList->clear();
    nCurrentIndex = ui->cbIFType->currentIndex();

    // ch:根据类型枚举不同的采集卡 | en:Enumerate all Interface by Type
    switch (nCurrentIndex)
    {
    case USER_SEL_0:
        nRet = CMvCamera::EnumInterfaces(MV_GIGE_INTERFACE, &m_stIFList);
        break;
    case USER_SEL_1:
        nRet = CMvCamera::EnumInterfaces(MV_CAMERALINK_INTERFACE, &m_stIFList);
        break;
    case USER_SEL_2:
        nRet = CMvCamera::EnumInterfaces(MV_CXP_INTERFACE, &m_stIFList);
        break;
    case USER_SEL_3:
        nRet = CMvCamera::EnumInterfaces(MV_XOF_INTERFACE, &m_stIFList);
        break;
    default:
        break;
    }

    if(MV_OK != nRet)
    {
        ShowErrorMsg("Enum fail", nRet);
        return;
    }

    if(0 == m_stIFList.nInterfaceNum)
    {
        ShowErrorMsg("No device", 0);
        return;
    }

    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
    for(unsigned int i = 0; i < m_stIFList.nInterfaceNum; i++)
    {
        MV_INTERFACE_INFO* pstInterfaceInfo = m_stIFList.pInterfaceInfos[i];
        if(NULL == pstInterfaceInfo)
        {
            continue;
        }

        char strDevInfo[256] = {0};

        if (pstInterfaceInfo->nTLayerType == MV_GIGE_INTERFACE)
        {
            snprintf(strDevInfo, 256, "GEV[%d]: %s | %s | %s", i,
                pstInterfaceInfo->chUserDefinedName,
                pstInterfaceInfo->chModelName,
                pstInterfaceInfo->chSerialNumber);
        }
        else if (pstInterfaceInfo->nTLayerType == MV_CAMERALINK_INTERFACE)
        {
            snprintf(strDevInfo, 256, "CML[%d]: %s | %s | %s", i,
                pstInterfaceInfo->chUserDefinedName,
                pstInterfaceInfo->chModelName,
                pstInterfaceInfo->chSerialNumber);
        }
        else if (pstInterfaceInfo->nTLayerType == MV_CXP_INTERFACE)
        {
            snprintf(strDevInfo, 256, "CXP[%d]: %s | %s | %s", i,
                pstInterfaceInfo->chUserDefinedName,
                pstInterfaceInfo->chModelName,
                pstInterfaceInfo->chSerialNumber);
        }
        else if (pstInterfaceInfo->nTLayerType == MV_XOF_INTERFACE)
        {
            snprintf(strDevInfo, 256, "XoF[%d]: %s | %s | %s", i,
                pstInterfaceInfo->chUserDefinedName,
                pstInterfaceInfo->chModelName,
                pstInterfaceInfo->chSerialNumber);
        }
        else
        {
            snprintf(strDevInfo, 256, "Unknown device type[%d]", i);
        }

        ui->cbIFList->addItem(QString::fromLocal8Bit(strDevInfo));
    }

    ui->cbIFList->setCurrentIndex(USER_SEL_0);
    EnableControls(true);
}

void InterfaceBasicDemo::on_bnOpenIF_clicked()
{
    int nIndex = ui->cbIFList->currentIndex();
    if(0 > nIndex || MV_MAX_INTERFACE_NUM <= nIndex)
    {
        ShowErrorMsg("Please select device", 0);
        return;
    }

    // ch:由设备信息创建设备实例 | en:Device instance created by device information
    if (NULL == m_stIFList.pInterfaceInfos[nIndex])
    {
        ShowErrorMsg("Interface does not exist", 0);
        return;
    }

    if(m_pcMyCamera == NULL)
    {
        m_pcMyCamera = new(std::nothrow) CMvCamera;
        if (NULL == m_pcMyCamera)
        {
            return;
        }
    }

    int nRet = m_pcMyCamera->OpenInterface(m_stIFList.pInterfaceInfos[nIndex], "NULL");
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        ShowErrorMsg("Open Fail", nRet);
        return;
    }

    m_bOpenInterface = true;
    EnableControls(true);
}

void InterfaceBasicDemo::on_bnCloseIF_clicked()
{
    if(m_pcMyCamera)
    {
        m_pcMyCamera->CloseInterface();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }
    m_bOpenInterface = false;
    EnableControls(true);
}

void InterfaceBasicDemo::on_bnConfig_clicked()
{
    nCurrentIndex = ui->cbIFType->currentIndex();
    switch (nCurrentIndex)
    {
    case USER_SEL_0:
        {
            GEVConfigForm CMyForm(this);
            CMyForm.setModal(false);
            CMyForm.show();
            CMyForm.exec();
        }
        break;
    case USER_SEL_1:
        {
            CMLConfigForm CMyForm(this);
            CMyForm.setModal(false);
            CMyForm.show();
            CMyForm.exec();
        }
        break;
    case USER_SEL_2:
        {
            CXPConfigForm CMyForm(this);
            CMyForm.setModal(false);
            CMyForm.show();
            CMyForm.exec();
        }
        break;
    case USER_SEL_3:
        {
            XOFConfigForm CMyForm(this);
            CMyForm.setModal(false);
            CMyForm.show();
            CMyForm.exec();
        }
        break;
    }
}
