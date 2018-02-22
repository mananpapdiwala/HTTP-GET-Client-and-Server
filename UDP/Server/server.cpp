#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

#define BUFFSIZE 10000000

using namespace std;

string parseHttp(string httpRequest){
	size_t pathStart = httpRequest.find("/");
	
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
		string tempBuffer = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: closed\r\n\r\nEOF";
		strcpy(buffer, tempBuffer.c_str());	
	}
	else{
		string tempBuffer = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: closed\r\n\r\n";
		string temp = "";
		while(getline(file_id, temp)){
			tempBuffer+=(temp+"\n");
		}
		tempBuffer+="\r\nEOF";
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
	
	char* buffer = (char*)calloc(BUFFSIZE,sizeof(char));
	struct sockaddr_in remaddr;
	socklen_t raddrlen = sizeof(remaddr);
	while(1){
		cout<<"Receiving on port: "<<portNumber<<endl;
		recvlen = recvfrom(server_fd, buffer, BUFFSIZE, 0, (struct sockaddr*)&remaddr, &raddrlen);
		string httpRequest(buffer);
		string tempRequest(buffer);		
		
		while(tempRequest.find("EOF") == string::npos && recvlen > 0){			
			recvlen = recvfrom(server_fd, buffer, BUFFSIZE, 0, (struct sockaddr*)&remaddr, &raddrlen);
			tempRequest = string(buffer);
			httpRequest+=tempRequest;			
		}
		
		if(recvlen > 0){					
			string path = parseHttp(httpRequest);
			bzero(buffer, BUFFSIZE);
			generateHttpResponse(path, buffer);
			
			string dataToSend(buffer);
			int count = 0;
			string currentSend = "";
			while(count < dataToSend.size()){
				currentSend = dataToSend.substr(count, 1500);
				strcpy(buffer, currentSend.c_str());
				if(sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&remaddr, raddrlen) < 0){
					perror("Error: ");
					cout<<"Sending Failed"<<endl;
					close(server_fd);
					free(buffer);
					exit(EXIT_FAILURE);
				}
				count+=currentSend.size();
			}
			

			bzero(buffer, BUFFSIZE);
		}
	}
	close(server_fd);
	free(buffer);
	
	return 0;
}
