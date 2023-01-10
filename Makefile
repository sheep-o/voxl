CC := g++
CFLAGS := -Wall -g
TARGET := voxl 

LIBPATHS := -L/usr/lib
LIBS := -lGL -lglfw -lGLEW

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, %.o, $(SRCS))

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(LIBPATHS) $(LIBS) -o $@ $^
%.o: %.cpp
	$(CC) $(CFLAGS) -c $<
clean:
	rm -rf $(TARGET) *.o
all: run
run: $(TARGET)
	./$(TARGET)
