import serial
import time
import re


def setUpSerial():
    # ser = serial.Serial('/dev/ttyTHS0', 115200, timeout=0.5)
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1.0)
    return ser


# Angles are in Byte Format
def send_data(ser, hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw, sumAll):
    # packet = b'\x0d' #header
    # packet = packet + angleA
    # packet = packet + angleB
    # packet = packet + angleC
    # packet = packet + b'\x03' #not used
    # packet = packet + b'\x04'
    # packet = packet + b'\x00'
    # packet = packet + b'\x04'
    # packet = packet + b'\x00'
    # packet = packet + b'\x04'
    # packet = packet + b'\x00'
    packet = 'a5'  # Header Byte
    packet = packet + '5a'
    packet = packet + '08'  # Length of Packet
    packet = packet + hex_int_Pitch
    packet = packet + hex_deci_Pitch
    packet = packet + hex_int_Yaw
    packet = packet + hex_deci_Yaw
    # packet = packet + fire_command
    packet = packet + sumAll
    packet = packet + 'ff'
    # print(packet)  # Packat before Conversion
    packet = bytes.fromhex(packet)
    # print(packet)  # Packet Get Sent
    ser.write(packet)

def get_imu(ser):

    ser.flushInput()
    buffer_size = ser.in_waiting
    imu_value = [None,None,None]
    counter = 0

    while True:
        raw_data = ser.read()

        data = raw_data.decode('latin-1').strip()
        counter += len(raw_data)  # Count bytes

        try:
            if data.startswith('Y:'):
                imu_value[0] = float(data.split(' ')[0].strip('Y:'))
                imu_value[1] = float(data.split(' ')[1].strip('P:'))
                imu_value[2] = float(data.split(' ')[2].strip('R:'))
                print(imu_value)
                return imu_value
        except:
            continue  # If we can't convert the value to a float, skip this line

        # if all(value is not None for value in imu_value):
        #     # If we have a complete set of IMU data and we have processed all data in the buffer
        #     # then we can break the loop
        #     if counter >= buffer_size:
        #         break

    # # return imu data, [0]:yaw,[1]:pitch,[2]:roll
    # return imu_value




# # Example usage
if __name__ == '__main__':
    ser = setUpSerial()
    while True:
        imu = get_imu(ser)
        print(imu)


# Below are example of how to use this utility
'''
ser = serial.Serial('/dev/ttyTHS2', 115200)

while 1==1 :
	
	#send_data(ser,b'\x0b',b'\x09',b'\x06')
	send_data(ser,'01','09','06')
	time.sleep(5)

'''
