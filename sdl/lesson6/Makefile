CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = clip_dots.o

LIBS = -lSDL -lSDL_image

TARGET = sdl_dots

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
