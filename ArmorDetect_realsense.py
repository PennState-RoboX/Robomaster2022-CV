import math

import pyrealsense2 as rs
import cv2
import numpy as np
import serial
from UART_UTIL import setUpSerial, send_data
from solve_Angle import solve_Angle455
from CamInfo_D455 import undistort
import time


def nothing(x):
    pass

def creatTrackbar():  # creat trackbar to adjust the color threshold.
    if targetColor: # red
        cv2.namedWindow("morphology_tuner")
        cv2.resizeWindow("morphology_tuner", 600, 180)
        cv2.createTrackbar("open", "morphology_tuner", 1, 30, nothing)
        cv2.createTrackbar("close", "morphology_tuner", 15, 30, nothing)
        cv2.createTrackbar("erode", "morphology_tuner", 2, 30, nothing)
        cv2.createTrackbar("dilate", "morphology_tuner", 3, 30, nothing)
    else: #blue
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
    frame = cap
    # frame = unread_morphologydistort(frame)
    global targetColor  #
    B, G, R = cv2.split(frame)  # Split channels

    """
    Method1: subtract the opposite color's channel with the desired color channel(For Red:R-B or For Blue:B-R)
    """

    # if targetColor: # Red = 1
    #    redHighLight = cv2.subtract(R, B) * 2  # subtract Red channel with Blue Channel
    #    redBlur = cv2.blur(redHighLight, (3, 3))  # blur the overexposure part(central part of the light bar)
    #    ret, mask = cv2.threshold(redBlur, 30, 255, cv2.THRESH_BINARY)  # Convert to binary img
    # else:
    # blueHighLight = cv2.subtract(B, R) * 2  # subtract Red channel with Blue Channel
    # blueBlur = cv2.blur(blueHighLight, (3, 3))  # blur the overexposure part(central part of the light bar)
    # ret, mask = cv2.threshold(blueBlur, 110, 255, cv2.THRESH_BINARY)  # Convert to binary img
    """
    Method2: try thresholds on differnet channels seperatedly(higher threshold on desired color channel; lower
    threshold on other channels)
    """
    """ for red """
    if targetColor:  # Red = 1
        ret1, mask1 = cv2.threshold(R, 130, 255, cv2.THRESH_BINARY)
        ret2, mask2 = cv2.threshold(G, 100, 255, cv2.THRESH_BINARY_INV)
        # ret3, mask3 = cv2.threshold(B, 250, 255, cv2.THRESH_BINARY_INV)
        mask_processed = cv2.bitwise_and(mask1, mask2)  # split channels,set threshold seperately, bitwise together
        #mask_processed = cv2.blur(mask_red, (3, 3))  # blur the overexposure part(central part of the light bar)
        # maskRBG1 = cv2.bitwise_and(maskRG1, mask3)

        #red_high_light = cv2.subtract(R, B) * 3   # subtract Red channel with Blue Channel
        #red_blur = cv2.blur(red_high_light, (3, 3))  # blur the overexposure part(central part of the light bar)
        #ret, mask_processed = cv2.threshold(red_blur, 80, 255, cv2.THRESH_BINARY)  # Convert to binary img

    else:  # Blue mode
        """ for blue in threshold method, but it can't filter out white lights
            ret3, mask4_max = cv2.threshold(B, 240, 255, cv2.THRESH_BINARY_INV)
            ret3, mask4_max_min = cv2.threshold(B, 210, 255, cv2.THRESH_BINARY)
            ret2, mask5 = cv2.threshold(G, 245, 255, cv2.THRESH_BINARY_INV)
            ret2, mask6 = cv2.threshold(mask5, 71, 255, cv2.THRESH_BINARY)
            maskGB = cv2.bitwise_and(mask4_max_min, mask6)  # split channels,set threshold seperately, bitwise together
            #maskRGB = cv2.bitwise_and(maskGB, mask6)
        """
        blue_high_light = cv2.subtract(B, R) * 2  # subtract Red channel with Blue Channel
        blue_blur = cv2.blur(blue_high_light, (3, 3))  # blur the overexposure part(central part of the light bar)
        ret, mask_processed = cv2.threshold(blue_blur, 110, 255, cv2.THRESH_BINARY)  # Convert to binary img

    """
    combine Method 1 and 2 together; needed or not?
    """

    # combination = cv2.bitwise_and(maskRBG, mask)

    """
    Show difference between Method 1 and Method 2
    """
    cv2.imshow("sub/threshold", mask_processed)
    #cv2.imshow("thresholded", mask)

    """
    Morphological processing of the processed binary image
    """
    # open = cv2.getTrackbarPos('open', 'morphology_tuner') currently not needed
    close = cv2.getTrackbarPos('close', 'morphology_tuner')
    erode = cv2.getTrackbarPos('erode', 'morphology_tuner')
    dilate = cv2.getTrackbarPos('dilate', 'morphology_tuner')
    # dst_open = open_binary(mask, open, open) currently not needed
    dst_close = close_binary(mask_processed, close, close)
    dst_erode = erode_binary(dst_close, erode, erode)
    dst_dilate = dilate_binary(dst_erode, dilate, dilate)

    """
    Display the final image after preprocessing
    """
    cv2.imshow("erode", dst_dilate)

    return dst_dilate, frame


def find_contours(binary, frame, fps):  # find contours and main screening section
    global num
    contours, heriachy = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    length = len(contours)
    first_data = []  # include all potential light bar's contourArea information dict by dict
    second_data1 = []
    second_data2 = []
    potential_Targets = []  # unit: per imgPoints(np.array([[bl], [tl], [tr],[br]]))

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
            # vertices.append(coor) # ignroe it now
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
            #box = np.int0(coor)
            #cv2.drawContours(frame, [box], -1, (0, 255, 0), 3)
            #print("rh: ",rh, "rw: ",rw,"z: ",z)

            """filer out undesired rectangle, only keep lightBar-like shape"""
            """90--->45-->0-center(horizontally)->90-->45-->0"""
            if (float(rh / rw) >= 1.5) and (float(rh / rw) <= 8) \
                    and (float(z) <= 15 and float(z) >= 0):  # filer out undesired rectangle, only keep lightBar-like shape

                first_data.append(data_dict)
                box = np.int0(coor)
                cv2.drawContours(frame, [box], -1, (255, 0, 0), 3)  # test countor minRectangle
                #print(float(z))
            # The rh will become rw when center(horizontally)->90-->45-->0, rw below will represent the minRectangle's height now
            elif (float(rh / rw) >= 0.125) and (float(rh / rw) <= 0.7) \
                    and (float(z) <= 90 and float(z) >= 75):

                '''switch rw and rh back to normal'''
                temp = data_dict["rh"]
                data_dict["rh"] = data_dict["rw"]
                data_dict["rw"] = temp

                #print(float(z))
                first_data.append(data_dict)
                box = np.int0(coor)
                cv2.drawContours(frame, [box], -1, (0, 0, 255), 3)  # test countor minRectangle
                # print(z)

        for i in range(len(first_data)):

            nextRect = i + 1
            while nextRect < len(first_data):
                data_ryi = float(first_data[i].get("ry", 0))  # i = initial
                data_ryc = float(first_data[nextRect].get("ry", 0))  # c = current
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



                nextRect = nextRect + 1

        if len(second_data1):

            for i in range(len(second_data1)): #find vertices for each

                rectangle_x1 = int(second_data1[i]["x1"])
                rectangle_y1 = int(second_data1[i]["y1"])
                rectangle_x2 = int(second_data2[i]["x3"])
                rectangle_y2 = int(second_data2[i]["y3"])

                if abs(rectangle_y1 - rectangle_y2) <= 3 * (abs(rectangle_x1 - rectangle_x2)): #if find potential bounded lightbar formed targets
                    # all point data-type here are <class 'numpy.float32'>
                    point1_1x = second_data1[i]["x1"]
                    point1_1y = second_data1[i]["y1"]
                    point1_2x = second_data1[i]["x2"]
                    point1_2y = second_data1[i]["y2"]
                    point1_3x = second_data1[i]["x3"]
                    point1_3y = second_data1[i]["y3"]
                    point1_4x = second_data1[i]["x4"]
                    point1_4y = second_data1[i]["y4"]
                    point1_z = second_data1[i]["z"]
                    #print(type(point1_2y))

                    '''rect1_param: output vertices in order, [bl,tl,tr,br]'''
                    rect1_param = np.array([[point1_1x,point1_1y],[point1_2x,point1_2y],[point1_3x,point1_3y],[point1_4x,point1_4y]])
                    rect1_param = findVerticesOrder(rect1_param)
                    #rect1_param[0][1] equals to bl_y


                    point2_1x = second_data2[i]["x1"]
                    point2_1y = second_data2[i]["y1"]
                    point2_2x = second_data2[i]["x2"]
                    point2_2y = second_data2[i]["y2"]
                    point2_3x = second_data2[i]["x3"]
                    point2_3y = second_data2[i]["y3"]
                    point2_4x = second_data2[i]["x4"]
                    point2_4y = second_data2[i]["y4"]
                    point2_z = second_data2[i]["z"]

                    '''rect2_param: output vertices in order, [bl,tl,tr,br]'''
                    rect2_param = np.array([[point2_1x, point2_1y], [point2_2x, point2_2y], [point2_3x, point2_3y],[point2_4x, point2_4y]])
                    rect2_param = findVerticesOrder(rect2_param)  # output order: [bl,tl,tr,br]

                    if point1_4x > point2_4x:  # point 1 is the rectangle vertices of right light bar
                        right_lightBar_len = abs(rect1_param[2][1] - rect1_param[3][1])  # (TR - BR)y-axis
                        left_lightBar_len = abs(rect2_param[1][1] - rect2_param[0][1]) # (TL - BL)y-axis
                        """all armor tr,tl,br,bl are exclude the light bar"""
                        armor_tl_y = int(rect2_param[1][1] - 1 / 2 * left_lightBar_len)
                        armor_br_y = int(rect1_param[3][1] + 1 / 2 * right_lightBar_len)
                        armor_tr_y = int(rect1_param[2][1] - 1 / 2 * right_lightBar_len)
                        armor_bl_y = int(rect2_param[0][1] + 1 / 2 * left_lightBar_len)
                        armor_tl_x = int(rect2_param[1][0])
                        armor_br_x = int(rect1_param[3][0])
                        armor_tr_x = int(rect1_param[2][0])
                        armor_bl_x = int(rect2_param[0][0])

                        # cv2.rectangle(frame, (int(point1_3x), int(armor_br)), (int(point2_1x), int(armor_tl)),(255, 0, 255), 2)
                        #cv2.line(frame, (armor_tl_x, armor_tl_y), (armor_br_x, armor_br_y), (0, 0, 255), 2)
                        #cv2.line(frame, (armor_tr_x, armor_tr_y), (armor_bl_x, armor_bl_y), (0, 0, 255), 2)

                        # cv2.circle(frame, (int(armor_tr_x), int(armor_tr_y)), 9, (255, 255, 255), -1)  # test armor_tr
                        # cv2.circle(frame, (int(armor_tl_x), int(armor_tl_y)), 9, (0, 255, 0), -1)  # test armor_tl
                        # cv2.circle(frame, (int(armor_bl_x), int(armor_bl_y)), 9, (255, 255, 0), -1) # test bottom left
                        # cv2.circle(frame, (int(armor_br_x), int(armor_br_y)), 9, (0, 100, 250), -1)  # test bottom left

                        '''Prepare rect 4 vertices array and then pass it to (1) solve_Angle455's argument (2) number detection'''
                        imgPoints = np.array(
                            [[armor_bl_x, armor_bl_y], [armor_tl_x, armor_tl_y], [armor_tr_x, armor_tr_y],
                             [armor_br_x, armor_br_y]], dtype=np.float64)
                        tvec, Yaw, Pitch = solve_Angle455(imgPoints)


                    else:  # point 2 is the rectangle vertices of right light bar
                        right_lightBar_len = abs(rect2_param[2][1] - rect2_param[3][1])  # (TR - BR)y-axis
                        left_lightBar_len = abs(rect1_param[1][1] - rect1_param[0][1])  # (TL - BL)y-axis
                        """all armor tr,tl,br,bl are exclude the light bar"""
                        armor_tl_y = int(rect1_param[1][1] - 1 / 2 * left_lightBar_len)
                        armor_br_y = int(rect2_param[3][1] + 1 / 2 * right_lightBar_len)
                        armor_tr_y = int(rect2_param[2][1] - 1 / 2 * right_lightBar_len)
                        armor_bl_y = int(rect1_param[0][1] + 1 / 2 * left_lightBar_len)
                        armor_tl_x = int(rect1_param[1][0])
                        armor_br_x = int(rect2_param[3][0])
                        armor_tr_x = int(rect2_param[2][0])
                        armor_bl_x = int(rect1_param[0][0])

                        #cv2.line(frame, (armor_tl_x, armor_tl_y), (armor_br_x, armor_br_y), (255, 255, 255), 2)
                        #cv2.line(frame, (armor_tr_x, armor_tr_y), (armor_bl_x, armor_bl_y), (255, 255, 255), 2)

                        # cv2.circle(frame, (int(armor_tr_x), int(armor_tr_y)), 9, (255, 255, 255), -1)  # test armor_tr
                        # cv2.circle(frame, (int(armor_tl_x), int(armor_tl_y)), 9, (0, 255, 0), -1)  # test armor_tl
                        # cv2.circle(frame, (int(armor_bl_x), int(armor_bl_y)), 9, (255, 255, 0), -1) # test bottom left
                        # cv2.circle(frame, (int(armor_br_x), int(armor_br_y)), 9, (0, 100, 250), -1)  # test bottom left

                        '''Prepare rect 4 vertices array and then pass it as solve_Angle455's argument'''
                        imgPoints = np.array(
                            [[armor_bl_x, armor_bl_y], [armor_tl_x, armor_tl_y], [armor_tr_x, armor_tr_y],
                             [armor_br_x, armor_br_y]], dtype=np.float64)
                        tvec, Yaw, Pitch = solve_Angle455(imgPoints)








                    '''collecting data set at below'''
                    armboard_width = 27
                    armboard_height = 25


                    coordinate_before = np.float32(imgPoints)
                    # coordinate_after is in the order of imgPoints (bl,tl,tr,br)
                    coordinate_after = np.float32([[0, armboard_height], [0, 0], [armboard_width, 0],
                                                   [armboard_width,armboard_height]])

                    # Compute the transformation matrix
                    trans_mat = cv2.getPerspectiveTransform(coordinate_before, coordinate_after)
                    # Perform transformation and show the result
                    trans_img = cv2.warpPerspective(frame, trans_mat, (armboard_width, armboard_height))


                    trans_img = np.array(255 * (trans_img / 255) ** 0.5, dtype='uint8')

                    # Zero light bar effect on image edges
                    # Define the number of pixel zeroed
                    vertical_pixel = 2
                    horizontal_pixel = 4
                    for row in range(armboard_height):
                        for col in range(armboard_width):
                            if (row < vertical_pixel or row > armboard_height - vertical_pixel - 1) \
                                    or (col < horizontal_pixel or col > armboard_width - horizontal_pixel - 1):
                                trans_img[row, col] = 0

                    cv2.imshow("trans_img", trans_img)
                    cv2.resizeWindow("trans_img", 180, 180)

                    # Convert to grayscale image
                    gray_img = cv2.cvtColor(trans_img, cv2.COLOR_BGR2GRAY)
                    #cv2.imshow("dila_img", gray_img)
                    # Convert to binary image
                    #_, binary_img = cv2.threshold(gray_img, threshold, 255, cv2.THRESH_BINARY)
                    # Erosion and dilation to denoise
                    # Define the kernel (5 pixel * 5 pixel square)
                    #kernel = np.ones((2, 2), np.uint8)
                    #erode_img = cv2.erode(binary_img, kernel, iterations=1)
                    #dila_img = cv2.dilate(erode_img, kernel, iterations=1)

                    cv2.imshow("dila_img", gray_img)

                    #cv2.imwrite('c:/Users/Zhuang/Desktop/RM2022\DataSet\General/{}.png'.format(num), gray_img)
                    num += 1



                    '''when angle = 90 or 90 --> angle --> 45; vertices: [ 1 2 ]
                                                                         [ 4 3 ]
                        if point1_4x > point2_4x, point 1 is the rectangle vertices of right light bar;
                        if point1_4x < point2_4x, point 2 is the rectangle vertices of right light bar
                    '''
                    '''when angle != 90; vertices: [ 2 3 ]
                                                   [ 1 4 ]
                        if point1_4x > point2_4x, point 1 is the rectangle vertices of right light bar;
                        if point1_4x < point2_4x, point 2 is the rectangle vertices of right light bar
                    '''






                    """calculate image's area"""


                    depth = str(tvec[2][0]) + 'mm'
                    cv2.putText(frame, depth, (90, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, str(Yaw), (90, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, str(Pitch), (90, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, str(fps), (90, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])

                    cv2.putText(frame, "target:", (rectangle_x2, rectangle_y2 - 5), cv2.FONT_HERSHEY_SIMPLEX,
                                0.5, [255, 255, 255])
                    X = int((point2_2x + point1_4x) / 2)
                    Y = int((point2_2y + point1_4y) / 2)
                    center = (X, Y)
                    cv2.circle(frame, center, 2, (0, 0, 255), -1)  # draw the center of the detected armor board
                    #print("Target at (x,y) = (" + str(X) + "," + str(Y) + ")")


                    '''collect potential targets' info'''
                    target_Dict = dict()
                    target_Dict["depth"] = float(tvec[2][0])
                    target_Dict["Yaw"] = Yaw
                    target_Dict["Pitch"] = Pitch
                    target_Dict["imgPoints"] = imgPoints
                    potential_Targets.append(target_Dict)

        # else:
        # print("Looking for Targets...")


        return potential_Targets

def targetsFilter(potential_Targetsets, frame):
    '''
    target with Number & greatest credits wins in filter process
    Credit Consideration: Area, Depth, Pitch, Yaw
    Credit Scale: 1 - 3
    '''
    max_Credit = 0
    best_Target = [] # order: [depth, Yaw, Pitch, imgpoints]
    for target in potential_Targetsets:
        depth = float(target.get("depth", 0))
        Yaw = float(target.get("Yaw", 0))
        Pitch = float(target.get("Pitch", 0))
        imgPoints = target.get("imgPoints", 0)

        # target with greatest credits wins in filter process;total_Credit = depth credit + angle credit
        depth_Credit = 0
        angle_Credit = 0


        """get area; distort caused large variation; failed so far"""
        '''
        dim = np.zeros(frame.shape[:2], dtype="uint8")  #(h,w)=iamge.shape[:2]
        polygon_mask = cv2.fillPoly(dim, np.array([imgPoints.astype(np.int32)]), 255)
        area = np.sum(np.greater(polygon_mask, 0))
        print(area)
        '''

        """Assess Depth"""
        if depth < 1800:
            depth_Credit += 5
        elif depth < 2500:
            depth_Credit += 3

        """Assess Angle"""
        if abs(Yaw) < 5 or abs(Pitch) < 10:
            angle_Credit += 4
        elif abs(Yaw) < 10 or abs(Pitch) < 15:
            angle_Credit += 3
        elif abs(Yaw) < 20 or abs(Pitch) < 20:
            angle_Credit += 3
        elif abs(Yaw) < 30 or abs(Pitch) < 30:
            angle_Credit += 2

        """evaluate score"""
        if (depth_Credit + angle_Credit) > max_Credit:
            max_Credit = (depth_Credit + angle_Credit)
            best_Target = [depth, Yaw, Pitch, imgPoints]

    imgPoints = best_Target[3]

    cv2.line(frame, (int(imgPoints[1][0]), int(imgPoints[1][1])), (int(imgPoints[3][0]), int(imgPoints[3][1])), (255, 255, 255), 2)
    cv2.line(frame, (int(imgPoints[2][0]), int(imgPoints[2][1])), (int(imgPoints[0][0]), int(imgPoints[0][1])), (255, 255, 255), 2)
    return best_Target





def findVerticesOrder(vertices):
    ''' sort rectangle points by clockwise '''
    sort_x = vertices[np.argsort(vertices[:, 0]), :]

    Left = sort_x[:2, :]
    Right = sort_x[2:, :]
    # Left sort
    Left = Left[np.argsort(Left[:, 1])[::-1], :]
    # Right sort
    Right = Right[np.argsort(Right[:, 1]), :]

    return np.concatenate((Left, Right), axis=0)


def decimalToHexSerial(Yaw,Pitch):
    '''for int part'''
    int_Pitch = int(Pitch + 50) # for check sum
    hex_int_Pitch = str(hex(int_Pitch))  # form: 0xa; encode -45 degree to -45+50=5 degree
    hex_int_Pitch = ('0' + hex_int_Pitch[2:])[-2:]  # delete '0x'

    int_Yaw = int(Yaw + 50) # for check sum
    hex_int_Yaw = str(hex(int_Yaw))  # encode -45 degree to -45+50=5 degree
    hex_int_Yaw = ('0' + hex_int_Yaw[2:])[-2:]

    '''for decimal part to serial hex: input -314.159 output ===> 10'''
    deci_Pitch = format(math.modf(abs(Pitch))[0], '.2f')  # decimal part is always positive
    str_deci_Pitch = str(deci_Pitch)[-2:]
    int_deci_Pitch = int(str_deci_Pitch)  # for check sum
    hex_deci_Pitch = str(hex(int_deci_Pitch))
    hex_deci_Pitch = ('0' + hex_deci_Pitch[2:])[-2:]  # to transfer by serial; form:314.159 => 16

    deci_Yaw = format(math.modf(abs(Yaw))[0], '.2f')  # decimal part is always positive
    str_deci_Yaw = str(deci_Yaw)[-2:]
    int_deci_Yaw = int(str_deci_Yaw)  # for check sum
    hex_deci_Yaw = str(hex(int_deci_Yaw))
    hex_deci_Yaw = ('0' + hex_deci_Yaw[2:])[-2:]  # to transfer by serial; form:314.159 => 16

    int_sumAll = int_Pitch + int_Yaw + int_deci_Pitch + int_deci_Yaw + 9
    hex_sumAll = str(hex(int_sumAll))
    hex_sumAll = ('0' + hex_sumAll[2:])[-2:]  #delete '0x'

    serial_lst = [hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw, hex_sumAll]
    return serial_lst

def main():
    creatTrackbar()

    ser = None
    #ser = serial.Serial('com3', 115200)

    fps = 0
    target_coor = []
    lock = False  # whether found the best target or not
    track_init_frame = None

    try:

        while True:



            if lock == False:

                # starttime = time.time()
                '''get an image'''
                # Wait for a coherent pair of frames: depth and color
                frames = pipeline.wait_for_frames()
                # depth_frame = frames.get_depth_frame()
                color_frame = frames.get_color_frame()
                if not color_frame:
                    continue
                # Convert images to numpy arrays
                # depth_image = np.asanyarray(depth_frame.get_data())
                color_image = np.asanyarray(color_frame.get_data())  # obtain the image to detect armors


                """Do detection"""
                binary, frame = read_morphology(color_image)  # changed read_morphology()'s output from binary to mask

                potential_Targetsets = find_contours(binary, frame, fps) # get the list with all potential targets' info

                if potential_Targetsets: # if returned any potential targets
                    final_Target = targetsFilter(potential_Targetsets,frame) # filter out fake & bad targets and lock on single approachable target

                    depth = float(final_Target[0])
                    Yaw = float(final_Target[1])
                    Pitch = float(final_Target[2])
                    imgPoints = final_Target[3]

                    if (-30 < Pitch < 30) and (-45 < Yaw < 45):

                        '''
                        all encoded number got plus 50 in decimal: input(Yaw or Pitch)= -50, output(in deci)= 0
                        return list = [hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw, hex_sumAll]
                        '''
                        serial_lst = decimalToHexSerial(Yaw,Pitch)

                        #send_data(ser, serial_lst[0], serial_lst[1], serial_lst[2], serial_lst[3], serial_lst[4])

                        lock = True # Successfully detect one target
                        target_coor = [[int(imgPoints[0][0]), int(imgPoints[0][1])], [int(imgPoints[1][0]), int(imgPoints[1][1])], [int(imgPoints[2][0]), int(imgPoints[2][1])], [int(imgPoints[3][0]), int(imgPoints[3][1])]]#[bl,tl,tr,br]

                        track_init_frame = color_image

                    else:

                        print("!!! Angle exceed limit !!!")
                #else:
                    #send_data(ser, '01', '01','01','01','01')

            else:
                count = 0

                """Start Tracking"""
                tracker = cv2.legacy.TrackerCSRT_create()

                target_coor_tl_x = int(target_coor[1][0])
                target_coor_tl_y = int(target_coor[1][1])
                target_coor_width = abs(int(target_coor[2][0]) - int(target_coor[1][0]))
                target_coor_height = abs(int(target_coor[1][1]) - int(target_coor[0][1]))

                # bbox format:  (init_x,init_y,w,h)
                bbox = (target_coor_tl_x, target_coor_tl_y, target_coor_width, target_coor_height)

                #init the tracker with target detected frame & target coordinace
                success = tracker.init(track_init_frame, bbox)
                print(bbox)
                cv2.imshow("orhhhl", frame)
                cv2.waitKey(1)
                # track target for 10 frames
                while count < 10:

                    # Wait for a coherent pair of frames: depth and color
                    frames = pipeline.wait_for_frames()
                    # depth_frame = frames.get_depth_frame()
                    color_frame = frames.get_color_frame()
                    if not color_frame:
                        continue
                    # Convert images to numpy arrays
                    # depth_image = np.asanyarray(depth_frame.get_data())
                    frame = np.asanyarray(color_frame.get_data())  # obtain the image to detect armors



                    # Start timer
                    timer = cv2.getTickCount()

                    # Update tracker
                    success, bbox = tracker.update(frame)

                    # Solve angle
                    armor_tl_x = bbox[0] #bbox = (init_x,init_y,w,h)
                    armor_tl_y = bbox[1]
                    armor_bl_x = bbox[0]
                    armor_bl_y = bbox[1] + bbox[3]
                    armor_tr_x = bbox[0] + bbox[2]
                    armor_tr_y = bbox[1]
                    armor_br_x = bbox[0] + bbox[2]
                    armor_br_y = bbox[1] + bbox[3]
                    imgPoints = np.array(
                        [[armor_bl_x, armor_bl_y], [armor_tl_x, armor_tl_y], [armor_tr_x, armor_tr_y],
                         [armor_br_x, armor_br_y]], dtype=np.float64)
                    tvec, Yaw, Pitch = solve_Angle455(imgPoints)


                    # Calculate FPS
                    fps = cv2.getTickFrequency() / (cv2.getTickCount() - timer);

                    # Draw bounding box
                    if success:
                        # Tracking success
                        p1 = (int(bbox[0]), int(bbox[1]))
                        p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
                        cv2.rectangle(frame, p1, p2, (255, 0, 0), 2, 1)
                    else:
                        # Tracking failure
                        cv2.putText(frame, "Tracking failure detected", (600, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75,
                                    (0, 0, 255), 2)

                    # Display tracker type on frame
                    cv2.putText(frame, " Tracker", (600, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (50, 170, 50), 2);

                    # Display FPS on frame
                    cv2.putText(frame, "FPS : " + str(int(fps)), (600, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (50, 170, 50),
                                2);


                    cv2.circle(frame, (640, 360), 2, (255, 255, 255), -1)
                    cv2.putText(frame, 'Depth: ', (20, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, 'Yaw: ', (20, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, 'Pitch: ', (20, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, 'FPS: ', (20, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])


                    depth = str(tvec[2][0]) + 'mm'
                    cv2.putText(frame, depth, (90, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, str(Yaw), (90, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, str(Pitch), (90, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                    cv2.putText(frame, str(fps), (90, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])

                    cv2.imshow("original", frame)
                    cv2.waitKey(1)

                    count += 1

                lock = False





            cv2.circle(frame, (640, 360), 2, (255, 255, 255), -1)
            cv2.putText(frame, 'Depth: ', (20, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
            cv2.putText(frame, 'Yaw: ', (20, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
            cv2.putText(frame, 'Pitch: ', (20, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
            cv2.putText(frame, 'FPS: ', (20, 110), cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
            cv2.imshow("original", frame)

                #cv2.imshow("track_init_frame", track_init_frame)
            cv2.waitKey(1)

            #print(tvec, Yaw, Pitch)

            #endtime = time.time()
            #fps = 1 / (endtime - starttime)

    finally:

        # Stop streaming
        pipeline.stop()


if __name__ == "__main__":
    num = 0  # for collecting dataset, pictures' names
    """Declare your desired target color here"""
    targetColor = 1  # Red = 1 ; Blue = 0

    """init camera as cap, modify camera parameters at here"""
    # Configure depth and color streams
    pipeline = rs.pipeline()
    config = rs.config()

    # Get device product line for setting a supporting resolution
    pipeline_wrapper = rs.pipeline_wrapper(pipeline)
    pipeline_profile = config.resolve(pipeline_wrapper)
    device = pipeline_profile.get_device()
    device_product_line = str(device.get_info(rs.camera_info.product_line))

    found_rgb = False
    for s in device.sensors:
        if s.get_info(rs.camera_info.name) == 'RGB Camera':
            found_rgb = True
            break
    if not found_rgb:
        print("The demo requires Depth camera with Color sensor")
        exit(0)

    #config.enable_stream(rs.stream.depth, 640, 360, rs.format.z16, 30)

    if device_product_line == 'L500':  # if not D455
        config.enable_stream(rs.stream.color, 960, 540, rs.format.bgr8, 30)
    else:
        config.enable_stream(rs.stream.color, 1280, 720, rs.format.bgr8, 30)

    # Start streaming
    pipeline.start(config)

    # Get the sensor once at the beginning. (Sensor index: 1)
    sensor = pipeline.get_active_profile().get_device().query_sensors()[1]

    # Set the exposure anytime during the operation
    sensor.set_option(rs.option.exposure, 8.000)

    main()
