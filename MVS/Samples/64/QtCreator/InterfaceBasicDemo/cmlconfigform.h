#ifndef CMLCONFIGFORM_H
#define CMLCONFIGFORM_H

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
class CMLConfigForm;
}

class CMLConfigForm : public QDialog
{
    Q_OBJECT

public:
    explicit CMLConfigForm(QWidget *parent = 0);
    ~CMLConfigForm();

private slots:
    void on_bnGetParam_clicked();

    void on_bnSetParam_clicked();

    void on_cbStreamSelector_currentTextChanged(const QString &arg1);

    void on_cbCameraType_currentTextChanged(const QString &arg1);

    void on_cbStreamPartialImageControl_currentTextChanged(const QString &arg1);

private:
    Ui::CMLConfigForm *ui;

    CMvCamera* m_pcMyCamera;

    QMap<QString, int> m_mapStreamSelector;
    QMap<QString, int> m_mapCameraType;
    QMap<QString, int> m_mapStreamPartialImageControl;

private:
    void ShowErrorMsg(QString csMessage, unsigned int nErrorNum);

    int GetStreamSelector();

    int GetCameraType();

    int GetStreamPartialImageControl();

    int GetImageHeight();

    int GetFrameTimeoutTime();

};

#endif // CMLCONFIGFORM_H
