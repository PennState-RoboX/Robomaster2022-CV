import can
import serial

#init CAN and uart
bus = can.bus(interface='socketcan',channel='can0',receive_own_messages=True)
ser = serial.Serial('/dev/tty0',115200)

#send message
message = can.Message(arbitration_id = 123, is_extended_id=True,data =[0x11])
bus.send(message,timeout=0.2)

ser.write(0x01)


#recieve message
for msg in bus:
    print("{X}: {}".format(msg.arbitriation_id, msg.data))

#recommanded notify method for recieving
notifier = can.Notifier(bus, [canLogger("recorded.log"), can.Printer()])

while ser.in_waiting():
    print(ser.readline())
