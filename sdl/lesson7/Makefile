CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = font.o

LIBS = -lSDL -lSDL_image -lSDL_ttf

TARGET = sdl_font

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
