#ifndef INTERFACEBASICDEMO_H
#define INTERFACEBASICDEMO_H

#include <QMainWindow>
#include "MvCamera.h"
#include "cxpconfigform.h"
#include "gevconfigform.h"
#include "cmlconfigform.h"
#include "xofconfigform.h"

namespace Ui {
class InterfaceBasicDemo;
}

class InterfaceBasicDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit InterfaceBasicDemo(QWidget *parent = 0);
    ~InterfaceBasicDemo();
    CMvCamera* m_pcMyCamera;
    int nCurrentIndex;
    bool m_bOpenInterface;
private:
    void ShowErrorMsg(QString csMessage, unsigned int nErrorNum);
    void EnableControls(bool bIsCameraReady);
private slots:

    void on_bnEnumIF_clicked();

    void on_bnOpenIF_clicked();

    void on_bnCloseIF_clicked();

    void on_bnConfig_clicked();

private:
    Ui::InterfaceBasicDemo *ui;

    MV_INTERFACE_INFO_LIST m_stIFList;

};

#endif // INTERFACEBASICDEMO_H
