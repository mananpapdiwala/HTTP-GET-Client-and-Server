all:
	@echo "Compiling. Please wait."
	@g++ server.cpp -o server
	@echo "Compilation finished. Please use make run to execute."
run:
	./server ${PORT}
clean:
	@rm -rf ./server
	@echo "Server file deleted."
