#ifndef CXPCONFIGFORM_H
#define CXPCONFIGFORM_H

#include <QDialog>

#include <QMainWindow>
#include "MvCamera.h"
#include <map>
#include <QMap>
#include <QDebug>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTextCodec>


namespace Ui {
class CXPConfigForm;
}

class CXPConfigForm : public QDialog
{
    Q_OBJECT

public:
    explicit CXPConfigForm(QWidget *parent = 0);
    ~CXPConfigForm();
    CMvCamera* m_pcMyCamera;

private slots:
    void on_bnGetParam_clicked();

    void on_cbBayerCFAEnable_clicked(bool checked);

    void on_cbIspGammaEnable_clicked(bool checked);

    void on_cbStreamSelector_currentTextChanged(const QString &arg1);

    void on_bnSetParam_clicked();

private:
    Ui::CXPConfigForm *ui;

    QMap<QString, int> m_mapStreamSelector;


private:
    void ShowErrorMsg(QString csMessage, unsigned int nErrorNum);

    int GetStreamSelector();

    int GetCurrentStreamDevice();

    int GetStreamEnableStatus();

    int GetBayerCFAEnable();

    int GetIspGammaEnable();

    int GetIspGamma();
};

#endif // CXPCONFIGFORM_H
