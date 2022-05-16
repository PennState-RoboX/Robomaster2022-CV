import cv2
import numpy as np


def solve_Angle455(imgPoints):
    width_size_half = 50.0
    height_size_half = 32.5
    fx = 324.714304343673
    cx = 316.498636446160
    fy = 324.197628291452
    cy = 236.288203358557
    k1, k2, p1, p2, k3 = -0.0561272094560157, 0.0613986698843776, 0.0, 0.0, 0.0

    K = np.array([[fx, 0, cx],
                  [0, fy, cy],
                  [0, 0, 1] ],
                 dtype=np.float64)

    objPoints = np.array([[-width_size_half, -height_size_half, 0],
                          [width_size_half, -height_size_half, 0],
                          [width_size_half, height_size_half, 0],
                          [-width_size_half, height_size_half, 0]], dtype=np.float64)
    #imgPoints #= np.array([[608, 167], [514, 167], [518, 69], [611, 71]], dtype=np.float64)
    cameraMatrix = K
    distCoeffs = np.array([k1, k2, p1, p2, k3])
    retval,rvec,tvec  = cv2.solvePnP(objPoints, imgPoints, cameraMatrix, distCoeffs)
    # cv2.Rodrigues()
    print (retval, rvec, tvec)
    print("Yaw: ",tvec[(0,0)]/ tvec[(2,0)] / 2 / 3.1415926535897932 * 360)
    print("Pitch: ",tvec[(1, 0)] / tvec[(2, 0)] / 2 / 3.1415926535897932 * 360)
