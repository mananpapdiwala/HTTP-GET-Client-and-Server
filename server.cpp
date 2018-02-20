#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fstream>

using namespace std;

string parseHttp(string httpRequest){
	size_t pathStart = httpRequest.find_first_of("/");
	if(pathStart == string::npos){
		cout<<"Error parsing HttpRequest. InValid Path."<<endl;
		return NULL;		
	}
	size_t pathEnd = httpRequest.find_first_of(" ", pathStart);

	if(pathEnd == pathStart + 1){
		cout<<"Path is empty"<<endl;
		return "index.html";
	}
	return httpRequest.substr(pathStart+1, pathEnd - pathStart-1);
	
}

void generateHttpResponse(string filePath, char* buffer){
	ifstream file_id(filePath.c_str());
	if(!file_id){
		cout<<"Could not find "<<filePath<<"."<<endl;
		string tempBuffer = "HTTP/1.1 404 Not Found\r\nContent-Type: txt\r\n\r\n";
		strcpy(buffer, tempBuffer.c_str());	
	}
	else{
		string tempBuffer = "HTTP/1.1 200 OK\r\nContent-Type: txt\r\n\r\n";
		string temp = "";
		while(getline(file_id, temp)){
			tempBuffer+=(temp+"\n");
		}
		tempBuffer+="\r\n";
		file_id.close();
		strcpy(buffer, tempBuffer.c_str());	
	}
}

int main(int argc, char const* argv[]){
	if(argc != 2) cout<<"Please enter a port number"<<endl;
	int portNumber = atoi(argv[1]);
	int opt = 1;
	int server_fd, new_socket, valread;	
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		cout<<"Socket Creating Failed"<<endl;
		exit(EXIT_FAILURE);
	}
	
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
		cout<<"Error Assigning port"<<endl;
		exit(EXIT_FAILURE);
	}
	
	struct sockaddr_in address;
	int addlen = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portNumber);
	
	if(bind(server_fd, (struct sockaddr* )&address, sizeof(address)) < 0){
		cout<<"Binding Failed"<<endl;
		exit(EXIT_FAILURE);
	}
	
	if(listen(server_fd, 3) < 0){
		cout<<"Error in listening"<<endl;
		exit(EXIT_FAILURE);
	}
	
	while(1){
		cout<<"Receiving on port: "<<portNumber<<endl;
		if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addlen)) < 0){
			cout<<"Accept Failed"<<endl;
			exit(EXIT_FAILURE);
		}
		char buffer[1024] = {0};
		valread = read(new_socket, buffer, 1024);
		string httpRequest(buffer);
		string path = parseHttp(httpRequest);
		for(int i = 0; i < 1024; i++){
			buffer[i] = 0;
		}
		generateHttpResponse(path, buffer);
		
		send(new_socket, buffer, strlen(buffer), 0);
		cout<<"Message Sent"<<endl;
		close(new_socket);
	}
	close(server_fd);
	return 0;
}
