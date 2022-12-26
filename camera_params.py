import enum
import math

import numpy as np


class DepthSource(enum.IntEnum):
    STEREO = 0
    PNP = 1


camera_params = {
    'Generic Webcam': {
        'frame_rate': 30,
        'exposure': {'red': 45.0, 'blue': 20.0},
        'capture_res': (1280, 720),
        'depth_source': DepthSource.PNP,
        'fov': (90, 65)
    },
    'Intel RealSense D435I': {
        'frame_rate': 30,
        'exposure': {'red': 15.0, 'blue': 35.0},
        'fov': (69, 42),
        'capture_res': (960, 540),
        'depth_source': DepthSource.STEREO
    },
    'Intel RealSense D455': {
        'frame_rate': 30,
        'exposure': {'red': 45.0, 'blue': 20.0},
        'capture_res': (1280, 720),
        'depth_source': DepthSource.STEREO,
        'fov': (90, 65),
        'cx': 643.077674664939,
        'cy': 357.730289611374,
        'fx': 645.455984328821,
        'fy': 644.606305889468,
        'k1': -0.0557535647706463,
        'k2': 0.0538700601952326,
        'p1': -0.000454149012521474,
        'p2': 0.00119677524381670,
        'k3': 0.0
    }
}

for cam_name, param_dict in camera_params.items():
    if 'cx' not in param_dict:
        # Use naive camera matrix parameters
        param_dict.update({
            'cx': param_dict['capture_res'][0] / 2,
            'cy': param_dict['capture_res'][1] / 2,
            'fx': (param_dict['capture_res'][0] / 2) / math.tan((param_dict['fov'][0] / 2) * math.pi / 180.0),
            'fy': (param_dict['capture_res'][1] / 2) / math.tan((param_dict['fov'][1] / 2) * math.pi / 180.0),
            'k1': 0.0,
            'k2': 0.0,
            'p1': 0.0,
            'p2': 0.0,
            'k3': 0.0
        })
    param_dict['camera_matrix'] = np.array([[param_dict['fx'], 0, param_dict['cx']],
                                            [0, param_dict['fy'], param_dict['cy']],
                                            [0, 0, 1] ],
                                           dtype=np.float64)

    param_dict['distort_coeffs'] = np.array([param_dict['k1'], param_dict['k2'],
                                             param_dict['p1'], param_dict['p2'],
                                             param_dict['k3']])
