#
# This makefile builds the gstvision library which wraps gstreamer
# pipeline creation and management in a C++ helper class that allows
# you to write simple vision-processing callback functions.
#

# Pull in the CXX flags for gstreamermm-1.0 library
CXXFLAGS +=  $(shell pkg-config --cflags --libs gstreamer-1.0)
CXXFLAGS += -fPIC -I.

all: libgstvision.so basic_test

# Build gstvision wrapper library:
libgstvision.so: gstvision.cpp gstvision.h
	g++ -c -Wall $(<) $(CXXFLAGS)
	g++ -shared gstvision.o $(CXXFLAGS) -o $@

# Build basic test of gstvision wrapper library:
basic_test: basic_test.cpp libgstvision.so
	g++ -c -Wall $(<) $(CXXFLAGS)
	g++ basic_test.o -o $@ libgstvision.so

clean:
	rm -f libgstvision.so *.o
	rm -f basic_test
	rm -f *~
