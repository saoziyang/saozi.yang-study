CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = map_color_key.o

LIBS = -lSDL -lSDL_image

TARGET = sdl_map_color

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
