TARGET = bin/raceya

LIBS =  -lglut -lGLU -lGL -lSDL -lSDLmain -lSDL_mixer -lSDL_image -ljpeg -lboost_filesystem-mt -lboost_system-mt -lftgl -lfreetype -framework Cocoa
INCS = -I/Developer/SDKs/MacOSX10.4u.sdk/usr/X11R6/include/GL/ -I/opt/local/include/GL -I/opt/local/include -L/opt/local/lib -L/Developer/SDKs/MacOSX10.4u.sdk/usr/X11R6/lib/ -I/Users/geoffsmith/usr/local/include/ -L/Users/geoffsmith/usr/local/lib -I/opt/local/include/freetype2/

CC = /usr/bin/g++

PROFILE =

OPTIONS = -O2

default: $(TARGET)

all: default

$(TARGET): src/main.cpp obj.o matrix.o lib.o jpeg.o camera.o car.o wheel.o frame_timer.o
	$(CC) $(PROFILE) -Wall $(INCS) $(LIBS) -o $(TARGET) src/main.cpp obj.o matrix.o lib.o jpeg.o camera.o car.o wheel.o frame_timer.o

obj.o: src/obj.h src/obj.cpp src/matrix.h src/jpeg.h
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o obj.o src/obj.cpp

matrix.o: src/matrix.h src/matrix.cpp src/lib.h
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o matrix.o src/matrix.cpp

lib.o: src/lib.cpp src/lib.h
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o lib.o src/lib.cpp

jpeg.o: src/jpeg.cpp src/jpeg.h
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o jpeg.o src/jpeg.cpp

camera.o: src/camera.h src/camera.cpp
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o camera.o src/camera.cpp

car.o: src/car.h src/car.cpp src/obj.h src/wheel.h src/frame_timer.h
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o car.o src/car.cpp

wheel.o: src/wheel.h src/wheel.cpp src/obj.h
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o wheel.o src/wheel.cpp

frame_timer.o: src/frame_timer.h src/frame_timer.cpp
	$(CC) $(OPTIONS) $(PROFILE) -Wall $(INCS) -c -o frame_timer.o src/frame_timer.cpp

clean:
	-rm -f *.o $(TARGET)
