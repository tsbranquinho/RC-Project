CC = gcc
CFLAGS = -Wall -g -Iinclude
BUILD_DIR = build
SRC_DIR = src
UTILS_DIR = ../utils
TCP_DIR = $(SRC_DIR)/tcp
UDP_DIR = $(SRC_DIR)/udp

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/player.c \
       $(wildcard $(TCP_DIR)/*.c) \
       $(wildcard $(UDP_DIR)/*.c) \
       $(UTILS_DIR)/common.c
OBJS = $(SRCS:.c=.o)

# Final executable
EXEC = $(BUILD_DIR)/server

all: $(EXEC) 
	@echo "Server build complete!"

$(EXEC): $(OBJS)
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/server $(OBJS)

run: $(EXEC)
	@$(EXEC) $(ARGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(EXEC) $(UTILS_DIR)/common.o *.o
	@echo "Server clean complete!"
