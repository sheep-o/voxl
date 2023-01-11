CC := g++
CFLAGS := -Wall -g -I/opt/homebrew/include -std=c++20
TARGET := voxl 

LIBPATHS := -L/usr/lib -L/opt/homebrew/lib
LIBS := -framework OpenGL -lglfw -lGLEW

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
