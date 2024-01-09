import math
import cv2
import sys

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
'''
Yaw = 45165
Pitch = 30.71
if (-30 < Pitch <= 30.99) and (-45 < Yaw <= 45.99):
    hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw,sumAll = decimalToHexSerial(Yaw,Pitch)
    print(hex_int_Pitch, hex_deci_Pitch, hex_int_Yaw, hex_deci_Yaw)
    print(45+30+100+65+71)
'''



if __name__ == '__main__':

    tracker = cv2.legacy.TrackerCSRT_create()

    # Read video
    video = cv2.VideoCapture(1)


    # Read first frame.
    ok, frame = video.read()

    # Define an initial bounding box
    bbox = (222, 222, 111, 222)

    # Initialize tracker with first frame and bounding box
    success = tracker.init(frame, bbox)

    while True:
        # Read a new frame
        success, frame = video.read()
        if not success:
            break

        # Start timer
        timer = cv2.getTickCount()

        # Update tracker
        success, bbox = tracker.update(frame)

        # Calculate Frames per second (FPS)
        fps = cv2.getTickFrequency() / (cv2.getTickCount() - timer);

        # Draw bounding box
        if success:
            # Tracking success
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(frame, p1, p2, (255, 0, 0), 2, 1)
        else:
            # Tracking failure
            cv2.putText(frame, "Tracking failure detected", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)

        # Display tracker type on frame
        cv2.putText(frame, " Tracker", (100, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (50, 170, 50), 2);

        # Display FPS on frame
        cv2.putText(frame, "FPS : " + str(int(fps)), (100, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (50, 170, 50), 2);

        # Display result
        cv2.imshow("Tracking", frame)

        # Exit if ESC pressed
        k = cv2.waitKey(1) & 0xff
        if k == 27: break