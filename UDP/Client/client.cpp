#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>
#include <string>

using namespace std;

void clearBuffer(char* buffer){
	for(int i = 0; i < 1024; i++){
		buffer[i] = 0;
	}
}

int getStatus(char* buffer){
	string data(buffer);
	size_t pos = data.find(" ");
	size_t endpos = data.find(" ", pos+1);
	int status = stoi(data.substr(pos+1, endpos - pos - 1));
	return status;
}

void saveFile(char* buffer, string file_name){
	string data(buffer);
	size_t position = data.find("\r\n\r\n");
	if(position == string::npos){
		cout<<"Data not available."<<endl;
		return;
	}
	data = data.substr(position+4);
	ofstream outputFile;
	if(file_name == "/") file_name = "index.html";
	outputFile.open(file_name.c_str());
	outputFile << data;
	outputFile.close();
}

void generateHttpRequest(string file_name, char* buffer){
	if(file_name == "/") file_name = "";
	string tempBuffer = "GET /" + file_name + " HTTP/1.1\r\n";
	tempBuffer+="User-Agent: CustomClient\r\n";
	tempBuffer+="Host: 129.79.247.5\r\n";
	tempBuffer+="Connection: closed";
	tempBuffer+="\r\n\r\n";
	strcpy(buffer, tempBuffer.c_str());
}

int main(int argc, char const* argv[]){
	if(argc != 4){
		cout<<"Invalid number of parameters."<<endl;
		exit(EXIT_FAILURE);
	}

	string server_host = string(argv[1]);
	int server_port = atoi(argv[2]);
	string file_name = string(argv[3]);

	int sock_id;

	struct sockaddr_in server_address;
	socklen_t addrlen = sizeof(server_address);
	if((sock_id = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		cout<<"Error in creating the socket."<<endl;
		exit(EXIT_FAILURE);
	}
	memset((char*)&server_address, '0', sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	
	if(inet_pton(AF_INET, server_host.c_str(), &server_address.sin_addr) <= 0){
		cout<<"Invalid address. This address is not supported."<<endl;
		exit(EXIT_FAILURE);
	}

	char buffer[1024];
	generateHttpRequest(file_name, buffer);
	if(sendto(sock_id, buffer, sizeof(buffer), 0, (struct sockaddr*) &server_address, addrlen) < 0){
		cout<<"Sending Failed"<<endl;
		close(sock_id);
		exit(EXIT_FAILURE);
	}
	clearBuffer(buffer);
	if(recvfrom(sock_id, buffer, 1024, 0, (struct sockaddr*)&server_address, &addrlen) < 0){
		cout<<"Receiving Failed"<<endl;
		close(sock_id);
		exit(EXIT_FAILURE);	
	}
	
	int status = getStatus(buffer);

	if(status == 200){
		saveFile(buffer, file_name);
	}
	else if(status == 404){
		cout<<"File Not Found. 404 Error"<<endl;
	}
	else{
		cout<<"Status Not Supported"<<endl;
	}

	close(sock_id);
}