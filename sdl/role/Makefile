CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = main.o timer.o role.o background.o sdl_opt.o

LIBS = -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

TARGET = main

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
