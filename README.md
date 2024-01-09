# Robomaster2022-CV

# RoboX CV Readme

## **Project Introduction**

This project is a computer vision application designed for real-time object detection, tracking, and prediction in [Robot Master Competition](https://www.robomaster.com/en-US). The main features of this project are:

- Utilizing opencv for image processing, it identifies and follows armor board of enemy robots using camera.
- Establishing data transmission with lower level platform STM32 via UART, it commands the gimbal of robot to auto aim target.

## **Requirement**

**Camera Spec:**

To be continue＃

**System**

- Ubuntu: 20.04 Arm64
- Robot Master DevelopmentBoard C

**Packages**
- Instll MVS SDK for Hikvision camera
https://www.hikvision.com/us-en/support/download/sdk/
- See **`requirements.txt`** for more detail

## **How to Run**

### **Normal Mode**

Run the project in the default mode:

```bash
python3 ArmorDetect_D435i.py --target-color [RED/BLUE]
```

### **Debug Mode**

Run the project with additional debug output:

```bash
python3 ArmorDetect_D435i.py --debug --target-color [RED/BLUE]
```

### **Recording Mode**

Run the project to record video :

```bash
python3 ArmorDetect_D435i.py --target-color [RED/BLUE] --recording-dest [Path for output video] 
```

### Test via Recording

You can also load existing video to test the project:

```
python3 ArmorDetect_D435i.py --target-color [RED/BLUE] --recording-source [Path for input video]
```

## **Structure**

```
.
├── MVS                      # Tools for hikvision camera 
├── Deprecated               # deprecated scripts or old versions of files.

├── ArmorDetect_D435i.py     # armor detection, main file to run 
├── camera_params.py         # parameters and configurations of camera.
├── camera_source.py         # Camera class
├── hik_driver.py            # Driver script for Hikvision camera
├── KalmanFilterClass.py     # Implementation of the Kalman Filter for object tracking.
├── kinematic_prediction.py  # Module for predicting the kinematic behavior of detected objects.
└── UART_UTIL.py             # serial communication
```
