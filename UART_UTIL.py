import serial
import time
import re


# Angles are in Byte Format
def send_data(ser, hex_Yaw, hex_Pitch, checksum, detect_success: bool):
    # Convert detect_success from boolean to hex string ('01' for True, '00' for False)
    hex_detect_success = '01' if detect_success else '00'
    
    # Construct the packet
    packet = 'a5'  # Header Byte
    packet += '5a'
    packet += '0d'  # Length of Packet, 11 bytes
    packet += hex_Yaw
    packet += hex_Pitch
    packet += checksum
    packet += hex_detect_success
    packet += 'ff'
    
    # Convert the hexadecimal string to bytes
    packet_bytes = bytes.fromhex(packet)
    # Write the bytes to the serial port
    ser.write(packet_bytes)


def get_imu(ser):
    imu_value = [0, 0, 0]
    if not ser.in_waiting or ser.in_waiting == 0:
        return imu_value

    while (True):
        # print(ser.in_waiting)
        raw_data = ser.read(100)
        data = raw_data.decode('utf-8', 'replace')
        # print(len(raw_data))  # Count bytes

        if 'A5' in data:
            try:
                start = data.index('A5') + 2  # Skip the 'A5'
                end = data.index('A5', start)  # Find the next 'A5' or end of the string
                imu_readings = data[start:end].split(',')

                # Convert the first three elements to floats and return them
                if len(imu_readings) >= 3:
                    imu_value = [float(x) for x in imu_readings[:3]]
                    # print(imu_value)
                    return imu_value
            except Exception as e:
                print(f"Error processing IMU data: {e}")
                continue
                # Handle the exception or continue the loop
    return imu_value


# # Example usage
if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB0', 115200)
    while True:
        imu = get_imu(ser)
        print(imu)
        # send_data()
