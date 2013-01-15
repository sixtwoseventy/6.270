CC = gcc
CXX = g++
RM = rm
MAKE = make
CFLAGS = -std=c99 $(DEBUG)
CXXFLAGS = $(DEBUG)
HEADERS = vision_mock1.h projection.h serial.h util.h game.h packet.h table.h goals.h
VISION_MOCK1_SOURCES = projection.c serial.c goals.c vision_mock1.c util.c table.c
VISION_MOCK1_OBJS = $(VISION_MOCK1_SOURCES:.c=.o)
CALIBRATE_OBJS = calibrate.o
CIRCLEDETECT_OBJS = circledetect.o
OUTPUTS = vision_mock1 calibrate circledetect

UNAME := $(shell uname)
#ifeq ($(UNAME), Linux)
LOCATION=/usr/local
#LIBS=-lm -lcv -lcxcore -lhighgui -lcvaux
LIBS=-lm -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_calib3d
CFLAGS += -D LINUX
#endif
#ifeq ($(UNAME), Darwin)
#LOCATION=/usr
#LIBS=-lm -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_calib3d
#CFLAGS += -D DARWIN
#endif

CFLAGS += -I $(LOCATION)/include/opencv/
CXXFLAGS += -I $(LOCATION)/include/opencv/

%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OUTPUTS)

vision_mock1: $(VISION_MOCK1_OBJS)
	$(CC) $(CFLAGS) $(VISION_MOCK1_OBJS) -o $@ -I $(LOCATION)/include/opencv/ -L$(LOCATION)/lib $(LIBS)

circledetect: $(CIRCLEDETECT_OBJS)
	$(CC) $(CFLAGS) $(CIRCLEDETECT_OBJS) -o $@ -I $(LOCATION)/include/opencv/ -L$(LOCATION)/lib $(LIBS)

debug:
	$(MAKE) DEBUG="-ggdb -D DEBUG"

calibrate: $(CALIBRATE_OBJS)
	$(CXX) $(CXXFLAGS) $(CALIBRATE_OBJS) -o $@ -L$(LOCATION)/lib $(LIBS)

.PHONY: clean
clean:
	$(RM) -r *.o $(OUTPUTS)
