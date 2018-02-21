#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

using namespace std;

int getStatus(char* buffer){
	string data(buffer);
	size_t pos = data.find(" ");
	size_t endpos = data.find(" ", pos+1);
	int status = stoi(data.substr(pos+1, endpos - pos - 1));
	return status;
}

void generateHttpRequest(string file_name, char* buffer, string connection_type){
	if(file_name == "/") file_name = "";
	string tempBuffer = "GET /" + file_name + " HTTP/1.1\r\n";
	tempBuffer+="User-Agent: CustomClient\r\n";
	tempBuffer+="Host: 129.79.247.5\r\n";
	tempBuffer+=connection_type;
	tempBuffer+="\r\n\r\n";
	strcpy(buffer, tempBuffer.c_str());
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

int createSocket(string server_host, int server_port, struct sockaddr_in& address, struct sockaddr_in& server_address){
	int sock_id;
	if((sock_id = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout<<"Error in creating the socket."<<endl;
		exit(EXIT_FAILURE);
	}
	memset(&server_address, '0', sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	
	if(inet_pton(AF_INET, server_host.c_str(), &server_address.sin_addr) <= 0){
		cout<<"Invalid address. This address is not supported."<<endl;
		exit(EXIT_FAILURE);
	}
	return sock_id;
}

void clearBuffer(char* buffer){
	for(int i = 0; i < 1024; i++){
		buffer[i] = 0;
	}
}

void persistentConnection(string server_host, int server_port, string* files, int file_count){
	struct sockaddr_in address, server_address;
	int sock_id = createSocket(server_host, server_port, address, server_address);

	if(connect(sock_id, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
		cout<<"Connection Failed"<<endl;
		exit(EXIT_FAILURE);
	}
	char buffer[1024] = {0};
	string connection_type =  "Connection: keep-alive";
	for(int i = 0; i < file_count; i++){
		if(i == file_count-1) connection_type = "Connection: closed";
		generateHttpRequest(files[i], buffer, connection_type);
		send(sock_id, buffer, strlen(buffer), 0);
		clearBuffer(buffer);
		int valread = read(sock_id, buffer, 1024);
		int status = getStatus(buffer);

		if(status == 200){
			saveFile(buffer, files[i]);
		}
		else if(status == 404){
			cout<<"File Not Found. 404 Error"<<endl;
		}
		else{
			cout<<"Status Not Supported"<<endl;
		}
	}
	close(sock_id);

}

void nonPersistentConnection(string server_host, int server_port, string* files, int file_count){
	string connection_type =  "Connection: Closed";
	for(int i = 0; i < file_count; i++){
		struct sockaddr_in address, server_address;
		int sock_id = createSocket(server_host, server_port, address, server_address);

		if(connect(sock_id, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
			cout<<"Connection Failed"<<endl;
			exit(EXIT_FAILURE);
		}
		char buffer[1024] = {0};
		generateHttpRequest(files[i], buffer, connection_type);
		send(sock_id, buffer, strlen(buffer), 0);
		clearBuffer(buffer);
		int valread = read(sock_id, buffer, 1024);
		int status = getStatus(buffer);

		if(status == 200){
			saveFile(buffer, files[i]);
		}
		else if(status == 404){
			cout<<"File Not Found. 404 Error"<<endl;
		}
		else{
			cout<<"Status Not Supported"<<endl;
		}
		close(sock_id);
	}
}

int main(int argc, char const* argv[]){
	
	if(argc < 5){
		cout<<"Invalid number of parameters."<<endl;
		exit(EXIT_FAILURE);
	}
	
	int file_count = argc - 4;
	string files[file_count];
	
	for(int i = 0; i < file_count; i++){
		files[i] = string(argv[4+i]);
	}

	
	string server_host = string(argv[1]);
	int server_port = atoi(argv[2]);
	int connection_type = atoi(argv[3]);
	if(connection_type == 1) persistentConnection(server_host, server_port, files, file_count);
	else if(connection_type == 0) nonPersistentConnection(server_host, server_port, files, file_count);
	else{
		cout<<"Invalid Connection Type"<<endl;
	}
	return 0;
}
