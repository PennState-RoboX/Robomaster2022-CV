class Target:
    def __init__(self, target_dict):
        self.depth = target_dict.get("depth")
        self.yaw = target_dict.get("Yaw")
        self.pitch = target_dict.get("Pitch")
        self.imgPoints = target_dict.get("imgPoints")

    # Method to display target information
    def display_info(self):
        print(f"Depth: {self.depth}, Yaw: {self.yaw}, Pitch: {self.pitch}")
        print(f"Image Points: {self.imgPoints}")
