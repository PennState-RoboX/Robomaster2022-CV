import serial #Please Use PySetial package
import time
def setUpSerial():
    ser = serial.Serial('/dev/ttyTHS2', 115200)

def send_data(ser,angleA, angleB, angleC): #Angles are in Byte Formact
    packet = 'a5' #Header Byte 
    packet = packet + '5a'
    packet = packet + '07' # Length of Packet
    packet = packet + angleA
    packet = packet + angleB
    packet = packet + angleC
    packet = packet + 'ff'
    print(packet) # Packat before Conversion
    packet = bytes.fromhex(packet)
    print(packet) #Packet Get Sent
    ser.write(packet)

#Below are example of how to use this utility
ser = serial.Serial('/dev/ttyTHS2', 115200)

while 1==1 :
	
	#send_data(ser,b'\x0b',b'\x09',b'\x06')
	send_data(ser,'01','09','06')
	time.sleep(5)

