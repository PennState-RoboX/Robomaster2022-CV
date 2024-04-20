# This File is used to send data to the IMU and get data from the IMU
# The IMU is connected to the board via UART
# The IMU is used to get the Pitch and Yaw angles of the board

import serial # The serial library is used to communicate with the IMU
import time
import re


# Angles are in Byte Format
def send_data(ser, hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw, sumAll): # this function is called in ArmorDetect_D435i.py

    ## ! Explaination of send_data function
    ## ser is the serial object
    ## hex_int_Pitch is the integer part of the Pitch angle
    ## hex_deci_Pitch is the decimal part of the Pitch angle
    ## hex_int_Yaw is the integer part of the Yaw angle
    ## hex_deci_Yaw is the decimal part of the Yaw angle
    ## sumAll is the sum of all the bytes in the packet

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
    packet = packet + hex_int_Pitch # Pitch Angle
    packet = packet + hex_deci_Pitch # Pitch Angle (decimal part)
    packet = packet + hex_int_Yaw # Yaw Angle
    packet = packet + hex_deci_Yaw # Yaw Angle (decimal part)
    # packet = packet + fire_command
    packet = packet + sumAll
    packet = packet + 'ff' # End Byte
    # print(packet)  # Packat before Conversion
    packet = bytes.fromhex(packet)
    # print(packet)  # Packet Get Sent
    ser.write(packet) # Send the packet to the IMU


def get_imu(ser): # this function is called in ArmorDetect_D435i.py

    ## ! Explaination of get_imu function
    ## ser is the serial object
    ## The function reads the data from the IMU
    ## The function returns the Pitch and Yaw angles of the board

    imu_value = [0, 0, 0] # Default value
    if not ser.in_waiting or ser.in_waiting == 0: 
        # If there is no data in the buffer return the default value
        return imu_value

    while (True): # Loop to read the data from the IMU
        print(ser.in_waiting) # Count bytes
        raw_data = ser.read(100) # Read the data from the IMU
        data = raw_data.decode('utf-8', 'replace') # Decode the data

        if 'A5' in data: # Check if the data contains the 'A5' header we made earlier
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
    ser = serial.Serial('/dev/ttyUSB0', 115200) # Open the serial port
    while True:
        imu = get_imu(ser)
        print(imu)
