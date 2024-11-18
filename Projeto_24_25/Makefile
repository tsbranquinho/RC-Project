CC = gcc
CFLAGS = -pthread -Wall
TARGETS = player GS
COMMON_SRC = common.c

all: $(TARGETS)

$(TARGETS): %: $(COMMON_SRC) %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS)
