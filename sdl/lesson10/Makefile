CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = lesson10.o

LIBS = -lSDL -lSDL_image -lSDL_ttf

TARGET = lesson10 

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
