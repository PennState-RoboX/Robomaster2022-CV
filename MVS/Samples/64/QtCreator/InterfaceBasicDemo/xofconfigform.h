#ifndef XOFCONFIGFORM_H
#define XOFCONFIGFORM_H

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
class XOFConfigForm;
}

class XOFConfigForm : public QDialog
{
    Q_OBJECT

public:
    explicit XOFConfigForm(QWidget *parent = 0);
    ~XOFConfigForm();

private slots:
    void on_cbStreamSelector_currentTextChanged(const QString &arg1);

    void on_cbMinFrameDelay_clicked(bool checked);

    void on_bnGetParam_clicked();

    void on_bnSetParam_clicked();

    void on_cbCameraType_currentTextChanged(const QString &arg1);

    void on_cbPartialImageOutputMode_currentTextChanged(const QString &arg1);

private:
    Ui::XOFConfigForm *ui;

    CMvCamera* m_pcMyCamera;

    QMap<QString, int> m_mapStreamSelector;
    QMap<QString, int> m_mapCameraType;
    QMap<QString, int> m_mapPartialImageOutputMode;

private:
    void ShowErrorMsg(QString csMessage, unsigned int nErrorNum);

    int GetStreamSelector();

    int GetCurrentStreamDevice();

    int GetMinFrameDelay();

    int GetCameraType();

    int GetImageHeight();

    int GetFrameTimeoutTime();

    int GetPartialImageOutputMode();
};

#endif // XOFCONFIGFORM_H
