# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import numpy as np

def order_points(pts):
    ''' sort rectangle points by clockwise '''
    sort_x = pts[np.argsort(pts[:, 0]), :]

    Left = sort_x[:2, :]
    Right = sort_x[2:, :]
    # Left sort
    Left = Left[np.argsort(Left[:, 1])[::-1], :]
    # Right sort
    Right = Right[np.argsort(Right[:, 1]), :]

    return np.concatenate((Left, Right), axis=0)

img = np.zeros((512, 512, 3), dtype=np.uint8)
pts = np.array([[1.02,1],[3,3],[10.74540,0],[0,10]], dtype=np.float64)
pts = order_points(pts)
print(pts)


# See PyCharm help at https://www.jetbrains.com/help/pycharm/
print(z)