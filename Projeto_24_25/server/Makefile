CC = gcc
CFLAGS = -Wall -g -Iinclude
BUILD_DIR = build

SRCS = src/*.c ../utils/common.c
OBJS = $(SRCS:.c=.o)

all: server 

server: $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/server $(OBJS)

run:
	$(BUILD_DIR)/server $(ARGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BUILD_DIR)/server ../utils/src/*.o
