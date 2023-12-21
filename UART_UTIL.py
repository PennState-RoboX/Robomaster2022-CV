import serial
import time
import re


def setUpSerial():
    ser = serial.Serial('/dev/ttyTHS0', 115200) # Direct Connection
    # ser = serial.Serial('/dev/ttyUSB0', 115200) # sometime the USB ID change for no reason, switch this two line accordingly
    # ser = serial.Serial('/dev/ttyUSB1', 115200)
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
    packet = bytes.fromhex(packet)
    ser.write(packet)
    return
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
    print(f"hex_int_Pitch: {hex_int_Pitch}")
    print(f"hex_deci_Pitch: {hex_deci_Pitch}")
    print(f"hex_int_Yaw: {hex_int_Yaw}")
    print(f"hex_deci_Yaw: {hex_deci_Yaw}")
    print(sumAll)




def get_imu(ser):
    # buffer_size = ser.in_waiting # sometime the in_waiting returns 0, but there's still data coming in
    imu_value = [None,None,None]
    counter = 0

    while True:
        raw_data = ser.read(4095)
        data = raw_data.decode('utf-8','replace')
        counter += len(raw_data)  # Count bytes



        if 'A5' in data:
            try:
                start = data.index('A5') + 2  # Skip the 'A5'
                end = data.index('A5', start)  # Find the next 'A5' or end of the string
                imu_readings = data[start:end].split(',')

                # Convert the first three elements to floats and return them
                if len(imu_readings) >= 3:
                    imu_value = [float(x) for x in imu_readings[:3]]
                    return imu_value
            except Exception as e:
                print(f"Error processing IMU data: {e}")
                continue
                # Handle the exception or continue the loop






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
