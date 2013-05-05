CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = lesson11.o

LIBS = -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

TARGET = lesson11

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
