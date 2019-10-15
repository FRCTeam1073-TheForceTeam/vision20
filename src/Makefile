#
# This makefile builds the gstvision library which wraps gstreamer
# pipeline creation and management in a C++ helper class that allows
# you to write simple vision-processing callback functions.
#

# Pull in the CXX flags for gstreamermm-1.0 library
CXXFLAGS +=  $(shell pkg-config --cflags --libs gstreamer-1.0)
CXXFLAGS += -fPIC -I. -L.

all: libgstvision.a basic_test vision_test

# Build gstvision wrapper library:
libgstvision.a: gstvision.cpp gstvision.h
	g++ -c -Wall $(<) $(CXXFLAGS)
	ar rvs $@ gstvision.o 

# Build basic test of gstvision wrapper library:
basic_test: basic_test.cpp libgstvision.a
	g++ -c -Wall $(<) $(CXXFLAGS)
	g++ basic_test.o -o $@  -lgstvision $(CXXFLAGS)  -lgstapp-1.0

vision_test: vision_test.cpp libgstvision.a
	g++ -c -Wall $(<) $(CXXFLAGS)
	g++ vision_test.o -o $@  -lgstvision $(CXXFLAGS)  -lgstapp-1.0

clean:
	rm -f libgstvision.a *.o
	rm -f basic_test
	rm -f vision_test
	rm -f *~
