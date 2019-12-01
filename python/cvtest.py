import numpy as np
import cv2
import math


# Capture Video and set resolution
capture = cv2.VideoCapture("nvarguscamerasrc do-timestamp=true ! video/x-raw(memory:NVMM),format=(string)NV12,width=(int)1920,height=(int)1080,framerate=30/1 ! nvvidconv flip-method=0 ! video/x-raw,width=(int)320,height=(int)180,format=(string)BGRx ! videoconvert ! video/x-raw, format(string)BGR ! appsink");


if capture.isOpened():
    print "Capture opened successfully...."

while(True):

    # Capture frame-by-frame
    ret, frame = capture.read()
    
    frame2 = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(frame2, 50, 150, None, 3)
    cv2.imshow('edges', edges)

    lines = cv2.HoughLines(edges,2, np.pi/90.0, 120)
    
    if lines is not None:
        for i in range(0, len(lines)):
            rho = lines[i][0][0]
            theta = lines[i][0][1]
            a = math.cos(theta)
            b = math.sin(theta)
            x0 = a * rho
            y0 = b * rho
            pt1 = (int(x0 + 1000*(-b)), int(y0 + 1000*(a)))
            pt2 = (int(x0 - 1000*(-b)), int(y0 - 1000*(a)))
            cv2.line(frame, pt1, pt2, (0,0,200), 3, cv2.LINE_AA)

            
    cv2.imshow('frame', frame) 

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
capture.release()
cv2.destroyAllWindows()
