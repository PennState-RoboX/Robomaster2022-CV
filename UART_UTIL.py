import serial
import time
import re


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
    imu_value = [0, 0, 0]
    if not ser.is_open:
        return imu_value
    
    while (True):
        print(ser.in_waiting)
        raw_data = ser.read(100)
        data = raw_data.decode('utf-8', 'replace')
        print(len(raw_data))  # Count bytes

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
    return imu_value


# # Example usage
if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB0',115200)  
    while True:
        imu = get_imu(ser)
        print(imu)

