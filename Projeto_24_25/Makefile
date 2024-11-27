.PHONY: all server client run-server run-client clean

all: server client
	@echo "Build complete!"

server: 
	@$(MAKE) -C server --no-print-directory

client:
	@$(MAKE) -C client --no-print-directory

run-server:
	@$(MAKE) -C server run --no-print-directory

run-client:
	@$(MAKE) -C client run --no-print-directory

clean:
	@$(MAKE) -C server clean --no-print-directory
	@$(MAKE) -C client clean --no-print-directory
	@echo "Clean complete!"


