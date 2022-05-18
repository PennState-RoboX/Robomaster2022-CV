import cv2
import numpy as np
import math

def solve_Angle455(imgPoints):
    width_size_half = 70 # small armor board's width(include light bar's width)
    height_size_half = 62.5 #  small armor board's height
    fx = 645.455984328821
    cx = 643.077674664939
    fy = 644.606305889468
    cy = 357.730289611374
    k1, k2, p1, p2, k3 = -0.0557535647706463, 0.0538700601952326, -0.000454149012521474, 0.00119677524381670, 0.0

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
    print("Yaw: ",np.arctan(tvec[(0,0)]/ tvec[(2,0)]) / 2 / 3.1415926535897932 * 360)
    print("Pitch: ",np.arctan(tvec[(1, 0)] / tvec[(2, 0)]) / 2 / 3.1415926535897932 * 360)
