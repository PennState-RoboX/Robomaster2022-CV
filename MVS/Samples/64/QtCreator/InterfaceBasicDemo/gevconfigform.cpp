#include "gevconfigform.h"
#include "ui_gevconfigform.h"
#include "interfacebasicdemo.h"

GEVConfigForm::GEVConfigForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GEVConfigForm)
{
    ui->setupUi(this);
    InterfaceBasicDemo* p=(InterfaceBasicDemo *)parent;
    m_pcMyCamera=p->m_pcMyCamera;
}

GEVConfigForm::~GEVConfigForm()
{
    delete ui;
}

void GEVConfigForm::ShowErrorMsg(QString csMessage, unsigned int nErrorNum)
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

int GEVConfigForm::GetStreamSelector()
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

int GEVConfigForm::GetTimerSelector()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    m_mapTimerSelector.clear();
    MVCC_ENUMVALUE stEnumValue = { 0 };
    MVCC_ENUMENTRY stEnumEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TimerSelector", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->cbTimerSelector->clear();
    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        memset(&stEnumEntry, 0, sizeof(stEnumEntry));
        stEnumEntry.nValue = stEnumValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TimerSelector", &stEnumEntry);

        ui->cbTimerSelector->addItem((QString)stEnumEntry.chSymbolic);

        m_mapTimerSelector.insert((QString)stEnumEntry.chSymbolic, stEnumEntry.nValue);
    }

    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        if (stEnumValue.nCurValue == stEnumValue.nSupportValue[i])
        {
            ui->cbTimerSelector->setCurrentIndex(i);
        }
    }

    return MV_OK;
}

int GEVConfigForm::GetTimerTriggerSource()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    m_mapTimerTriggerSource.clear();
    MVCC_ENUMVALUE stEnumValue = { 0 };
    MVCC_ENUMENTRY stEnumEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TimerTriggerSource", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->cbTimerTriggerSource->clear();
    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        memset(&stEnumEntry, 0, sizeof(stEnumEntry));
        stEnumEntry.nValue = stEnumValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TimerTriggerSource", &stEnumEntry);

        ui->cbTimerTriggerSource->addItem((QString)stEnumEntry.chSymbolic);

        m_mapTimerTriggerSource.insert((QString)stEnumEntry.chSymbolic, stEnumEntry.nValue);
    }

    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        if (stEnumValue.nCurValue == stEnumValue.nSupportValue[i])
        {
            ui->cbTimerTriggerSource->setCurrentIndex(i);
        }
    }

    return MV_OK;
}

int GEVConfigForm::GetTimerTriggerActivation()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    m_mapTimerTriggerActivation.clear();
    MVCC_ENUMVALUE stEnumValue = { 0 };
    MVCC_ENUMENTRY stEnumEntry = { 0 };

    int nRet = m_pcMyCamera->GetEnumValue("TimerTriggerActivation", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    ui->cbTimerTriggerActivation->clear();
    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        memset(&stEnumEntry, 0, sizeof(stEnumEntry));
        stEnumEntry.nValue = stEnumValue.nSupportValue[i];
        m_pcMyCamera->GetEnumEntrySymbolic("TimerTriggerActivation", &stEnumEntry);

        ui->cbTimerTriggerActivation->addItem((QString)stEnumEntry.chSymbolic);

        m_mapTimerTriggerActivation.insert((QString)stEnumEntry.chSymbolic, stEnumEntry.nValue);
    }

    for (int i = 0; i < stEnumValue.nSupportedNum; i++)
    {
        if (stEnumValue.nCurValue == stEnumValue.nSupportValue[i])
        {
            ui->cbTimerTriggerActivation->setCurrentIndex(i);
        }
    }

    return MV_OK;
}

int GEVConfigForm::GetTimerDelay()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("TimerDelay", &stIntValue);
    if (MV_OK != nRet)
    {
        ui->teTimerDelay->setText("");
        return nRet;
    }

    ui->teTimerDelay->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int GEVConfigForm::GetTimerDuration()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("TimerDuration", &stIntValue);
    if (MV_OK != nRet)
    {
        ui->teTimerDuration->setText("");
        return nRet;
    }

    ui->teTimerDuration->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int GEVConfigForm::GetTimerFrequency()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    MVCC_INTVALUE_EX stIntValue = { 0 };

    int nRet = m_pcMyCamera->GetIntValue("TimerFrequency", &stIntValue);
    if (MV_OK != nRet)
    {
        ui->teTimerFrequency->setText("");
        return nRet;
    }

    ui->teTimerFrequency->setText(QString::number(stIntValue.nCurValue,10));

    return MV_OK;
}

int GEVConfigForm::GetHBDecompression()
{
    if(NULL==m_pcMyCamera)
    {
        return MV_E_RESOURCE;
    }
    bool bEnable = false;
    int nRet = m_pcMyCamera->GetBoolValue("HBDecompression", &bEnable);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if (true == bEnable)
    {
        ui->cbHBDecompression->setChecked(true);
    }
    else
    {
        ui->cbHBDecompression->setChecked(false);
    }

    return MV_OK;
}

void GEVConfigForm::on_bnTimerReset_clicked()
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    int nRet = m_pcMyCamera->CommandExecute("TimerReset");
    if(MV_OK != nRet)
    {
        ShowErrorMsg("TimerReset fail", nRet);
    }
}

void GEVConfigForm::on_bnTimerTriggerSoftware_clicked()
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    int nRet = m_pcMyCamera->CommandExecute("TimerTriggerSoftware");
    if(MV_OK != nRet)
    {
        ShowErrorMsg("TimerTriggerSoftware fail", nRet);
    }
}

void GEVConfigForm::on_bnGetParam_clicked()
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

    nRet = GetTimerSelector();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get TimerSelector fail", nRet);
    }

    nRet = GetTimerTriggerSource();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get TimerTriggerSource fail", nRet);
    }

    nRet = GetTimerTriggerActivation();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get TimerTriggerActivation fail", nRet);
    }

    nRet = GetTimerDelay();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get TimerDelay fail", nRet);
    }

    nRet = GetTimerDuration();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get TimerDuration fail", nRet);
    }

    nRet = GetTimerFrequency();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get TimerFrequency fail", nRet);
    }

    nRet = GetHBDecompression();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get HBDecompression fail", nRet);
    }
}

void GEVConfigForm::on_bnSetParam_clicked()
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }

    if(NULL != m_pcMyCamera)
    {
        QString  QStr = ui->teTimerDuration->text();
        int nTimerDuration = QStr.toInt();
        int nRet = m_pcMyCamera->SetIntValue("TimerDuration", (int)nTimerDuration);
        if (MV_OK != nRet)
        {
            ui->teTimerDuration->setText("");
            ShowErrorMsg("Set TimerDuration fail", nRet);
        }
    }

    if(NULL != m_pcMyCamera)
    {
        QString  QStr = ui->teTimerDelay->text();
        int nTimerDelay = QStr.toInt();
        int nRet = m_pcMyCamera->SetIntValue("TimerDelay", (int)nTimerDelay);
        if (MV_OK != nRet)
        {
            ui->teTimerDelay->setText("");
            ShowErrorMsg("Set TimerDelay fail", nRet);
        }
    }

    if(NULL != m_pcMyCamera)
    {
        QString  QStr = ui->teTimerDelay->text();
        int nTimerFrequency = QStr.toInt();
        int nRet = m_pcMyCamera->SetIntValue("TimerFrequency", (int)nTimerFrequency);
        if (MV_OK != nRet)
        {
            ui->teTimerFrequency->setText("");
            ShowErrorMsg("Set TimerFrequency fail", nRet);
        }
    }
}

void GEVConfigForm::on_cbHBDecompression_clicked(bool checked)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    if (true ==  checked)
    {
        int nRet = m_pcMyCamera->SetBoolValue("HBDecompression", true);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set HBDecompression fail", nRet);
            return;
        }
    }
    else
    {
        int nRet = m_pcMyCamera->SetBoolValue("HBDecompression", false);
        if (MV_OK != nRet)
        {
            ShowErrorMsg("Set HBDecompression fail", nRet);
            return;
        }
    }
}

void GEVConfigForm::on_cbStreamSelector_currentTextChanged(const QString &arg1)
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

void GEVConfigForm::on_cbTimerSelector_currentTextChanged(const QString &arg1)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    for (QMap<QString, int>::iterator it = m_mapTimerSelector.begin(); it != m_mapTimerSelector.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("TimerSelector", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set TimerSelector fail", nRet);
                return;
            }
            break;
        }
    }
}

void GEVConfigForm::on_cbTimerTriggerSource_currentTextChanged(const QString &arg1)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    for (QMap<QString, int>::iterator it = m_mapTimerTriggerSource.begin(); it != m_mapTimerTriggerSource.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("TimerTriggerSource", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set TimerTriggerSource fail", nRet);
                return;
            }
            break;
        }
    }
}

void GEVConfigForm::on_cbTimerTriggerActivation_currentTextChanged(const QString &arg1)
{
    if(NULL==m_pcMyCamera)
    {
        return;
    }
    for (QMap<QString, int>::iterator it = m_mapTimerTriggerActivation.begin(); it != m_mapTimerTriggerActivation.end(); it++)
    {
        if (it.key() == arg1)
        {
            int nRet = m_pcMyCamera->SetEnumValue("TimerTriggerActivation", it.value());
            if (MV_OK != nRet)
            {
                ShowErrorMsg("Set TimerTriggerActivation fail", nRet);
                return;
            }
            break;
        }
    }
}
