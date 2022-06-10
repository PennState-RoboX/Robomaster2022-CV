import math


def decimalToHexSerial(Yaw, Pitch):
    '''for int part'''
    int_Pitch = int(Pitch + 50) # for check sum
    hex_int_Pitch = str(hex(int_Pitch))  # form: 0xa; encode -45 degree to -45+50=5 degree
    hex_int_Pitch = ('0' + hex_int_Pitch[2:])[-2:]  # delete '0x'

    int_Yaw = int(Yaw + 50) # for check sum
    hex_int_Yaw = str(hex(int_Yaw))  # encode -45 degree to -45+50=5 degree
    hex_int_Yaw = ('0' + hex_int_Yaw[2:])[-2:]

    '''for decimal part to serial hex: input -314.159 output ===> 10'''
    deci_Pitch = format(math.modf(abs(Pitch))[0], '.2f')  # decimal part is always positive
    str_deci_Pitch = str(deci_Pitch)[-2:]
    int_deci_Pitch = int(str_deci_Pitch)  # for check sum
    hex_deci_Pitch = str(hex(int_deci_Pitch))
    hex_deci_Pitch = ('0' + hex_deci_Pitch[2:])[-2:]  # to transfer by serial; form:314.159 => 16

    deci_Yaw = format(math.modf(abs(Yaw))[0], '.2f')  # decimal part is always positive
    str_deci_Yaw = str(deci_Yaw)[-2:]
    int_deci_Yaw = int(str_deci_Yaw)  # for check sum
    hex_deci_Yaw = str(hex(int_deci_Yaw))
    hex_deci_Yaw = ('0' + hex_deci_Yaw[2:])[-2:]  # to transfer by serial; form:314.159 => 16

    int_sumAll = int_Pitch + int_Yaw + int_deci_Pitch + int_deci_Yaw
    hex_sumAll = str(hex(int_sumAll))
    hex_sumAll = ('0' + hex_sumAll[2:])[-2:]  #delete '0x'

    return hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw,hex_sumAll
Yaw = 45.65
Pitch = 30.71
if (-30 < Pitch <= 30.99) and (-45 < Yaw <= 45.99):
    hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw,sumAll = decimalToHexSerial(Yaw,Pitch)
    print(hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw)
    print(sumAll+1)
    print(45+30+100+65+71)