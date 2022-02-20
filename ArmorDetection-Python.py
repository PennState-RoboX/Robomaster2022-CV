import pyrealsense2 as rs
import cv2
import numpy as np


def nothing(x):
    pass


def creatTrackbar():  # creat trackbar to adjust the color threshold.
    cv2.namedWindow("morphology_tuner")
    cv2.resizeWindow("morphology_tuner", 600, 180)
    cv2.createTrackbar("open", "morphology_tuner", 1, 30, nothing)
    cv2.createTrackbar("close", "morphology_tuner", 5, 30, nothing)
    cv2.createTrackbar("erode", "morphology_tuner", 2, 30, nothing)
    cv2.createTrackbar("dilate", "morphology_tuner", 2, 30, nothing)


def open_binary(binary, x, y):
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (x, y))
    dst = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel)
    return dst


def close_binary(binary, x, y):
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (x, y))
    dst = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel)
    return dst


def erode_binary(binary, x, y):
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (x, y))
    dst = cv2.erode(binary, kernel)
    return dst


def dilate_binary(binary, x, y):
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (x, y))
    dst = cv2.dilate(binary, kernel)
    return dst


def read_morphology(cap):  # read cap and morphological operation to get led binary image.
    ret, frame = cap.read()

    global targetColor #

    """
    Method1: subtract the opposite color's channel with the desired color channel(For Red:R-B or For Blue:B-R)
    """
    B, G, R = cv2.split(frame)  # Split channels
    if targetColor: # Red = 1
        redHighLight = cv2.subtract(R, B) * 2  # subtract Red channel with Blue Channel
        redBlur = cv2.blur(redHighLight, (3, 3))  # blur the overexposure part(central part of the light bar)
        ret, mask = cv2.threshold(redBlur, 30, 255, cv2.THRESH_BINARY)  # Convert to binary img
    else:
        blueHighLight = cv2.subtract(B, R) * 2  # subtract Red channel with Blue Channel
        blueBlur = cv2.blur(blueHighLight, (3, 3))  # blur the overexposure part(central part of the light bar)
        ret, mask = cv2.threshold(blueBlur, 30, 255, cv2.THRESH_BINARY)  # Convert to binary img
    """
    Method2: try thresholds on differnet channels seperatedly(higher threshold on desired color channel; lower
    threshold on other channels)
    """
    ret1, mask1 = cv2.threshold(R, 130, 255, cv2.THRESH_BINARY)
    ret2, mask2 = cv2.threshold(G, 50, 255, cv2.THRESH_BINARY_INV)
    ret3, mask3 = cv2.threshold(B, 50, 255, cv2.THRESH_BINARY_INV)
    maskRG = cv2.bitwise_and(mask1, mask2)  # split channels,set threshold seperately, bitwise together

    """
    combine Method 1 and 2 together; needed or not?
    """
    maskRBG = cv2.bitwise_and(maskRG, mask3)
    combination = cv2.bitwise_and(maskRBG, mask)

    """
    Show difference between Method 1 and Method 2
    """
    cv2.imshow("substraction", mask)
    cv2.imshow("thresholded", maskRBG)

    """
    Morphological processing of the processed binary image
    """
    # open = cv2.getTrackbarPos('open', 'morphology_tuner') currently not needed
    close = cv2.getTrackbarPos('close', 'morphology_tuner')
    erode = cv2.getTrackbarPos('erode', 'morphology_tuner')
    dilate = cv2.getTrackbarPos('dilate', 'morphology_tuner')
    # dst_open = open_binary(mask, open, open) currently not needed
    dst_close = close_binary(mask, close, close)
    dst_erode = erode_binary(dst_close, erode, erode)
    dst_dilate = dilate_binary(dst_erode, dilate, dilate)

    """
    Display the final image after preprocessing
    """
    cv2.imshow("erode", dst_dilate)

    return dst_dilate, frame


def find_contours(binary, frame):  # find contours and main screening section
    _, contours, heriachy = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    length = len(contours)
    first_data = []  # include all potential light bar's contourArea information dict by dict
    second_data1 = []
    second_data2 = []
    vertices = [] # for future use


    if length > 0:
        # collect info for every contour's rectangle
        for i, contour in enumerate(contours):
            data_dict = dict()
            # print("countour", contour)
            area = cv2.contourArea(contour)

            # area smaller than certain value will not be considered as armor board
            if area < 5:
                continue

            rect = cv2.minAreaRect(contour)
            rx, ry = rect[0]  # min Rectangle's center's (x,y)
            rw = rect[1][0]  # rect's width
            rh = rect[1][1]  # rect's height
            z = rect[2]  # rect's Rotation angle, θ

            coor = cv2.boxPoints(rect)  # coordinates of the four vertices of the rectangle
            #vertices.append(coor) # ignroe it now
            # box = np.int0(coor)
            # cv2.drawContours(frame, [box], -1, (255, 0, 0), 3)#test countor minRectangle
            x1 = coor[0][0]
            y1 = coor[0][1]
            x2 = coor[1][0]
            y2 = coor[1][1]
            x3 = coor[2][0]
            y3 = coor[2][1]
            x4 = coor[3][0]
            y4 = coor[3][1]

            data_dict["area"] = area
            data_dict["rx"] = rx
            data_dict["ry"] = ry
            data_dict["rh"] = rh
            data_dict["rw"] = rw
            data_dict["z"] = z
            data_dict["x1"] = x1
            data_dict["y1"] = y1
            data_dict["x2"] = x2
            data_dict["y2"] = y2
            data_dict["x3"] = x3
            data_dict["y3"] = y3
            data_dict["x4"] = x4
            data_dict["y4"] = y4

            """filer out undesired rectangle, only keep lightBar-like shape"""
            if (float(rh / rw) >= 3) and (float(rh / rw) <= 9) \
                    and (
                    float(z) <= -70 or float(z) >= -30):  # filer out undesired rectangle, only keep lightBar-like shape
                first_data.append(data_dict)
                box = np.int0(coor)
                cv2.drawContours(frame, [box], -1, (255, 0, 0), 3)  # test countor minRectangle

            # The rh will become rw when -70 <= z < -90, rw below will represent the minRectangle's height now
            elif (float(rw / rh) >= 2) and (float(rw / rh) <= 9) \
                    and (float(z) <= -70 or float(z) >= -30):
                first_data.append(data_dict)
                box = np.int0(coor)
                cv2.drawContours(frame, [box], -1, (255, 0, 0), 3)  # test countor minRectangle
                #print(z)

        for i in range(len(first_data)):

            nextRect = i + 1
            while nextRect < len(first_data):
                data_ryi = float(first_data[i].get("ry", 0))
                data_ryc = float(first_data[nextRect].get("ry", 0))
                data_rhi = float(first_data[i].get("rh", 0))
                data_rhc = float(first_data[nextRect].get("rh", 0))
                data_rxi = float(first_data[i].get("rx", 0))
                data_rxc = float(first_data[nextRect].get("rx", 0))
                data_rzi = float(first_data[i].get("z", 0))
                data_rzc = float(first_data[nextRect].get("z", 0))
                data_rwi = float(first_data[i].get("rw", 0))
                data_rwc = float(first_data[nextRect].get("rw", 0))

                if (abs(data_ryi - data_ryc) <= 3 * ((data_rhi + data_rhc) / 2)) \
                        and (abs(data_rhi - data_rhc) <= 0.2 * max(data_rhi, data_rhc)) \
                        and (abs(data_rxi - data_rxc) <= 3 * ((data_rhi + data_rhc) / 2)) \
                        and (abs(data_rzi - data_rzc)) < 10:
                    second_data1.append(first_data[i])
                    second_data2.append(first_data[nextRect])

                elif (abs(data_ryi - data_ryc) <= 3 * ((data_rhi + data_rhc) / 2)) \
                        and (abs(data_rwi - data_rwc) <= 0.2 * max(data_rwi, data_rwc)) \
                        and (abs(data_rxi - data_rxc) <= 3 * ((data_rwi + data_rwc) / 2)) \
                        and (abs(data_rzi - data_rzc)) < 10:
                    second_data1.append(first_data[i])
                    second_data2.append(first_data[nextRect])

                nextRect = nextRect + 1

        if len(second_data1):

            for i in range(len(second_data1)):

                rectangle_x1 = int(second_data1[i]["x1"])
                rectangle_y1 = int(second_data1[i]["y1"])
                rectangle_x2 = int(second_data2[i]["x3"])
                rectangle_y2 = int(second_data2[i]["y3"])

                if abs(rectangle_y1 - rectangle_y2) <= 3 * (abs(rectangle_x1 - rectangle_x2)):

                    point1_1x = second_data1[i]["x1"]
                    point1_1y = second_data1[i]["y1"]
                    point1_2x = second_data1[i]["x2"]
                    point1_2y = second_data1[i]["y2"]
                    point1_3x = second_data1[i]["x3"]
                    point1_3y = second_data1[i]["y3"]
                    point1_4x = second_data1[i]["x4"]
                    point1_4y = second_data1[i]["y4"]
                    point1_z  = second_data1[i]["z"]

                    point2_1x = second_data2[i]["x1"]
                    point2_1y = second_data2[i]["y1"]
                    point2_2x = second_data2[i]["x2"]
                    point2_2y = second_data2[i]["y2"]
                    point2_3x = second_data2[i]["x3"]
                    point2_3y = second_data2[i]["y3"]
                    point2_4x = second_data2[i]["x4"]
                    point2_4y = second_data2[i]["y4"]
                    point2_z  = second_data2[i]["z"]



                    if point1_1x > point2_1x:
                        cv2.rectangle(frame, (point2_2x, point2_2y), (point1_4x, point1_4y), (255, 255, 255), 2)

                    else:
                        cv2.rectangle(frame, (point1_2x, point1_2y), (point2_4x, point2_4y), (255, 255, 255), 2)


                    cv2.putText(frame, "target:", (rectangle_x2, rectangle_y2 - 5), cv2.FONT_HERSHEY_SIMPLEX,
                                0.5, [255, 255, 255])
                    X = int((point2_2x + point1_4x) / 2)
                    Y = int((point2_2y + point1_4y) / 2)
                    center = (X, Y)
                    cv2.circle(frame, center, 2, (0, 0, 255), -1)  # draw the center of the detected armor board
                    print("Target at (x,y) = (" + str(X) + "," + str(Y) + ")")


        else:
            print("Looking for Targets...")



def main():


    creatTrackbar()
    while True:
        binary, frame = read_morphology(cap)  # changed read_morphology()'s output from binary to mask
        find_contours(binary, frame)
        cv2.imshow("original", frame)
        cv2.waitKey(1)

if __name__ == "__main__":

    """Declare your desired target color here"""
    targetColor = 1  # Red = 1 ; Blue = 0

    """init camera as cap, modify camera parameters at here"""
    cap = cv2.VideoCapture(2) # the number here depends on your device's camera, usually default with 0
    cap.set(15, -10)  # EXPOSURE -10 ; threshold's version exposure -8

    main()
