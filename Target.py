class Target:
    # This main class is used to store the target information in a dictionary
    # format of the target_dict is- {"depth": x, "Yaw": y, "Pitch": z, "imgPoints": [[0, 0], [0, 0], [0, 0], [0, 0]]}
    
    def __init__(self, target_dict):
        self.depth = target_dict.get("depth")
        self.yaw = target_dict.get("Yaw")
        self.pitch = target_dict.get("Pitch")
        self._imgPoints = target_dict.get("imgPoints")

    @property
    def imgPoints(self):
        # simply return the image points from the dict
        return self._imgPoints

    @imgPoints.setter
    def imgPoints(self, value):
        # set the image points from the dict
        self._imgPoints = value

    @property
    def target_coor(self):
        # return the target coordinates in the format of [bottemLeft, topLeft,topRight, bottomRight]
        # sample output: [[0, 0], [0, 0], [0, 0], [0, 0]]
        #                   bl      tl      tr      br
        return [[int(self.imgPoints[0][0]), int(self.imgPoints[0][1])],
                [int(self.imgPoints[1][0]), int(self.imgPoints[1][1])],
                [int(self.imgPoints[2][0]), int(self.imgPoints[2][1])],
                [int(self.imgPoints[3][0]), int(self.imgPoints[3][1])]]  # [bl,tl,tr,br]

    @property
    def bottomLeft(self):
        return self.target_coor[0]

    @property
    def topLeft(self):
        return self.target_coor[1]

    @property
    def topRight(self):
        return self.target_coor[2]

    @property
    def bottomRight(self):
        return self.target_coor[3]
