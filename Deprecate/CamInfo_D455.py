import cv2
import numpy as np
#cap = cv2.VideoCapture(1)

def undistort(frame):
    fx = 645.455984328821
    cx = 643.077674664939
    fy = 644.606305889468
    cy = 357.730289611374
    k1, k2, p1, p2, k3 = -0.0557535647706463, 0.0538700601952326, -0.000454149012521474, 0.00119677524381670, 0.0

    # 相机坐标系到像素坐标系的转换矩阵
    k = np.array([
        [fx, 0, cx],
        [0, fy, cy],
        [0, 0, 1]
    ], dtype=np.float64)
    # 畸变系数
    d = np.array([
        k1, k2, p1, p2, k3
    ])
    h, w = frame.shape[:2]
    mapx, mapy = cv2.initUndistortRectifyMap(k, d, None, k, (w, h), 5)
    return cv2.remap(frame, mapx, mapy, cv2.INTER_LINEAR)

'''
while(cap.isOpened()):
    ret, frame = cap.read()
   # frame =
    cv2.imshow('frame', undistort(frame))

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()
'''
