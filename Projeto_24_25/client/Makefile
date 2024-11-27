CC = gcc
CFLAGS = -Wall -g -Iinclude
BUILD_DIR = build

SRCS = src/client.c ../utils/common.c
OBJS = $(SRCS:.c=.o)

all: client

client: $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/client $(OBJS)

run:
	$(BUILD_DIR)/client $(ARGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BUILD_DIR)/client ../utils/src/*.o
