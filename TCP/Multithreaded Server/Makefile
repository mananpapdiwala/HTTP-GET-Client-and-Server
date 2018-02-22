all:
	@echo "Compiling. Please wait."
	@g++ multithreaded_server.cpp -o multithreaded_server -lpthread
	@echo "Compilation finished. Please use make run to execute."
run:
	./multithreaded_server ${PORT}
clean:
	@rm -rf ./multithreaded_server
	@echo "Server file deleted."
