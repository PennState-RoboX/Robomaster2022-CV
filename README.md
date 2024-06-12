# Robomaster2022-CV

## **Project Introduction**

This project is a computer vision application designed for real-time object detection, tracking, and prediction in [Robot Master Competition](https://www.robomaster.com/en-US). The main features of this project are:

- Utilizing opencv for image processing, it identifies and follows armor board of enemy robots using camera.
- Establishing data transmission with lower level platform STM32 via UART, it commands the gimbal of robot to auto aim target.

## **Requirement**

**Camera Spec:**
under construction

**System**

- Ubuntu: 20.04 Arm64
- Robot Master DevelopmentBoard C

**Packages**
- Install MVS SDK for Hikvision camera
https://www.hikvision.com/us-en/support/download/sdk/
- See **`requirements.txt`** for more detail

## **How to Run**

### **Normal Mode**

Run the project in the default mode (video stream off):

```bash
python3 ArmorDetect.py --target-color [red/blue] --show-stream NO
```

### Testing with video stream
Run the project with video stream on/off

```bash
python3 ArmorDetect.py --target-color [red/blue] --show-stream YES/NO
```

### **Debug Mode**

Run the project with additional debug output
You can adjust the color threshold to detect the Armor board in different light settings. 

```bash
python3 ArmorDetect.py --debug --target-color [red/blue] --show-stream YES
```

### **Recording Mode**

The project supports recording video for testing:

```bash
python3 ArmorDetect.py --target-color [red/blue] --recording-dest [Path for output video] --show-stream YES
```

### Test with Recording

You can load existing videos (.mp4) to test the project:

```
python3 ArmorDetect.py --target-color [red/blue] --recording-source [Path for input video] --show-stream YES
```


## **Structure**

```
MVS/                                                # Tools for hikvision camera 
├── Samples/
│   ├── README-CH.md                                # Machine Vision Camera Linux SDK User Manual (Chinese Version)
│   ├── README-EN.md                                # Machine Vision Camera Linux SDK User Manual (English Version)
│   └── aarch64/
│       └── Python/
│           └── MvImport/
│               ├── CameraParams_const.py
│               ├── CameraParams_header.py
│               ├── MvCameraControl_class.py
│               ├── MvCameraControl_header.py
│               ├── MvErrorDefine_const.py
│               ├── PixelType_const.py
│               ├── PixelType_header.py
├── README.md
├── LICENSE
├── requirements.txt
├── ArmorDetect.py                            # Armor detection, main file to run 
├── camera_params.py                                # Parameters and configurations of camera.
├── camera_source.py                                # Camera Class
├── hik_driver.py                                   # Driver script for Hikvision camera
├── kinematic_prediction.py                         # Module for predicting the kinematic behavior of detected objects.
├── Target.py                                       # Target class
└── UART_UTIL.py                                    # Serial communication

```



