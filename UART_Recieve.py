# import serial
#
#
# def setUpSerial():
#     ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.5)
#     return ser
#
#
# '''
# Y:
# P:
# R:
#
# ...
#
# Y:
# P:
# R:
#
# ...
#
# Y:
# P:
# R:
#
# 1. buffer
# 2. FOV range due to resolution change
# 3.
#
# '''
#
#
# def get_imu(ser):
#     buffer_size = ser.in_waiting
#     imu_value = [None, None, None]
#
#     while True:
#         if ser.in_waiting > 0:
#             data = ser.readline().decode('latin-1').strip()
#             print(data)
#             #get bytes in line
#             #counter++
#             if data.startswith('Y:'):
#                 imu_value[0] = float(data.split(':')[1].strip())
#             elif data.startswith('P:'):
#                 imu_value[1] = float(data.split(':')[1].strip())
#             elif data.startswith('R:'):
#                 imu_value[2] = float(data.split(':')[1].strip())
#
#             # add if counter>=buffer_size
#             if imu_value[0] is not None and imu_value[1] is not None and imu_value[2] is not None :
#                 break
#     # return imu data, [0]:yaw,[1]:pitch,[2]:roll
#     return imu_value
#
#
# # Example usage
# if __name__ == '__main__':
#     ser = setUpSerial()
#     while True:
#         imu = get_imu(ser)
#         print(imu)

import serial
import time

ser = serial.Serial('/dev/ttyUSB0',115200,timeout=0.5)

ser.flushInput()

while True:
    if ser.in_waiting > 0:
        print(ser.read(ser.in_waiting))

    else:
        time.sleep(0.1)
