CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = event.o

LIBS = -lSDL -lSDL_image

TARGET = sdl_event

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
