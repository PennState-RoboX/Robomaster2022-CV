#include "xofconfigform.h"
#include "ui_xofconfigform.h"
#include "interfacebasicdemo.h"

XOFConfigForm::XOFConfigForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XOFConfigForm)
{
    ui->setupUi(this);
    InterfaceBasicDemo* p=(InterfaceBasicDemo *)parent;
    m_pcMyCamera=p->m_pcMyCamera;
}

XOFConfigForm::~XOFConfigForm()
{
    delete ui;
}

void XOFConfigForm::ShowErrorMsg(QString csMessage, unsigned int nErrorNum)
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

int XOFConfigForm::GetStreamSelector()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    m_mapStreamSelector.clear();
    MVCC_ENUMVALUE stEnumValue = { 0 };
    MVCC_ENUMENTRY stEnumEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("StreamSelector", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->cbStreamSelector->clear();
    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        memset(&stEnumEntry, 0, sizeof(stEnumEntry));
        stEnumEntry.nValue = stEnumValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("StreamSelector", &stEnumEntry);

        ui->cbStreamSelector->addItem((QString)stEnumEntry.chSymbolic);

        m_mapStreamSelector.insert((QString)stEnumEntry.chSymbolic, stEnumEntry.nValue);
    }

    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        if (stEnumValue.nCurValue == stEnumValue.nSupportValue[i])
        {
            ui->cbStreamSelector->setCurrentIndex(i);
        }
    }

    return MV_OK;
}

int XOFConfigForm::GetCurrentStreamDevice()
{
    ui->teCurrentStreamDevice->setEnabled(false);
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_STRINGVALUE str = {0};

    int nRet = m_pcMyCamera->GetStringValue("CurrentStreamDevice", &str);
    if (MV_OK != nRet)
    {
        ui->teCurrentStreamDevice->setText("");
        return nRet;
    }

    ui->teCurrentStreamDevice->setText(str.chCurValue);

    return MV_OK;
}

int XOFConfigForm::GetMinFrameDelay()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    bool bEnable = false;
    int nRet = m_pcMyCamera->GetBoolValue("MinFrameDelay", &bEnable);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if (true == bEnable)
    {
        ui->cbMinFrameDelay->setChecked(true);
    }
    else
    {
        ui->cbMinFrameDelay->setChecked(false);
    }

    return MV_OK;
}

int XOFConfigForm::GetCameraType()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    m_mapCameraType.clear();
    MVCC_ENUMVALUE stEnumValue = { 0 };
    MVCC_ENUMENTRY stEnumEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("CameraType", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->cbCameraType->clear();
    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        memset(&stEnumEntry, 0, sizeof(stEnumEntry));
        stEnumEntry.nValue = stEnumValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("CameraType", &stEnumEntry);

        ui->cbCameraType->addItem((QString)stEnumEntry.chSymbolic);

        m_mapCameraType.insert((QString)stEnumEntry.chSymbolic, stEnumEntry.nValue);
    }

    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        if (stEnumValue.nCurValue == stEnumValue.nSupportValue[i])
        {
            ui->cbCameraType->setCurrentIndex(i);
        }
    }

    return MV_OK;
}

int XOFConfigForm::GetImageHeight()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("ImageHeight", &stIntValue);
    if (MV_OK != nRet)
    {
        ui->teImageHeight->setText("");
        return nRet;
    }

    ui->teImageHeight->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int XOFConfigForm::GetFrameTimeoutTime()
{
    ui->teFrameTimeoutTime->setEnabled(false);
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }

    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("FrameTimeoutTime", &stIntValue);
    if (MV_OK != nRet)
    {
        ui->teFrameTimeoutTime->setText("");
        return nRet;
    }

    ui->teFrameTimeoutTime->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int XOFConfigForm::GetPartialImageOutputMode()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    m_mapPartialImageOutputMode.clear();
    MVCC_ENUMVALUE stEnumValue = { 0 };
    MVCC_ENUMENTRY stEnumEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("PartialImageOutputMode", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->cbPartialImageOutputMode->clear();
    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        memset(&stEnumEntry, 0, sizeof(stEnumEntry));
        stEnumEntry.nValue = stEnumValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("PartialImageOutputMode", &stEnumEntry);

        ui->cbPartialImageOutputMode->addItem((QString)stEnumEntry.chSymbolic);

        m_mapPartialImageOutputMode.insert((QString)stEnumEntry.chSymbolic, stEnumEntry.nValue);
    }

    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        if (stEnumValue.nCurValue == stEnumValue.nSupportValue[i])
        {
            ui->cbPartialImageOutputMode->setCurrentIndex(i);
        }
    }

    return MV_OK;
}

void XOFConfigForm::on_cbStreamSelector_currentTextChanged(const QString &arg1)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    for (QMap<QString, int>::iterator it = m_mapStreamSelector.begin(); it != m_mapStreamSelector.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("StreamSelector", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set StreamSelector fail", nRet);
                return;
            }
            break;
        }
    }
}

void XOFConfigForm::on_cbMinFrameDelay_clicked(bool checked)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    if (true ==  checked)
    {
        int nRet = m_pcMyCamera->SetBoolValue("MinFrameDelay", true);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set MinFrameDelay fail", nRet);
            return;
        }
    }
    else
    {
        int nRet = m_pcMyCamera->SetBoolValue("MinFrameDelay", false);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set MinFrameDelay fail", nRet);
            return;
        }
    }
}

void XOFConfigForm::on_bnGetParam_clicked()
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    int nRet = GetStreamSelector();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get StreamSelector fail", nRet);
    }

    nRet = GetCurrentStreamDevice();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get CurrentStreamDevice fail", nRet);
    }

    nRet = GetMinFrameDelay();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get MinFrameDelay fail", nRet);
    }

    nRet = GetCameraType();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get CameraType fail", nRet);
    }

    nRet = GetImageHeight();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get ImageHeight fail", nRet);
    }

    nRet = GetFrameTimeoutTime();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get FrameTimeoutTime fail", nRet);
    }

    nRet = GetPartialImageOutputMode();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get PartialImageOutputMode fail", nRet);
    }
}

void XOFConfigForm::on_bnSetParam_clicked()
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    if(NULL != m_pcMyCamera)
    {
        QString  QStr = ui->teImageHeight->text();
        int nImageHeight = QStr.toInt();
        int nRet = m_pcMyCamera->SetIntValue("ImageHeight", (int)nImageHeight);
        if (MV_OK != nRet)
        {
            ui->teImageHeight->setText("");
            ShowErrorMsg("Set ImageHeight fail", nRet);
        }
    }
}

void XOFConfigForm::on_cbCameraType_currentTextChanged(const QString &arg1)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    for (QMap<QString, int>::iterator it = m_mapCameraType.begin(); it != m_mapCameraType.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("CameraType", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set CameraType fail", nRet);
                return;
            }
            break;
        }
    }
}

void XOFConfigForm::on_cbPartialImageOutputMode_currentTextChanged(const QString &arg1)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    for (QMap<QString, int>::iterator it = m_mapPartialImageOutputMode.begin(); it != m_mapPartialImageOutputMode.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("PartialImageOutputMode", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set PartialImageOutputMode fail", nRet);
                return;
            }
            break;
        }
    }
}
