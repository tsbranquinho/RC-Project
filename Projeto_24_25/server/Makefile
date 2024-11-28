CC = gcc
CFLAGS = -Wall -g -Iinclude
BUILD_DIR = build
SRC_DIR = src
COMMON_DIR = ../common
TCP_DIR = $(SRC_DIR)/tcp
UDP_DIR = $(SRC_DIR)/udp
UTILS_DIR = $(SRC_DIR)/utils

SRCS = $(SRC_DIR)/main.c \
       $(wildcard $(TCP_DIR)/*.c) \
       $(wildcard $(UDP_DIR)/*.c) \
	   $(wildcard $(UTILS_DIR)/*.c) \
       $(COMMON_DIR)/common.c
OBJS = $(SRCS:.c=.o)

# Final executable
EXEC = $(BUILD_DIR)/GS

all: $(EXEC) 
	@echo "Server build complete!"

$(EXEC): $(OBJS)
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/GS $(OBJS)

run: $(EXEC)
	@$(EXEC) $(ARGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(EXEC) $(COMMON_DIR)/common.o *.o
	@echo "Server clean complete!"
