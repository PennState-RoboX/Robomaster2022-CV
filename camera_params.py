import math

camera_params = {
    'Intel RealSense D435I': {
        'exposure': {'red': 15.0, 'blue': 5.0},
        'fov': (69, 42),
        'capture_res': (960, 540)
    },
    'Intel RealSense D455': {
        'exposure': {'red': 8.0, 'blue': 8.0},
        'capture_res': (1280, 720),
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
