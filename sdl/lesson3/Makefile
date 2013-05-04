CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = background.o

LIBS = -lSDL

TARGET = sdl_back 

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
