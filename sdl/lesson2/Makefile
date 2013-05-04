CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = showbmp.o

LIBS = -lSDL

TARGET = sdl_load_bmp 

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
