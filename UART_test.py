import serial

def setUpSerial():
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.5)
    return ser

def parse_data(data):
    values = {'Y': None, 'P': None, 'R': None}
    lines = data.split('\n')
    for line in lines:
        if line.startswith('Y:'):
            values['Y'] = line.split(':')[1].strip()
        elif line.startswith('P:'):
            values['P'] = line.split(':')[1].strip()
        elif line.startswith('R:'):
            values['R'] = line.split(':')[1].strip()
    return values

def receive_data(ser):
    value=None;
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()
            values = parse_data(data)
        print("Received:", values)

def main():
    ser = setUpSerial()
    receive_data(ser)

if __name__ == '__main__':
    main()
