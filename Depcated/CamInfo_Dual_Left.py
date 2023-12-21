import cv2
import numpy as np
#cap = cv2.VideoCapture(1)

def undistort(frame):
    fx = 425.454564106161
    cx = 340.788184273818
    fy = 425.532941001197
    cy = 250.814007846180
    k1, k2, p1, p2, k3 = -0.422180200115741, 0.241627832004838, 0.0000000123449075101642, 0.000828155178622165, -0.0892381753236277

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
