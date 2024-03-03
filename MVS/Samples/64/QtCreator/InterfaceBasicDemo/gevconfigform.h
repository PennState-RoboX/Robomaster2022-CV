#ifndef GEVCONFIGFORM_H
#define GEVCONFIGFORM_H

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
class GEVConfigForm;
}

class GEVConfigForm : public QDialog
{
    Q_OBJECT

public:
    explicit GEVConfigForm(QWidget *parent = 0);
    ~GEVConfigForm();

private slots:
    void on_bnTimerReset_clicked();

    void on_bnTimerTriggerSoftware_clicked();

    void on_bnGetParam_clicked();

    void on_bnSetParam_clicked();

    void on_cbHBDecompression_clicked(bool checked);

    void on_cbStreamSelector_currentTextChanged(const QString &arg1);

    void on_cbTimerSelector_currentTextChanged(const QString &arg1);

    void on_cbTimerTriggerSource_currentTextChanged(const QString &arg1);

    void on_cbTimerTriggerActivation_currentTextChanged(const QString &arg1);

private:
    Ui::GEVConfigForm *ui;

    CMvCamera* m_pcMyCamera;

    QMap<QString, int> m_mapStreamSelector;
    QMap<QString, int> m_mapTimerSelector;
    QMap<QString, int> m_mapTimerTriggerSource;
    QMap<QString, int> m_mapTimerTriggerActivation;

private:
    void ShowErrorMsg(QString csMessage, unsigned int nErrorNum);

    int GetStreamSelector();

    int GetTimerSelector();

    int GetTimerTriggerSource();

    int GetTimerTriggerActivation();

    int GetTimerDelay();

    int GetTimerDuration();

    int GetTimerFrequency();

    int GetHBDecompression();

};

#endif // GEVCONFIGFORM_H
