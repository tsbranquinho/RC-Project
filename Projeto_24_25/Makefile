.PHONY: all ser cli run-server run-client clean

all: ser cli

ser: 
	$(MAKE) -C server

cli:
	$(MAKE) -C client

run-server:
	@./server/build/server

run-client:
	@./client/build/client

clean:
	$(MAKE) -C server clean
	$(MAKE) -C client clean


