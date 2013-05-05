CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = mouse_event.o

LIBS = -lSDL -lSDL_image -lSDL_ttf

TARGET = sdl_mouse_event

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
