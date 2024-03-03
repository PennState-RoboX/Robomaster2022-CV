#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MvCamera.h"
#include <map>
#include <QMap>
#include <QDebug>

#define MV_TRIGGER_SOURCE_EncoderModuleOut 6

#define STR_SOFTWARE "Software"
#define STR_FRAMEBURSTSTART "FrameBurstStart"

//模拟增益
typedef enum _MV_PREAMP_GAIN_
{
    GAIN_1000x = 1000,
    GAIN_1400x = 1400,
    GAIN_1600x = 1600,
    GAIN_2000x = 2000,
    GAIN_2400x = 2400,
    GAIN_4000x = 4000,
    GAIN_3200x = 3200,

}MV_PREAMP_GAIN;

//ImageCompressionMode模式
typedef enum _MV_IMAGE_COMPRESSION_MODE_
{
    IMAGE_COMPRESSION_MODE_OFF = 0,
    IMAGE_COMPRESSION_MODE_HB = 2,
}MV_IMAGE_COMPRESSION_MODE;

//触发选项
typedef enum _MV_CAM_TRIGGER_OPTION_
{
    FRAMEBURSTSTART = 6,
    LINESTART = 9,
}MV_CAM_TRIGGER_OPTION;

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
    void EnableControls(bool bIsCameraReady);                   // ch:判断按钮使能 | en:Enable the controls

    int GetImageCompressionMode();                              // ch:获取图像压缩模式  | en:Get Image Compression Mode
    int GetTriggerSelector();                                   // ch:获取触发选项  | en:Get Trigger Selector
    int GetTriggerMode();                                       // ch:获取触发模式 | en:Get Trigger Mode
    int GetExposureTime();                                      // ch:获取曝光时间 | en:Get Exposure Time
    int SetExposureTime();                                      // ch:设置曝光时间 | en:Set Exposure Time
    int GetDigitalShiftGain();                                  // ch:获取数字增益 | en:Get Gain
    int SetDigitalShiftGain();                                  // ch:设置数字增益 | en:Set Gain
    int GetPreampGain();                                        // ch:获取模拟增益 | en:Get PreampGain
    int GetTriggerSource();                                     // ch:获取触发源 | en:Get Trigger Source
    int GetPixelFormat();                                       // ch:获取像素格式 | en:Get Pixel Format
    int GetAcquisitionLineRate();                               // ch:获取实际行频值 | en:Get Acquisition LineRate
    int SetAcquisitionLineRate();                               // ch:设置行频   | en:set Acquisition LineRate
    int GetAcquisitionLineRateEnable();                         // ch:获取行频使能开关 | en:Get Acquisition LineRate Enable
    int GetResultingLineRate();                                 // ch:获取实际行频 | en:Get Resulting LineRate

    int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType);          // ch:保存图片 | en:Save Image

    int CloseDevice();                                          // ch:关闭设备 | en:Close Device


private slots:
    void on_EnumButton_clicked();

    void on_OpenButton_clicked();

    void on_GetParameterButton_clicked();

    void on_StartGrabbingButton_clicked();

    void on_CloseButton_clicked();

    void on_StopGrabbingButton_clicked();

    void on_SetParameterButton_clicked();

    void on_SelchangeTriggerselCombo_currentTextChanged(const QString &arg1);

    void on_SelchangeTriggerswitchCombo_currentTextChanged(const QString &arg1);

    void on_SelchangeTriggersourceCombo_currentTextChanged(const QString &arg1);

    void on_SoftwareOnceButton_clicked();


    void on_SelchangePixelformatCombo_currentTextChanged(const QString &arg1);

    void on_SelchangeImageCompressionModeCombo_currentTextChanged(const QString &arg1);

    void on_ExposureTimeLineEdit_editingFinished();

    void on_SelchangePreampgainCombo_currentTextChanged(const QString &arg1);

    void on_PreampGainLineEdit_editingFinished();

    void on_AcquisitionLineRateLineEdit_editingFinished();

    void on_AcquisitionLineRateEnableCheckBox_clicked(bool checked);

    void on_SaveBmpButton_clicked();

    void on_SaveJpgButton_clicked();

    void on_SaveTiffButton_clicked();

    void on_SavePngButton_clicked();

private:
    Ui::MainWindow *ui;

    void *m_hWnd;                                               // ch:显示窗口句柄 | en:The Handle of Display Window

    MV_CC_DEVICE_INFO_LIST  m_stDevList;                        // ch:设备信息链表 | en:The list of device info

    CMvCamera*              m_pcMyCamera;                       // ch:相机类设备实例 | en:The instance of CMvCamera

    bool                    m_bGrabbing;                        // ch:是否开始抓图 | en:The flag of Grabbing
    bool                    m_bOpenDevice;                      // ch:是否打开设备 | en:Whether to open device
    int                     m_nTriggerMode;                     // ch:触发模式 | en:Trigger Mode
    int                     m_nTriggerSelector;                 // ch:触发选项 | en:Trigger Selector
    int                     m_nTriggerSource;                   // ch:触发源 | en:Trigger Source
    int                     m_nPreampGain;                      // ch:模拟增益 | en:Preamp Gain
    int                     m_nPixelFormat;                     // ch:像素格式 | en:PixelFormat
    int                     m_nImageCompressionMode;            // ch:图像压缩模式 | en:ImageCompressionMode
    float                   m_dExposureEdit;                    // ch:图像曝光 | en:Image Exposure
    float                   m_dDigitalShiftGainEdit;            // ch:数字增益 | en:Digital Shift Gain
    int                     m_nAcquisitionLineRateEdit;         // ch:行频 | en:AcquisitionLineRate
    unsigned int            m_nSaveImageBufSize;                // ch:保存图像时缓冲区大小 | en:The size of SaveImageBuf

    bool                    m_bTriggerModeCheck;                // ch:触发模式开启/关闭标志位 | en:The flag of TriggerMode On/Off
    bool                    m_bPreampGain;                      // ch:模拟增益使能标志位 | en:The flag of PreampGain
    bool                    m_bAcquisitionLineRate;             // ch:行频使能标志位 | en:The flag of AcquisitionLineRate
    bool                    m_bHBMode;                          // ch:是否无损压缩模式 | en: HB Mode On/Off
    unsigned char*          m_pSaveImageBuf;                    // ch:保存图像缓冲区 | en: The Buffer of SaveImage
    MV_FRAME_OUT_INFO_EX    m_stImageInfo;                      // ch:图像信息结构体 | en: The image info

    pthread_mutex_t        m_hSaveImageMux;                     // ch:保存图像锁 | en: The Mutex of SaveImage


    QMap<QString, int> m_mapPixelFormat;                        // ch:像素格式字符串-值 map | The map of PixelFormat value
    QMap<QString, int> m_mapPreampGain;                         // ch:模拟增益字符串-值 map | The map of PreampGain value
    QMap<QString, int> m_mapTriggerSource;                      // ch:触发源字符串-值   map | The map of TriggerSource value

};

#endif // MAINWINDOW_H
