import serial
import time
def setUpSerial():
    ser = serial.Serial('/dev/ttyTHS2', 115200, timeout = 0.5)
    #ser.open()

def send_data(ser,hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw, sumAll): #Angles are in Byte Formact
    #packet = b'\x0d' #header
    #packet = packet + angleA
    #packet = packet + angleB
    #packet = packet + angleC
    #packet = packet + b'\x03' #not used
    #packet = packet + b'\x04'
    #packet = packet + b'\x00'
    #packet = packet + b'\x04'
    #packet = packet + b'\x00'
    #packet = packet + b'\x04'
    #packet = packet + b'\x00'
    packet = 'a5' #Header Byte
    packet = packet + '5a'
    packet = packet + '09' # Length of Packet
    packet = packet + hex_int_Pitch
    packet = packet + hex_deci_Pitch
    packet = packet + hex_int_Yaw
    packet = packet + hex_deci_Yaw
    packet = packet + sumAll
    packet = packet + 'ff'
    print(packet) # Packat before Conversion
    packet = bytes.fromhex(packet)
    print(packet) #Packet Get Sent
    ser.write(packet)


#Below are example of how to use this utility
'''
ser = serial.Serial('/dev/ttyTHS2', 115200)

while 1==1 :
	
	#send_data(ser,b'\x0b',b'\x09',b'\x06')
	send_data(ser,'01','09','06')
	time.sleep(5)

'''