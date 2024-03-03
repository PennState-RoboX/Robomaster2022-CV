#include "cxpconfigform.h"
#include "ui_cxpconfigform.h"
#include "interfacebasicdemo.h"

CXPConfigForm::CXPConfigForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CXPConfigForm)
{
    ui->setupUi(this);
    InterfaceBasicDemo* p=(InterfaceBasicDemo *)parent;
    m_pcMyCamera=p->m_pcMyCamera;
}

CXPConfigForm::~CXPConfigForm()
{
    delete ui;
}

void CXPConfigForm::ShowErrorMsg(QString csMessage, unsigned int nErrorNum)
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

int CXPConfigForm::GetStreamSelector()
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

int CXPConfigForm::GetCurrentStreamDevice()
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

int CXPConfigForm::GetStreamEnableStatus()
{
    ui->teStreamEnableStatus->setEnabled(false);
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("StreamEnableStatus", &stIntValue);
    if (MV_OK != nRet)
    {
        ui->teStreamEnableStatus->setText("");
        return nRet;
    }

    ui->teStreamEnableStatus->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int CXPConfigForm::GetBayerCFAEnable()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    bool bEnable = false;
    int nRet = m_pcMyCamera->GetBoolValue("BayerCFAEnable", &bEnable);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if (true == bEnable)
    {
        ui->cbBayerCFAEnable->setChecked(true);
    }
    else
    {
        ui->cbBayerCFAEnable->setChecked(false);
    }

    return MV_OK;
}

int CXPConfigForm::GetIspGammaEnable()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    bool bEnable = false;
    int nRet = m_pcMyCamera->GetBoolValue("IspGammaEnable", &bEnable);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if (true == bEnable)
    {
        ui->cbIspGammaEnable->setChecked(true);
    }
    else
    {
        ui->cbIspGammaEnable->setChecked(false);
    }

    return MV_OK;
}

int CXPConfigForm::GetIspGamma()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("IspGamma", &stFloatValue);
    if (MV_OK != nRet)
    {
        ui->teIspGamma->setText("");
        return nRet;
    }

    ui->teIspGamma->setText(QString::number(stFloatValue.fCurValue,'f',4));

    return MV_OK;
}


void CXPConfigForm::on_bnGetParam_clicked()
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
    nRet =  GetCurrentStreamDevice();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get CurrentStreamDevice fail", nRet);
    }

    nRet =  GetStreamEnableStatus();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get StreamEnableStatus fail", nRet);
    }

    nRet =  GetBayerCFAEnable();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get BayerCFAEnable fail", nRet);
    }

    nRet =  GetIspGammaEnable();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get IspGammaEnable fail", nRet);
    }

    nRet =  GetIspGamma();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get IspGamma fail", nRet);
    }
}

void CXPConfigForm::on_cbBayerCFAEnable_clicked(bool checked)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    if (true ==  checked)
    {
        int nRet = m_pcMyCamera->SetBoolValue("BayerCFAEnable", true);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set BayerCFAEnable fail", nRet);
            return;
        }
    }
    else
    {
        int nRet = m_pcMyCamera->SetBoolValue("BayerCFAEnable", false);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set BayerCFAEnable fail", nRet);
            return;
        }
    }
}

void CXPConfigForm::on_cbIspGammaEnable_clicked(bool checked)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    if (true ==  checked)
    {
        int nRet = m_pcMyCamera->SetBoolValue("IspGammaEnable", true);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set IspGammaEnable fail", nRet);
            return;
        }
    }
    else
    {
        int nRet = m_pcMyCamera->SetBoolValue("IspGammaEnable", false);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set IspGammaEnable fail", nRet);
            return;
        }
    }
}



void CXPConfigForm::on_cbStreamSelector_currentTextChanged(const QString &arg1)
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

void CXPConfigForm::on_bnSetParam_clicked()
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    if(NULL != m_pcMyCamera)
    {
        QString  QStr = ui->teIspGamma->text();
        float fIspGamma = QStr.toFloat();
        int nRet = m_pcMyCamera->SetFloatValue("IspGamma", (float)fIspGamma);
        if (MV_OK != nRet)
        {
            ui->teIspGamma->setText("");
            ShowErrorMsg("Set IspGamma fail", nRet);
            return;
        }
    }
}
