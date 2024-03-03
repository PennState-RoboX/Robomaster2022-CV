#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MvCamera.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void static __stdcall ImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
    void ImageCallBackInner(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInf);
private:
    void ShowErrorMsg(QString csMessage, unsigned int nErrorNum); // ch:显示错误信息窗口 | en: Show the window of error message
    void EnableControls(bool bIsCameraReady);// ch:判断按钮使能 | en:Enable the controls
    int SetTriggerMode();                    // ch:设置触发模式 | en:Set Trigger Mode
    int GetTriggerMode();                    // ch:获取触发模式 | en:Get Trigger Mode

private slots:
    void on_EnumInterface_clicked();

    void on_EnumDevice_clicked();

    void on_OpenDevice_clicked();

    void on_CloseDevice_clicked();

    void on_StartGrab_clicked();

    void on_StopGrab_clicked();

    void on_SoftCheckBox_clicked();

    void on_SoftOnceButton_clicked();

    void on_ContinueRadioButton_clicked();

    void on_TriggerRadioButton_clicked();

private:
    Ui::MainWindow *ui;
    MV_GENTL_IF_INFO_LIST   m_stIFList;                         // ch:采集卡信息链表 | en:The list of GENTLIF info
    MV_GENTL_DEV_INFO_LIST  m_stDevList;                        // ch:设备信息链表 | en:The list of device info
    void*                   m_hWnd;                             // ch:显示窗口句柄 | en:The Handle of Display Window
    bool                    m_bOpenDevice;                      // ch:是否打开设备 | en:Whether to open device
    bool                    m_bStartGrabbing;                   // ch:是否开始抓图 | en:Whether to start grabbing
    bool                    m_bSoftWareTriggerCheck;			// ch:是否软触发 | en:Whether to software
    bool                    m_bContinueRadioButton;				// ch:连续模式按钮 | en:continue button
    bool                    m_bTriggerRadioButton;				// ch:触发模式按钮 | en:Trigger button


    CMvCamera*              m_pcMyCamera;                       // ch:相机类设备实例 | en:The instance of CMvCamera
    int                     m_nTriggerMode;                     // ch:触发模式 | en:Trigger Mode
    int                     m_nTriggerSource;                   // ch:触发源 | en:Trigger Source

};

#endif // MAINWINDOW_H
