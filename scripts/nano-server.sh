#!/bin/sh


gst-launch-1.0 -v nvarguscamerasrc sensor-id=1 ! \
'video/x-raw(memory:NVMM),format=(string)NV12, width=(int)1280, height=(int)720, framerate=30/1' ! \
nvvidconv flip-method=0 ! \
'video/x-raw(memory:NVMM), width=(int)640, height=(int)360, format=(string)I420' ! \
omxh264enc control-rate=2 bitrate=250000 profile=1 preset-level=1 ! \
'video/x-h264, framerate=30/1, stream-format=(string)byte-stream' ! h264parse ! rtph264pay config-interval=1 ! \
udpsink host=$1 port=5000
