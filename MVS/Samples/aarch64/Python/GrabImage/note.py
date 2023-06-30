# -- coding: utf-8 --
import cv2
from flask import Flask, render_template, Response

import sys
import msvcrt
import base64
import datetime
import logging

sys.path.append("./MvImport")
from MvCameraControl_class import *
from JsonResponse import *
from JsonFlask import *

logging.basicConfig(level=logging.DEBUG,  # 控制台打印的日志级别
                    filename='hikrobot.log',
                    filemode='a',  ##模式，有w和a，w就是写模式，每次都会重新写日志，覆盖之前的日志
                    # a是追加模式，默认如果不写的话，就是追加模式
                    format=
                    '%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s'
                    # 日志格式
                    )

# 这里配置一下 template_folder为当前目录，不然可以找不到 index.html
app = JsonFlask(__name__, template_folder='.')


# index
@app.route('/')
def index():
    return render_template('./templates/index.html')


# 获取码流
def generate(cap):
    # 捕获异常信息
    try:
        while True:
            # 如果是关闭相机，先退出取视频流的循环
            global open
            if (not open):
                break;
            retgrab = cap.grab()
            if retgrab == True:
                logging.debug("Grab true")
            ret1, frame = cap.retrieve()
            # print(type(frame))
            if frame is None:
                logging.error("frame is None")
                continue
            ret1, jpeg = cv2.imencode('.jpg', frame)
            jpg_frame = jpeg.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + jpg_frame + b'\r\n')
    except Exception as e:
        logging.error("generate error: %s" % str(e))


# 开始预览
@app.route('/startPreview')
def startPreview():
    logging.info("======================================")
    logging.info("start to preview video stream, current_time: " + str(datetime.datetime.now()))
    # 全局变量，用于控制获取视频流的开关状态
    global open
    open = True

    # 全局变量，获取视频连接
    global cap
    cap = cv2.VideoCapture(1)

    if False == cap.isOpened():
        logging.error("can't open camera")
        quit()
    else:
        logging.info("start to open camera")

    logging.info("open camera ok")

    # 分辨率设置 3072*2048（海康机器人工业相机 MV-CU060-10GM）
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 3072)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 2048)
    # 帧率配置
    cap.set(cv2.CAP_PROP_FPS, 15)
    return Response(generate(cap), mimetype='multipart/x-mixed-replace;boundary=frame')


# 停止预览
@app.route('/stopPreview')
def stopPreview():
    logging.info("======================================")
    logging.info("stop to preview video stream, current_time: " + str(datetime.datetime.now()))
    logging.info("start to close camera")

    # 全局变量，用于停止循环
    global open
    open = False

    logging.info("release resources start")
    # 全局变量，用于释放相机资源
    try:
        global cap
        cap.release()
        cv2.destroyAllWindows()
    except Exception as e:
        logging.error("stopPreview error: %s" % str(e))
    logging.info("release resources end")
    logging.info("camera closed successfully, current_time: " + str(datetime.datetime.now()))
    logging.info("======================================")
    return "stop to preview"


@app.route('/openAndSave')
def openAndSave():
    logging.info("======================================")
    logging.info("start to grab image, current_time: " + str(datetime.datetime.now()))
    code = 100000
    msg = "连接相机时发生错误"
    # img_base64 = None
    try:
        deviceList = MV_CC_DEVICE_INFO_LIST()
        tlayerType = MV_GIGE_DEVICE | MV_USB_DEVICE

        # ch:枚举设备 | en:Enum device
        ret = MvCamera.MV_CC_EnumDevices(tlayerType, deviceList)
        if ret != 0:
            logging.error("enum devices fail! ret[0x%x]" % ret)
            sys.exit()

        if deviceList.nDeviceNum == 0:
            logging.error("find no device!")
            sys.exit()

        logging.info("find %d devices!" % deviceList.nDeviceNum)

        for i in range(0, deviceList.nDeviceNum):
            mvcc_dev_info = cast(deviceList.pDeviceInfo[i], POINTER(MV_CC_DEVICE_INFO)).contents
            if mvcc_dev_info.nTLayerType == MV_GIGE_DEVICE:
                logging.info("\ngige device: [%d]" % i)
                strModeName = ""
                for per in mvcc_dev_info.SpecialInfo.stGigEInfo.chModelName:
                    strModeName = strModeName + chr(per)
                logging.info("device model name: %s" % strModeName)

                nip1 = ((mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24)
                nip2 = ((mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16)
                nip3 = ((mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8)
                nip4 = (mvcc_dev_info.SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff)
                logging.info("current ip: %d.%d.%d.%d\n" % (nip1, nip2, nip3, nip4))
            elif mvcc_dev_info.nTLayerType == MV_USB_DEVICE:
                logging.info("\nu3v device: [%d]" % i)
                strModeName = ""
                for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chModelName:
                    if per == 0:
                        break
                    strModeName = strModeName + chr(per)
                logging.info("device model name: %s" % strModeName)

                strSerialNumber = ""
                for per in mvcc_dev_info.SpecialInfo.stUsb3VInfo.chSerialNumber:
                    if per == 0:
                        break
                    strSerialNumber = strSerialNumber + chr(per)
                logging.info("user serial number: %s" % strSerialNumber)

        nConnectionNum = 0

        if int(nConnectionNum) >= deviceList.nDeviceNum:
            logging.error("intput error!")
            sys.exit()

        # ch:创建相机实例 | en:Creat Camera Object
        cam = MvCamera()

        # ch:选择设备并创建句柄 | en:Select device and create handle
        stDeviceList = cast(deviceList.pDeviceInfo[int(nConnectionNum)], POINTER(MV_CC_DEVICE_INFO)).contents

        ret = cam.MV_CC_CreateHandle(stDeviceList)
        if ret != 0:
            logging.error("create handle fail! ret[0x%x]" % ret)
            sys.exit()

        # ch:打开设备 | en:Open device
        ret = cam.MV_CC_OpenDevice(MV_ACCESS_Exclusive, 0)
        if ret != 0:
            logging.error("open device fail! ret[0x%x]" % ret)
            sys.exit()

        # ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        if stDeviceList.nTLayerType == MV_GIGE_DEVICE:
            nPacketSize = cam.MV_CC_GetOptimalPacketSize()
            if int(nPacketSize) > 0:
                ret = cam.MV_CC_SetIntValue("GevSCPSPacketSize", nPacketSize)
                if ret != 0:
                    logging.warn("Warning: Set Packet Size fail! ret[0x%x]" % ret)
            else:
                logging.warn("Warning: Get Packet Size fail! ret[0x%x]" % nPacketSize)

        # ch:设置触发模式为off | en:Set trigger mode as off
        ret = cam.MV_CC_SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF)
        if ret != 0:
            logging.error("set trigger mode fail! ret[0x%x]" % ret)
            sys.exit()

        # ch:获取数据包大小 | en:Get payload size
        stParam = MVCC_INTVALUE()
        memset(byref(stParam), 0, sizeof(MVCC_INTVALUE))

        ret = cam.MV_CC_GetIntValue("PayloadSize", stParam)
        if ret != 0:
            logging.error("get payload size fail! ret[0x%x]" % ret)
            sys.exit()

        nPayloadSize = stParam.nCurValue

        # ch:开始取流 | en:Start grab image
        ret = cam.MV_CC_StartGrabbing()
        if ret != 0:
            logging.error("start grabbing fail! ret[0x%x]" % ret)
            sys.exit()

        stDeviceList = MV_FRAME_OUT_INFO_EX()
        memset(byref(stDeviceList), 0, sizeof(stDeviceList))
        data_buf = (c_ubyte * nPayloadSize)()

        ret = cam.MV_CC_GetOneFrameTimeout(byref(data_buf), nPayloadSize, stDeviceList, 1000)
        if ret == 0:
            logging.info("get one frame: Width[%d], Height[%d], nFrameNum[%d]" % (
            stDeviceList.nWidth, stDeviceList.nHeight, stDeviceList.nFrameNum))

            nRGBSize = stDeviceList.nWidth * stDeviceList.nHeight * 3
            stConvertParam = MV_SAVE_IMAGE_PARAM_EX()
            stConvertParam.nWidth = stDeviceList.nWidth
            stConvertParam.nHeight = stDeviceList.nHeight
            stConvertParam.pData = data_buf
            stConvertParam.nDataLen = stDeviceList.nFrameLen
            stConvertParam.enPixelType = stDeviceList.enPixelType
            stConvertParam.nImageLen = stConvertParam.nDataLen
            stConvertParam.nJpgQuality = 70
            stConvertParam.enImageType = MV_Image_Jpeg
            stConvertParam.pImageBuffer = (c_ubyte * nRGBSize)()
            stConvertParam.nBufferSize = nRGBSize
            # ret = cam.MV_CC_ConvertPixelType(stConvertParam)
            logging.info("nImageLen: %d" % stConvertParam.nImageLen)
            ret = cam.MV_CC_SaveImageEx2(stConvertParam)
            if ret != 0:
                logging.error("convert pixel fail ! ret[0x%x]" % ret)
                del data_buf
                sys.exit()
            # file_path = "AfterConvert_RGB2.jpg"
            # file_open = open(file_path, 'wb+')
            # file_open = open(file_path.encode('utf8'), 'wb')
            img_buff = (c_ubyte * stConvertParam.nImageLen)()
            cdll.msvcrt.memcpy(byref(img_buff), stConvertParam.pImageBuffer, stConvertParam.nImageLen)
            # file_open.write(img_buff)

            # 对返回的图片进行 base64 格式转换
            img_base64 = "data:image/jpg;base64," + str(base64.b64encode(img_buff)).split("'")[1]
            code = 200
            msg = "success"
        logging.info("Save Image succeed!")

        # ch:停止取流 | en:Stop grab image
        ret = cam.MV_CC_StopGrabbing()
        if ret != 0:
            logging.error("stop grabbing fail! ret[0x%x]" % ret)
            del data_buf
            sys.exit()

        # ch:关闭设备 | Close device
        ret = cam.MV_CC_CloseDevice()
        if ret != 0:
            logging.error("close deivce fail! ret[0x%x]" % ret)
            del data_buf
            sys.exit()

        # ch:销毁句柄 | Destroy handle
        ret = cam.MV_CC_DestroyHandle()
        if ret != 0:
            logging.error("destroy handle fail! ret[0x%x]" % ret)
            del data_buf
            sys.exit()

        del data_buf
    except Exception as e:
        logging.error("openAndSave error: %s" % str(e))
    # print("openAndSave finished, current_time: " + str(datetime.datetime.now()))

    return JsonResponse(code, msg, img_base64)