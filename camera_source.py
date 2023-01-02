import json
import logging
from pathlib import Path
from typing import Dict, Tuple, Optional

import cv2
import numpy as np

from camera_params import camera_params, DepthSource

logger = logging.getLogger(__name__)

RS_DEPTH_CAPTURE_RES = (640, 480)

# Unified image acquisition class for different types of cameras
class CameraSource:
    def __init__(self, default_config: Dict, target_color: str, cv_device_index: int = 0,
                 recording_source: Optional[Path] = None, recording_dest: Optional[Path] = None):
        assert recording_source is None or recording_dest is None
        self._rs_pipeline = None
        self._rs_frame_aligner = None
        self._cv_color_cap = None
        self._cv_depth_cap = None

        if recording_source is None:
            self.active_cam_config = default_config

            try:
                import pyrealsense2 as rs

                # Configure depth and color streams
                pipeline = rs.pipeline()
                config = rs.config()

                # Get device product line for setting a supporting resolution
                pipeline_wrapper = rs.pipeline_wrapper(pipeline)
                pipeline_profile = config.resolve(pipeline_wrapper)
                device = pipeline_profile.get_device()
                device_name = str(device.get_info(rs.camera_info.name))

                if device_name in camera_params:
                    self.active_cam_config = camera_params[device_name]
                else:
                    logger.warning(f'Unknown device name: "{device_name}". Falling back to default configuration.')

                config.enable_stream(rs.stream.color, self.active_cam_config['capture_res'][0],
                                     self.active_cam_config['capture_res'][1], rs.format.bgr8, self.active_cam_config['frame_rate'])

                if self.active_cam_config['depth_source'] == DepthSource.STEREO:
                    config.enable_stream(rs.stream.depth, RS_DEPTH_CAPTURE_RES[0], RS_DEPTH_CAPTURE_RES[1], rs.format.z16,
                                         self.active_cam_config['frame_rate'])
                    frame_aligner = rs.align(rs.stream.color)
                else:
                    frame_aligner = None

                # Start streaming
                pipeline.start(config)

                # Get the sensor once at the beginning. (Sensor index: 1)
                sensor = pipeline.get_active_profile().get_device().query_sensors()[1]

                # Set the exposure anytime during the operation
                sensor.set_option(rs.option.exposure, self.active_cam_config['exposure'][target_color])

                self._rs_pipeline = pipeline
                self._rs_frame_aligner = frame_aligner
            except ImportError:
                logger.warning('Intel RealSense backend is not available; pyrealsense2 could not be imported')
            except RuntimeError as ex:
                if len(ex.args) >= 1 and 'No device connected' in ex.args[0]:
                    logger.warning('No RealSense device was found')
                else:
                    raise

            if self._rs_pipeline is None:
                cap = cv2.VideoCapture()  # the number here depends on your device's camera, usually default with 0
                cap.open(cv_device_index)

                cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
                # os.system('v4l2-ctl --device=/dev/video1 --set-ctrl=exposure_auto=2')
                cap.set(cv2.CAP_PROP_EXPOSURE, self.active_cam_config['exposure'][target_color])
                cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.active_cam_config['capture_res'][0])
                cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.active_cam_config['capture_res'][1])
                cap.set(cv2.CAP_PROP_FPS, self.active_cam_config['frame_rate'])
                self._cv_color_cap = cap
        else:
            cam_config_path = recording_source.parent / (recording_source.stem + '.config.json')
            with open(cam_config_path, 'r', encoding='utf8') as cam_config_file:
                self.active_cam_config = json.load(cam_config_file)

            depth_frame_path = recording_source.parent / (recording_source.stem + '.depth' + recording_source.suffix)
            self._cv_color_cap = cv2.VideoCapture(str(recording_source))
            self._cv_depth_cap = cv2.VideoCapture(str(depth_frame_path))

        self.color_frame_writer = self.depth_frame_writer = None
        if recording_dest is not None:
            cam_config_path = recording_dest.parent / (recording_dest.stem + '.config.json')
            with open(cam_config_path, 'w', encoding='utf8') as cam_config_file:
                json.dump(self.active_cam_config, cam_config_file)

            codec = cv2.VideoWriter.fourcc('m', 'p', '4', 'v')
            self.color_frame_writer = cv2.VideoWriter(str(recording_dest), codec, self.active_cam_config['frame_rate'],
                                                      self.active_cam_config['capture_res'])
            if self._rs_pipeline is not None:
                depth_frame_path = recording_dest.parent / (recording_dest.stem + '.depth' + recording_dest.suffix)
                self.depth_frame_writer = cv2.VideoWriter(str(depth_frame_path), codec, self.active_cam_config['frame_rate'],
                                                          RS_DEPTH_CAPTURE_RES)

    def get_frames(self) -> Tuple[Optional[np.ndarray], Optional[np.ndarray]]:
        if self._rs_pipeline is not None:
            frames = self._rs_pipeline.wait_for_frames()

            if self._rs_frame_aligner is not None:
                frames = self._rs_frame_aligner.process(frames)

            # depth_frame = frames.get_depth_frame()
            color_frame = frames.get_color_frame()
            if color_frame:
                color_image = np.asanyarray(color_frame.get_data())
            else:
                color_image = None

            depth_frame = frames.get_depth_frame()
            if depth_frame:
                depth_image = np.asanyarray(depth_frame.get_data())
            else:
                depth_image = None
        elif self._cv_color_cap is not None:
            ret, color_image = self._cv_color_cap.read()
            if not ret:
                color_image = None

            if self._cv_depth_cap is None:
                depth_image = None
            else:
                ret, depth_image = self._cv_depth_cap.read()
                if ret:
                    B, G, _ = cv2.split(depth_image)
                    depth_image = (B.astype(np.uint16) << 8) + G.astype(np.uint16)
                else:
                    depth_image = None
        else:
            raise RuntimeError('No image source available')

        # test = cv2.split(cv2.resize(color_image, RS_DEPTH_CAPTURE_RES))
        # depth_image = (test[0].astype(np.uint16) << 8) + test[2].astype(np.uint16)

        if self.color_frame_writer is not None and color_image is not None:
            self.color_frame_writer.write(color_image)

        if self.depth_frame_writer is not None and depth_image is not None:
            storage_format_image = cv2.merge([(depth_image >> 8).astype(np.uint8), (depth_image % 256).astype(np.uint8),
                                              np.zeros_like(depth_image, dtype=np.uint8)])
            self.depth_frame_writer.write(storage_format_image)

        return color_image, depth_image

    def __del__(self):
        if self._rs_pipeline is not None:
            self._rs_pipeline.stop()

        if self.color_frame_writer is not None:
            self.color_frame_writer.release()

        if self.depth_frame_writer is not None:
            self.depth_frame_writer.release()
