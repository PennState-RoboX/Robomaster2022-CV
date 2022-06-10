import serial
import time
def setUpSerial():
    ser = serial.Serial('/dev/ttyTHS2', 115200)
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
    packet = 'A5'
    packet = packet + '5A'
    packet = packet + '09'
    packet = packet + hex_int_Pitch
    packet = packet + hex_deci_Pitch
    packet = packet + hex_int_Yaw
    packet = packet + hex_deci_Yaw
    packet = packet + sumAll
    packet = packet + 'FF'
    print(packet)
    packet = bytes.fromhex(packet)
    print(packet)
    ser.write(packet)

#ser = setUpSerial()
#send_data(ser, "0x00")
#send_data(ser, "0x06")
'''
ser = serial.Serial('/dev/ttyTHS2', 115200)

while 1==1 :
	
	#send_data(ser,b'\x0b',b'\x09',b'\x06')
	send_data(ser,'01','09','06')
	time.sleep(5)

'''