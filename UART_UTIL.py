import serial
import time
def setUpSerial():
    ser = serial.Serial('/dev/ttyTHS2', 115200)
    #ser.open()

def send_data(ser,angleA, angleB, angleC): #Angles are in Byte Formact
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
    packet = '0d'
    packet = packet + angleA
    packet = packet + angleB
    packet = packet + angleC
    packet = packet + '02'
    packet = packet + '0a'
    print(packet)
    packet = bytes.fromhex(packet)
    print(packet)
    ser.write(packet)

#ser = setUpSerial()
#send_data(ser, "0x00")
#send_data(ser, "0x06")
ser = serial.Serial('/dev/ttyTHS2', 115200)

while 1==1 :
	
	#send_data(ser,b'\x0b',b'\x09',b'\x06')
	send_data(ser,'01','09','06')
	time.sleep(5)

