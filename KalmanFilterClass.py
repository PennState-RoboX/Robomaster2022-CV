import cv2
import numpy as np


class KalmanFilter:
    kf = cv2.KalmanFilter(4, 2)
    kf.measurementMatrix = np.array([[1, 0, 0, 0], [0, 1, 0, 0]], np.float32)
    kf.transitionMatrix = np.array([[1, 0, 1, 0], [0, 1, 0, 1], [0, 0, 1, 0], [0, 0, 0, 1]], np.float32)


    def predict(self, frames=1):
        ''' This function estimates the position of the object'''
        #measured = np.array([[np.float32(coordX)], [np.float32(coordY)]])
        #self.kf.correct(measured)
        origPostState = self.kf.statePost

        for _ in range(frames):
            predicted = self.kf.predict()
            self.kf.statePost = self.kf.statePre

        self.kf.statePost = origPostState
        x, y = int(predicted[0]), int(predicted[1])
        return x, y

    def correct(self,coordX, coordY):
        '''This function correct the current state base on the previous state'''
        measured = np.array([[np.float32(coordX)], [np.float32(coordY)]])
        self.kf.correct(measured)
