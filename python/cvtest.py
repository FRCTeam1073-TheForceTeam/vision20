import numpy as np
import cv2
import math


# Capture Video and set resolution from gstreamer pipeline
capture = cv2.VideoCapture("nvarguscamerasrc do-timestamp=true ! video/x-raw(memory:NVMM),format=(string)NV12,width=(int)1920,height=(int)1080,framerate=30/1 ! nvvidconv flip-method=0 ! video/x-raw,width=(int)640,height=(int)360,format=(string)BGRx ! videoconvert ! video/x-raw, format(string)BGR ! appsink")


# Video output streaming to gstreamer pipeline
output = cv2.VideoWriter("appsrc ! videoconvert ! video/x-raw,format=(string)NV12 ! omxh264enc control-rate=2 bitrate=400000 profile=1 preset-level=1 ! video/x-h264, framerate=30/1, stream-format=(string)byte-stream ! h264parse ! rtph264pay config-interval=1 mtu=1000 ! udpsink host=127.0.0.1 port=5801", cv2.CAP_GSTREAMER, 30, (640,360))


if capture.isOpened():
    print "Capture input pipeline created successfully..."
else:
    print "Capture input pipeline creation failed!"

if output.isOpened():
    print "Video output pipeline created successfully..."
else:
    print "Video output pipeline creation failed!"

while(True):

    # Capture frame-by-frame
    ret, frame = capture.read()
    
    frame2 = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(frame2, 50, 150, None, 3)
#    cv2.imshow('edges', edges)

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

            
#    cv2.imshow('frame', frame)

    output.write(frame);

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
capture.release()
cv2.destroyAllWindows()
