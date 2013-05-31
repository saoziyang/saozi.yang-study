CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 

OBJS = main.o camera.o sdl_opt.o timer.o network.o msx264.o

LIBS = -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer -lx264 \
	    
TARGET = main

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	#$(CXX) $(CXXFLAGS)-o $(TARGET)  $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
