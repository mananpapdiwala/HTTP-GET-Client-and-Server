all:
	@echo "Compiling. Please wait."
	@g++ client.cpp -o client -std=c++11
	@echo "Compilation finished. Please use make run to execute."
run:
	./client ${HOST} ${PORT} ${CONNECTION_TYPE} ${FILE_NAME}
clean:
	@rm -rf ./client
	@echo "Server file deleted."
