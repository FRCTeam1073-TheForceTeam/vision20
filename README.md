# Vision20

Nanovision code developed in the 2019 off season and 2020 preseason

# Introduction

Nanovision is a new concept for the team where we utilize the new
Jetson Nano as a vision co-processor instead of Raspberry Pi. It is
sligtly more expensive but tremendously more powerful as a processor
and this can allow us to do more video /vision processing on the the
robot.

# Hardware Outline

## The Jetson Nano Developmenet Board
The hardware for the Nanovision setup is the off-the-shelf JetsonNano
baord, available for $99:

[The Jetson Nano Development Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)

The NVIDIA website has a wealth of resources available for the boards
and full access requires you sign up for a free account.

## SD Card Storage
The baord requires a high-quality micro-SD card to be its "disk"
storage and for that we use the following cards:

[128GB SD Cards](https://www.amazon.com/gp/product/B06XWZWYVP/ref=ppx_yo_dt_b_asin_title_o06_s01?ie=UTF8&psc=1)

Given the extremely tough environment of living in an FRC robot we
don't recommend skimping on the cards or other hardware parts or they
may fail on you in a match.

## Robot Capable Power Supply
In order to survive being part of an FRC robot you need to provide
very solid, unwavering power to this co-processor and you have to do
this by using the board as a "custom circuit". It should have 18 Guage
power wires coming from a snap-breaker protected circuit to a local
power regulator on the board. The power on a robot is a very messy
environment and you need a steady 5VDC supply for the Jetson Nano.
For this we use the following buck-boost regulator board from Pololu
for about $15.	

[Buck/Boost Power Module](https://www.pololu.com/product/2574)

This allows the board to tolerate the huge power swings seen in a
defensive robot power system. This can only supply about 10W of 
power to the Jetson Nano so you can't use its maximum capabilities
without a beefier power supply.

## Camera Modules
One cool part of the JetsonNano is that it is hardware compatible with
RaspberryPi Camera Modules and the clones out there. This gives you a
really nice collection of MIPI based camera modules that you can
choose from and flexibility to pick modules with different types of
lenses for different situations. We use a wide-angle lens setup on the
hardware MIPI camera port in our setups. We use the following basic
drive camera on the MIPI camera port in our setup:

[Jetson Nano / Raspberry Pi Wide Angle Camera](https://www.amazon.com/IMX219-160-Camera-Resolution-Degree-Angle-View-IMX219/dp/B07SQ92SC7/ref=sr_1_3?keywords=Jetson+Nano+Camera&qid=1569800148&s=electronics&sr=1-3)

The JetsonNano can also work with any USB/UVC compatible camera
modules and has 4x USB 3.0 ports for cameras or other things and it
has the horsepower to capture, compress, process and transmit many
video streams simultaneously.


## Development Case
We also recommend getting a development system for the desktop in
addition to any systems you embed into your robot setup. For these
there is an excellent desktop case that helps with experimentation and
camera development in a nice, clean package:

[Jetson Nano Development Case](https://www.amazon.com/gp/product/B07VWXP88M/ref=ppx_yo_dt_b_asin_title_o06_s00?ie=UTF8&psc=1)


# Software Outline

The JetsonNano is an Ubuntu based Linux machine running Linux on the
ARMv8 or "64-bit ARM" architecture. Because of this it has an enormous
and powerful collection of open-source software available for it. The
details of software setup are included in our notes directory.

The JetsonNano includes many specialized hardware "processing
accelerators" that allow you to do more than just the main processor
can do alone. These are accessed through libraries that support the
open-source Gstreamer-1.0 system.

## GStreamer Software
By using open-source and custom NVIDIA Gstreamer-1.0 plugins you can
use tools to create video capture and processing pipelines on your
JetsonNano setups.

Our first, basic setups for the JetsonNano are simply bash scripts
that use gst-launch-1.0 to launch a gstreamer pipeline of software
modules that take advantage of the hardware on the JetsonNano and
capture, compress and transmit video to our driver station very
efficiently. 

GStreamer is also available for windows and we load GStreamer onto our
driver station and have GStreamer scripts that allow us to view the
streams from our JetsonNano in real-time.

Once we get the basic opertions working, we can create dedicated
GStreamer applications (this means programming in C or C++) and we can
use GStreamer to provide 'hooks' into the hardware-accelerated video
stream to add in custom processing steps.



