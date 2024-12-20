CC = gcc
CFLAGS = -Wall -g -Iinclude -pthread
BUILD_DIR = build
SRC_DIR = src
COMMON_DIR = ../common
TCP_DIR = $(SRC_DIR)/tcp
UDP_DIR = $(SRC_DIR)/udp
UTILS_DIR = $(SRC_DIR)/utils
EXTRAS_DIR = $(SRC_DIR)/extras

SRCS = $(SRC_DIR)/main.c \
    $(wildcard $(TCP_DIR)/*.c) \
    $(wildcard $(UDP_DIR)/*.c) \
	$(wildcard $(UTILS_DIR)/*.c) \
	$(wildcard $(EXTRAS_DIR)/*.c) \
    $(COMMON_DIR)/common.c
OBJS = $(SRCS:.c=.o)

LOCAL_EXEC = ./GS
PARENT_EXEC = ./../GS

all: $(LOCAL_EXEC) $(PARENT_EXEC)
	@echo "Server build complete!"

$(LOCAL_EXEC): $(OBJS)
	@$(CC) $(CFLAGS) -o $(LOCAL_EXEC) $(OBJS)

$(PARENT_EXEC): $(OBJS)
	@$(CC) $(CFLAGS) -o $(PARENT_EXEC) $(OBJS)

run: $(LOCAL_EXEC)
	@$(LOCAL_EXEC) $(ARGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(EXEC) $(COMMON_DIR)/common.o *.o
	@rm -f GS
	@rm -f $(PARENT_EXEC)
	@echo "Server clean complete!"