import enum
import math
import pathlib
from dataclasses import dataclass
import cv2
import numpy as np
import serial
from UART_UTIL import send_data, get_imu
from camera_source import CameraSource
from kinematic_prediction import poly_predict

import argparse
import logging
import time
from camera_params import camera_params, DepthSource
from KalmanFilterClass import KalmanFilter
from Target import Target
#from sort import *

import struct

active_cam_config = None
frame_aligner = None


def nothing(x):
    pass


class TargetColor(enum.Enum):
    RED = 'red'
    BLUE = 'blue'


class CVParams:
    def __init__(self, target_color: TargetColor):
        self.target_color = target_color
        if target_color == TargetColor.RED:
            self.hue_min, self.hue_min_range = 4, (0, 180, 1)
            self.hue_max, self.hue_max_range = 30, (0, 180, 1)
            self.saturation_min, self.saturation_min_range = 218, (0, 255, 1)
            self.value_min, self.value_min_range = 111, (0, 255, 1)

            self.close_size = 1
            self.erode_size = 1
            self.dilate_size = 5
        else:
            self.hue_min, self.hue_min_range = 90, (0, 180, 1)
            self.hue_max, self.hue_max_range = 120, (0, 180, 1)
            self.saturation_min, self.saturation_min_range = 20, (0, 255, 1)
            self.value_min, self.value_min_range = 128, (0, 255, 1)

            self.close_size = 3
            self.erode_size = 2
            self.dilate_size = 2

        self.close_size_range = self.erode_size_range = self.dilate_size_range = (
            1, 20, 1)

        self.bar_aspect_ratio_min = 1.1
        self.bar_aspect_ratio_max = 13.0
        self.bar_z_angle_max = 20.0
        self.relative_x_delta_max = 3.0
        self.relative_y_delta_max = 3.0
        self.relative_height_diff_max = 0.5
        self.z_delta_max = 10.0


def createTrackbarsForParams(window_name: str, params: CVParams):
    for key, value in params.__dict__.items():
        if not key.endswith('_range') and type(value) in [int, float]:
            if hasattr(params, key + '_range'):
                slider_min, slider_max, scaling = getattr(
                    params, key + '_range')
            else:
                slider_min = 10 ** math.floor(math.log10(value))
                slider_max = 10 * slider_min
                scaling = 0.01

            cv2.createTrackbar(key, window_name, int(
                slider_min / scaling), int(slider_max / scaling), nothing)
            cv2.setTrackbarPos(key, window_name, int(value / scaling))


def updateParamsFromTrackbars(window_name: str, params: CVParams):
    for key, value in params.__dict__.items():
        if not key.endswith('_range') and type(value) in [int, float]:
            if hasattr(params, key + '_range'):
                scaling = getattr(params, key + '_range')[2]
            else:
                scaling = 0.01

            setattr(params, key, cv2.getTrackbarPos(
                key, window_name) * scaling)


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


# read cap and morphological operation to get led binary image.
def read_morphology(cap, config: CVParams):
    frame = cap
    # frame = unread_morphologydistort(frame)
    H, S, V = cv2.split(cv2.cvtColor(
        frame, cv2.COLOR_BGR2HSV))  # Split channels

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

    mask_processed = ((H >= config.hue_min) & (H <= config.hue_max) & (S >= config.saturation_min)
                      & (V >= config.value_min)).astype(np.uint8) * 255

    """
    combine Method 1 and 2 together; needed or not?
    """

    # combination = cv2.bitwise_and(maskRBG, mask)

    """
    Show difference between Method 1 and Method 2
    """
    # if debug:
    #     cv2.imshow("sub/threshold", mask_processed)
    #     #cv2.imshow("thresholded", mask)
    #
    #     """
    #     Morphological processing of the processed binary image
    #     """
    #     # open = cv2.getTrackbarPos('open', 'morphology_tuner') currently not needed
    #     close = cv2.getTrackbarPos('close', 'morphology_tuner')
    #     erode = cv2.getTrackbarPos('erode', 'morphology_tuner')
    #     dilate = cv2.getTrackbarPos('dilate', 'morphology_tuner')
    # dst_open = open_binary(mask, open, open) currently not needed
    dst_close = close_binary(
        mask_processed, config.close_size, config.close_size)
    dst_erode = erode_binary(dst_close, config.erode_size, config.erode_size)
    dst_dilate = dilate_binary(
        dst_erode, config.dilate_size, config.dilate_size)

    if debug:
        """
        Display the final image after preprocessing
        """
        cv2.imshow("erode", dst_dilate)

    return dst_dilate, frame


def spherical_to_cartesian(yaw: float, pitch: float, depth: float):
    phi = math.radians(90.0 - pitch)
    theta = math.radians(yaw)
    return depth * np.array([math.sin(phi) * math.cos(theta), math.sin(phi) * math.sin(theta), math.cos(phi)])


def cartesian_to_spherical(coords: np.ndarray):
    return (math.degrees(math.atan2(coords[1], coords[0])),
            90.0 -
            math.degrees(math.atan2(
                math.sqrt(coords[0] ** 2 + coords[1] ** 2), coords[2])),
            np.linalg.norm(coords).item())

def rotationMatrixToEulerAngles(R):
    sy = np.sqrt(R[0,0] * R[0,0] +  R[1,0] * R[1,0])
    singular = sy < 1e-6
    if not singular:
        x = np.arctan2(R[2,1], R[2,2])
        y = np.arctan2(-R[2,0], sy)
        z = np.arctan2(R[1,0], R[0,0])
    else:
        x = np.arctan2(-R[1,2], R[1,1])
        y = np.arctan2(-R[2,0], sy)
        z = 0
    return np.array([x, y, z])

def get_3d_target_location(imgPoints, frame, depth_frame):
    # Retrieve the camera's data & distortion coefficients from config
    camera_matrix, distort_coeffs = np.array(active_cam_config['camera_matrix'], dtype=np.float64), \
        np.array(active_cam_config['distort_coeffs'], dtype=np.float64)

    # Undistort the given image points
    imgPoints = cv2.undistortPoints(
        imgPoints, camera_matrix, distort_coeffs, P=camera_matrix)[:, 0, :]

    # Calculate the average (center) point of the image points
    center_point = np.average(imgPoints, axis=0)

    # Calculate the offset of the center point from the camera's optical center
    center_offset = center_point - \
        np.array([active_cam_config['cx'], active_cam_config['cy']])
    center_offset[1] = -center_offset[1]

    # Convert the offset to angular measurements (yaw and pitch) in degrees
    angles = np.rad2deg(np.arctan2(center_offset, np.array(
        [active_cam_config['fx'], active_cam_config['fy']])))

    # Calculate depth based on the configured depth source
    if active_cam_config['depth_source'] == DepthSource.PNP:
        # Define the real-world dimensions of the object for Perspective-n-Point (PnP) depth calculation
        width_size_half = 70  # half width of the object
        height_size_half = 62.5  # half height of the object

        # Object points in real world coordinates
        objPoints = np.array([[-width_size_half, -height_size_half, 0],
                              [width_size_half, -height_size_half, 0],
                              [width_size_half, height_size_half, 0],
                              [-width_size_half, height_size_half, 0]], dtype=np.float64)

        # Use solvePnP_IPPE method to find the object's pose and calculate the norm of the translation vector for depth
        retval, rvec, tvec = cv2.solvePnP(
            objPoints, imgPoints, camera_matrix, distort_coeffs, flags=cv2.SOLVEPNP_IPPE)
        
        # Get Depth value
        meanDVal = np.linalg.norm(tvec[:, 0])
        Yaw = np.arctan(tvec[(0,0)]/ tvec[(2,0)]) / 2 / 3.1415926535897932 * 360
        Pitch = -(np.arctan(tvec[(1, 0)] / tvec[(2, 0)]) / 2 / 3.1415926535897932 * 360)

        



    elif active_cam_config['depth_source'] == DepthSource.STEREO:
        # Ensure the depth frame is available for stereo depth calculation
        assert depth_frame is not None

        # Create a mask from the image points and scale it to match the depth frame size
        panel_mask = np.zeros(frame.shape[:2], dtype=np.uint8)
        cv2.drawContours(panel_mask, [imgPoints.astype(
            np.int64)], -1, 1, thickness=cv2.FILLED)
        panel_mask_scaled = cv2.resize(
            panel_mask, (depth_frame.shape[1], depth_frame.shape[0]))

        # Calculate the mean depth value within the masked area
        meanDVal, _ = cv2.meanStdDev(depth_frame, mask=panel_mask_scaled)
    else:
        # Throw an error if an invalid depth source is configured
        raise RuntimeError('Invalid depth source in camera config')

    # Store and return the calculated depth, yaw, pitch, and image points
    target_Dict = {"depth": meanDVal,
                   "Yaw": Yaw, "Pitch": Pitch, "imgPoints": imgPoints}
    return target_Dict


@dataclass
class ImageRect:
    points: np.ndarray

    @property
    def center(self):
        return np.average(self.points, axis=0)

    @property
    def width_vec(self):
        return np.average(self.points[2:, :], axis=0) - np.average(self.points[:2, :], axis=0)

    @property
    def width(self):
        return np.linalg.norm(self.width_vec)

    @property
    def height_vec(self):
        return np.average(self.points[(0, 3), :], axis=0) - np.average(self.points[(1, 2), :], axis=0)

    @property
    def height(self):
        return np.linalg.norm(self.height_vec)

    @property
    def angle(self):
        return 90.0 - np.rad2deg(np.arctan2(self.height_vec[1], self.height_vec[0]))


# find contours and main screening section
def find_contours(config: CVParams, binary, frame, depth_frame, fps):
    global num
    contours, heriachy = cv2.findContours(
        binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    first_data = []  # include all potential light bar's contourArea information dict by dict
    second_data = []
    # all potential target's [depth,yaw,pitch,imgPoints(np.array([[bl], [tl], [tr],[br]]))]
    potential_Targets = []
    # target_Dict = dict() # per target's [depth,yaw,pitch,imgPoints(np.array([[bl], [tl], [tr],[br]]))]

    if len(contours) > 0:
        # collect info for every contour's rectangle
        for i, contour in enumerate(contours):
            area = cv2.contourArea(contour)

            # area < 5 will not be considered as armor board, unit is pixel^2
            if area < 5:
                continue

            rect = cv2.minAreaRect(contour)
            # coordinates of the four vertices of the rectangle
            coor = cv2.boxPoints(rect).astype(np.int32)

            rect_param = findVerticesOrder(coor)  # output order: [bl,tl,tr,br]
            rect = ImageRect(rect_param)

            cv2.circle(frame, rect.points[0], 9,
                       (255, 255, 255), -1)  # test armor_tr
            cv2.circle(frame, rect.points[1], 9,
                       (0, 255, 0), -1)  # test armor_tl
            # test bottom left
            cv2.circle(frame, rect.points[2], 9, (255, 255, 0), -1)
            # test bottom left
            cv2.circle(frame, rect.points[3], 9, (0, 100, 250), -1)

            """filer out undesired rectangle, only keep lightBar-like shape"""
            """90--->45-->0-center(horizontally)->90-->45-->0"""
            aspect_ratio = rect.height / rect.width
            if (aspect_ratio >= config.bar_aspect_ratio_min) and (aspect_ratio <= config.bar_aspect_ratio_max) \
                    and (rect.angle <= config.bar_z_angle_max and rect.angle >= -config.bar_z_angle_max):  # filer out undesired rectangle, only keep lightBar-like shape

                first_data.append(rect)
                box = np.int0(coor)
                # test countor minRectangle
                cv2.drawContours(frame, [box], -1, (255, 0, 0), 3)

        for i in range(len(first_data)):
            nextRect = i + 1
            while nextRect < len(first_data):
                c = first_data[i]
                n = first_data[nextRect]
                if (abs(c.center[1] - n.center[1]) <= config.relative_y_delta_max * ((c.height + n.height) / 2)) \
                        and (abs(c.height - n.height) <= config.relative_height_diff_max * max(c.height, n.height)) \
                        and (abs(c.center[0] - n.center[0]) <= config.relative_x_delta_max * ((c.height + n.height) / 2)) \
                        and (abs(c.angle - n.angle)) < config.z_delta_max:
                    second_data.append((first_data[i], first_data[nextRect]))

                nextRect = nextRect + 1
        
        for r1, r2 in second_data:  # find vertices for each
            # if find potential bounded lightbar formed targets
            if abs(r1.points[0][1] - r2.points[2][1]) <= 3 * (abs(r1.points[0][0] - r2.points[2][0])):
                left_bar, right_bar = (
                    r1, r2) if r1.points[3][0] <= r2.points[3][0] else (r2, r1)
                left_side_vec = (left_bar.points[0] - left_bar.points[1]) / 2
                right_side_vec = (left_bar.points[3] - left_bar.points[2]) / 2

                '''Prepare rect 4 vertices array and then pass it to (1) solve_Angle455's argument (2) number detection'''
                imgPoints = np.array(
                    [left_bar.points[0] + left_side_vec, left_bar.points[1] - left_side_vec,
                     right_bar.points[2] - right_side_vec, right_bar.points[3] + right_side_vec],
                    dtype=np.float64)
                target_Dict = get_3d_target_location(
                    imgPoints, frame, depth_frame)

                target = Target(target_Dict)
                
                potential_Targets.append(target)

                if debug:
                    '''collecting data set at below'''
                    armboard_width = 27
                    armboard_height = 25

                    coordinate_before = np.float32(imgPoints)
                    # coordinate_after is in the order of imgPoints (bl,tl,tr,br)
                    coordinate_after = np.float32([[0, armboard_height], [0, 0], [armboard_width, 0],
                                                   [armboard_width, armboard_height]])

                    # Compute the transformation matrix
                    trans_mat = cv2.getPerspectiveTransform(
                        coordinate_before, coordinate_after)
                    # Perform transformation and show the result
                    trans_img = cv2.warpPerspective(
                        frame, trans_mat, (armboard_width, armboard_height))

                    trans_img = np.array(
                        255 * (trans_img / 255) ** 0.5, dtype='uint8')

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
                    cv2.imshow("dila_img", gray_img)

                num += 1
                cv2.putText(frame, "Potentials:", (int(imgPoints[2][0]), int(imgPoints[2][1]) - 5), cv2.FONT_HERSHEY_SIMPLEX,
                            0.5, [255, 255, 255])
                center = np.average(imgPoints, axis=0).astype(np.int32)
                # draw the center of the detected armor board
                cv2.circle(frame, center, 2, (0, 0, 255), -1)

        return potential_Targets


def targetsFilter(potential_Targetsets, frame, last_target_x):
    
    # if only one target, return it directly
    if len(potential_Targetsets) == 1:
        return potential_Targetsets[0] # the only target class object
    
    '''
    target with Number & greatest credits wins in filter process
    Credit Consideration: Area, Depth, Pitch, Yaw
    Credit Scale: 1 - 3
    '''
    max_Credit = 0
    best_Target = []  # order: [depth, Yaw, Pitch, imgpoints]
    all_distance_diff = []  # every target's x-axis distance between last target

    # if the target from last frame exists, filter out the closest one to keep tracking on the same target
    if last_target_x != None:
        for target in potential_Targetsets:
            imgPoints = target.imgPoints

            # current target's x-axis in a 1280*720 frame
            curr_target_x = imgPoints[0][0] + \
                (imgPoints[2][0] - imgPoints[0][0]) / 2

            all_distance_diff.append(abs(curr_target_x - last_target_x))
        sort_index = np.argsort(all_distance_diff)  # order: small to large
        closest_target = potential_Targetsets[sort_index[0]]
        return closest_target

    # if the target from last frame doesn't exist, filter out the best one based on credits
    for target in potential_Targetsets:
        depth = float(target.depth)
        Yaw = float(target.yaw)
        Pitch = float(target.pitch)
        imgPoints = target.imgPoints

        # current target's x-axis in a 1280*720 frame
        curr_target_x = imgPoints[0][0] + \
            (imgPoints[2][0] - imgPoints[0][0]) / 2

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
        """Assess distance between last target & current target"""

        """Assess Depth"""
        if depth < 1800:
            depth_Credit += 5
        elif depth < 2500:
            depth_Credit += 3

        """Assess Angle"""

        if abs(Yaw) < 5 or abs(Pitch) < 10:
            angle_Credit += 100
        elif abs(Yaw) < 10 or abs(Pitch) < 15:
            angle_Credit += 3
        elif abs(Yaw) < 20 or abs(Pitch) < 20:
            angle_Credit += 2
        elif abs(Yaw) < 30 or abs(Pitch) < 30:
            angle_Credit += 1

        """evaluate score"""
        if (depth_Credit + angle_Credit) > max_Credit:
            max_Credit = (depth_Credit + angle_Credit)
            best_Target = target

    return best_Target


def clipRect(rect_xywh, size):
    x, y, w, h = rect_xywh
    clipped_x, clipped_y = min(max(x, 0), size[0]), min(max(y, 0), size[1])
    return clipped_x, clipped_y, min(max(w, 0), size[0] - clipped_x), min(max(h, 0), size[1] - clipped_y)


def findVerticesOrder(pts):
    ''' sort rectangle points by clockwise '''
    # sort y-axis only
    sort_x = pts[np.argsort(pts[:, 1]), :]
    # get top 2 [x,y]
    Bottom = sort_x[2:, :]  # bot

    Top = sort_x[:2, :]  # top

    # Bottom sort: Bottom[0] = bl ;  Bottom[1] = br
    Bottom = Bottom[np.argsort(Bottom[:, 0]), :]

    # Top sort: Top[0] = tl ; Top[1] = tr
    Top = Top[np.argsort(Top[:, 0]), :]

    return np.stack([Bottom[0], Top[0], Top[1], Bottom[1]], axis=0)


def float_to_hex(f):
    ''' turn float to hex'''
    return ''.join([f'{byte:02x}' for byte in struct.pack('>f', f)])

def decimalToHexSerial(Yaw, Pitch):
    # 将Yaw和Pitch转换为IEEE 754标准的四字节浮点数表示，并转换为十六进制字符串
    # turn Yaw and Pitch to IEEE 754 standard four-byte floating point representation and convert to hexadecimal string
    hex_Yaw = float_to_hex(Yaw)
    hex_Pitch = float_to_hex(Pitch)

    # 计算校验和
    # calculate checksum
    bytes_for_checksum = struct.pack('>ff', Yaw, Pitch) # only checked Yaw & Pitch data so far
    checksum = sum(bytes_for_checksum) % 256
    hex_checksum = f'{checksum:02x}'

    # 构建十六进制数据列表
    # build hexadecimal data list
    return hex_Yaw, hex_Pitch, hex_checksum

def draw_crosshair(frame):
    height, width = frame.shape[:2]
    center_x, center_y = width // 2, height // 2
    color = (0, 255, 0)  # Green color
    thickness = 2
    size = 20
    cv2.line(frame, (center_x, center_y - size), (center_x, center_y + size), color, thickness)
    cv2.line(frame, (center_x - size, center_y), (center_x + size, center_y), color, thickness)
    return frame

def main(camera: CameraSource, target_color: TargetColor, show_stream: str):
    """
    Important commit updates: umature pred-imu; 50 deg limit; HSV red adj; get_imu; MVS arch rebuild ---- Shiao
    """
    cv_config = CVParams(target_color)

    # Create a window for CV parameters if debug mode is active
    if debug:
        cv2.namedWindow("CV Parameters")
        createTrackbarsForParams("CV Parameters", cv_config)
        cv2.resizeWindow("CV Parameters", 800, 180)

    '''Initialize variables for tracking and prediction'''

    fps = 0
    target_coor = []
    lock = False                    # Flag to indicate if the best target is found
    track_init_frame = None
    last_target_x = None
    last_target_y = None
    # success = False
    tracker = None
    tracking_frames = 0
    max_tracking_frames = 15        # Maximum number of frames to track

    max_history_length = 8          # Maximum number of samples for prediction
    # Time in seconds to predict the target's motion into the future
    prediction_future_time = 0.2
    '''
    Maximum time in seconds between history frames
    Should be long enough for a dropped frame or two,
    but not too long to group unrelated detections
    '''
    max_history_frame_delta = 0.15
    target_angle_history = []

    # Try to Open serial port for data transmission to STM32, if not found, continue without it
    try:
        ser = serial.Serial('/dev/ttyUSB0', 115200)
    except:
        ser = None
        print("Serial port not found, continuing without it")

    detect_success = False
    track_success = False

    while True:
        "to calculate fps"
        startTime = time.time()

        if debug:
            updateParamsFromTrackbars("CV Parameters", cv_config)

        color_image, depth_image = camera.get_frames()
        
        # color_image with crosshair
        color_image = draw_crosshair(color_image)
        
        """Do detection"""
        binary, frame = read_morphology(color_image, cv_config)

        # get the list with all potential targets' info
        potential_Targetsets = find_contours(cv_config, binary, frame, depth_image, fps)

        if potential_Targetsets: # if dectection success
            
            detect_success = True

            # filter out the best target
            final_Target = targetsFilter(potential_Targetsets, frame, last_target_x)

            #extract the target's position and angle
            depth = float(final_Target.depth)
            Yaw = float(final_Target.yaw)
            Pitch = float(final_Target.pitch)
            imgPoints = final_Target.imgPoints

            '''SORT tracking'''

        else: # if detection failed
            """Prepare Tracking"""

            detect_success = False
            if tracker is not None and tracking_frames < max_tracking_frames:
                tracking_frames += 1
                # Update tracker
                track_success, bbox = tracker.update(color_image)
            else:
                track_success = False

            """if Tracking success, Solve Angle & Draw bounding box"""
            if track_success:
                # Solve angle
                target_coor_width = abs(
                    int(final_Target.topRight[0]) - int(final_Target.topLeft[0]))
                target_coor_height = abs(
                    int(final_Target.topLeft[1]) - int(final_Target.bottomLeft[1]))
                
                # bbox format:  (init_x,init_y,w,h)
                bbox = (final_Target.topLeft[0] - target_coor_width * 0.05, final_Target.topLeft[1], target_coor_width * 1.10,
                        target_coor_height) # to enlarge the bbox to include the whole target, for better tracking by KCF or others
                
                bbox = clipRect(bbox, (color_image.shape[1], color_image.shape[0])) # clip the bbox to fit the frame
                
                armor_tl_x = bbox[0]  # bbox = (init_x,init_y,w,h)
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
                target_Dict = get_3d_target_location(
                    imgPoints, color_image, depth_image)
                
                final_Target.depth = target_Dict["depth"]
                final_Target.yaw = target_Dict["Yaw"]
                final_Target.pitch = target_Dict["Pitch"]
                final_Target.imgPoints = target_Dict["imgPoints"]
                # depth = float(tvec[2][0])

                '''draw tracking bouding boxes'''
                p1 = (int(bbox[0]), int(bbox[1]))
                p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
                cv2.rectangle(frame, p1, p2, (0, 255, 0), 2, 1)


        if detect_success or track_success:

            # store the current target's x-axis, used for detection in the next round
            last_target_x = imgPoints[0][0] + \
                (imgPoints[2][0] - imgPoints[0][0])/2

            '''
            Do Prediction
            '''

            if ser is not None:
                imu_yaw, imu_pitch, imu_roll = get_imu(ser)
                print(f"imu data receive: {imu_yaw}, {imu_pitch}, {imu_roll}")
            else:
                imu_yaw, imu_pitch, imu_roll = 20,20,20  # for test

            
            imu_yaw *= -1.2
            imu_pitch *= -1.2
            global_yaw = imu_yaw + Yaw
            global_pitch = imu_pitch + Pitch

            cartesian_pos = (spherical_to_cartesian(global_yaw, global_pitch, depth) -
                             np.array(camera.active_cam_config['camera_offset']))


            if (-30 < Pitch < 30) and (-45 < Yaw < 45):
                cv2.line(frame, (int(imgPoints[1][0]), int(imgPoints[1][1])),
                         (int(imgPoints[3][0]), int(imgPoints[3][1])),
                         (33, 255, 255), 2)
                cv2.line(frame, (int(imgPoints[2][0]), int(imgPoints[2][1])),
                         (int(imgPoints[0][0]), int(imgPoints[0][1])),
                         (33, 255, 255), 2)
                cv2.putText(frame, str(depth), (90, 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                cv2.putText(frame, str(Yaw), (90, 50),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
                cv2.putText(frame, str(Pitch), (90, 80),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])

                # """test Kalman Filter"""
                # X = int((imgPoints[0][0] + imgPoints[2][0]) / 2)
                # Y = int((imgPoints[0][1] + imgPoints[2][1]) / 2)

                current_time = time.time()
                if len(target_angle_history) < 1 or current_time - target_angle_history[-1][0] > max_history_frame_delta:
                    target_angle_history = [(current_time, *cartesian_pos)]
                else:
                    target_angle_history.append((current_time, *cartesian_pos))

                if len(target_angle_history) > max_history_length:
                    target_angle_history = target_angle_history[-max_history_length:]

                if len(target_angle_history) >= 2:
                    time_hist_array, x_hist_array, y_hist_array, z_hist_array =\
                        np.array([item[0] for item in target_angle_history]), \
                        np.array([item[1] for item in target_angle_history]), \
                        np.array([item[2] for item in target_angle_history]), \
                        np.array([item[3] for item in target_angle_history])

                    time_hist_array -= time_hist_array[0]

                    degree = 1  # if len(target_angle_history) == 2 else 2

                    weights = np.linspace(float(max_history_length) - len(
                        time_hist_array) + 1.0, float(max_history_length) + 1.0, len(time_hist_array))
                    predicted_x = poly_predict(time_hist_array, x_hist_array, degree,
                                               time_hist_array[-1] + prediction_future_time, weights=weights)
                    predicted_y = poly_predict(time_hist_array, y_hist_array, degree,
                                               time_hist_array[-1] + prediction_future_time, weights=weights)
                    predicted_z = poly_predict(time_hist_array, z_hist_array, degree,
                                               time_hist_array[-1] + prediction_future_time, weights=weights)

                    predicted_yaw, predicted_pitch, _ = cartesian_to_spherical(
                        np.array([predicted_x, predicted_y, predicted_z]))

                    """
                    set threshold to the predicted Yaw & Pitch: currently No More Than -50 or 50 degree
                    """

                else:
                    predicted_yaw, predicted_pitch = global_yaw, global_pitch

                current_point_coords = (int(active_cam_config['fx'] * math.tan(math.radians(Yaw)) + active_cam_config['cx']),
                                        int(active_cam_config['fy'] * math.tan(math.radians(-Pitch)) + active_cam_config['cy']))
                predicted_point_coords = (int(active_cam_config['fx'] * math.tan(math.radians(predicted_yaw - imu_yaw)) + active_cam_config['cx']),
                                          int(active_cam_config['fy'] * math.tan(math.radians(-(predicted_pitch - imu_pitch))) + active_cam_config['cy']))
                # cv2.circle(frame, predicted_point_coords, 4, (0, 255, 0), -1)
                cv2.line(frame, current_point_coords,
                         predicted_point_coords, (255, 255, 255), 2)
                '''
                all encoded number got plus 50 in decimal: input(Yaw or Pitch)= -50, output(in deci)= 0
                return list = [hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw, hex_sumAll]
                '''

                relative_pred_yaw = predicted_yaw - imu_yaw
                relative_pred_pitch = predicted_pitch - imu_pitch

                if relative_pred_pitch > 50:
                    relative_pred_pitch = 50
                elif relative_pred_pitch < -50:
                    relative_pred_pitch = -50
                if relative_pred_yaw > 50:
                    relative_pred_yaw = 50
                elif relative_pred_yaw < -50:
                    relative_pred_yaw = -50


                Yaw = np.deg2rad(Yaw)
                Pitch = np.deg2rad(Pitch)

                print(f"imu data send: {Yaw}, {Pitch}, {detect_success}")

                hex_Yaw, hex_Pitch, hex_checksum = decimalToHexSerial(
                    Yaw, Pitch)

                if ser is not None:
                    send_data(
                        ser, hex_Yaw, hex_Pitch, hex_checksum,detect_success)

            else:

                logger.warning(
                    f"Angle(s) exceed limits: Pitch: {Pitch}, Yaw: {Yaw}")

        else:
            # Tracking failure
            cv2.putText(frame, "Tracking failure detected", (600, 80),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)
            # send failure data(send 0 degree to make gimbal stop)
            if ser is not None:
                hex_Yaw, hex_Pitch, hex_checksum=decimalToHexSerial(0, 0)
                send_data(ser, hex_Yaw, hex_Pitch, hex_checksum,detect_success)


        cv2.circle(frame, (720, 540), 2, (255, 255, 255), -1)
        cv2.putText(frame, 'Depth: ', (20, 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
        cv2.putText(frame, 'Yaw: ', (20, 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
        cv2.putText(frame, 'Pitch: ', (20, 80),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
        cv2.putText(frame, 'FPS: ', (20, 110),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])
        cv2.putText(frame, str(fps), (90, 110),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, [0, 255, 0])


        if show_stream == 'YES':
            cv2.imshow("original", frame)
            cv2.waitKey(1)
        else:
            pass


        endtime = time.time()
        fps = 1 / (endtime - startTime)
        # print(fps)


if __name__ == "__main__":
    # set up argument parser
    parser = argparse.ArgumentParser()
    parser.add_argument('--target-color', required=True, type=str, choices=[val.value for val in TargetColor],
                        help='The armor board light color to detect')
    parser.add_argument('--recording-source', type=pathlib.Path,
                        help='Path to input video recordings')
    parser.add_argument('--recording-dest', type=pathlib.Path,
                        help='Path to record camera video to (MP4 format)')
    parser.add_argument('--debug', action='store_true',
                        help='Show intermediate results and debug output')
    parser.add_argument('--show-stream', type=str, choices=['YES', 'NO'], default='NO',
                        help='Display the camera stream (YES or NO)')


    args = parser.parse_args()

    # set up logger
    logger = logging.getLogger(__name__)
    debug: bool = args.debug
    logger.setLevel('DEBUG' if debug else 'INFO')

    args.target_color = TargetColor(args.target_color)
    num = 0  # for collecting dataset, pictures' names

    # choose camera params
    camera = CameraSource(camera_params['HIK MV-CS016-10UC General'], args.target_color.value,
                          recording_source=args.recording_source, recording_dest=args.recording_dest)
    
    active_cam_config = camera.active_cam_config
    main(camera, args.target_color, args.show_stream)
