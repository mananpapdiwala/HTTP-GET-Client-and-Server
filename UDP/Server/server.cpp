#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

void clearBuffer(char* buffer){
	for(int i = 0; i < 1024; i++){
		buffer[i] = 0;
	}
}

string parseHttp(string httpRequest){
	size_t pathStart = httpRequest.find("/");
	if(pathStart == string::npos){
		cout<<"Error parsing HttpRequest. InValid Path."<<endl;
		return NULL;		
	}
	size_t pathEnd = httpRequest.find(" ", pathStart);

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
		string tempBuffer = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: closed\r\n\r\n";
		strcpy(buffer, tempBuffer.c_str());	
	}
	else{
		string tempBuffer = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: closed\r\n\r\n";
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
	if(argc != 2){
		cout<<"The program takes only parameter which is the port number."<<endl;
		exit(EXIT_FAILURE);
	}

	int portNumber = atoi(argv[1]);
	
	int server_fd;
	int opt = 1;
	if((server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		cout<<"Socket creating failed."<<endl;
		exit(EXIT_FAILURE);
	}

	
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
		cout<<"Error Assigning port"<<endl;
		exit(EXIT_FAILURE);
	}
	
	struct sockaddr_in address;
	memset((char*)&address, 0, sizeof(address));

	int addlen = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(portNumber);

	if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
		cout<<"Binding Failed"<<endl;
		exit(EXIT_FAILURE);	
	}
	
	int recvlen;
	char buffer[1024];
	struct sockaddr_in remaddr;
	socklen_t raddrlen = sizeof(remaddr);
	while(1){
		cout<<"Receiving on port: "<<portNumber<<endl;
		recvlen = recvfrom(server_fd, buffer, 1024, 0, (struct sockaddr*)&remaddr, &raddrlen);
		if(recvlen > 0){
			string httpRequest(buffer);
			string path = parseHttp(httpRequest);
			clearBuffer(buffer);
			generateHttpResponse(path, buffer);
			sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&remaddr, raddrlen);
		}
	}
	close(server_fd);
	
	return 0;
}