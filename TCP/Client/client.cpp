#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

#define BUFFSIZE 100000

using namespace std;

int getStatus(string data){	
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

void saveFile(string data, string file_name){	
	size_t position = data.find("\r\n\r\n");
	if(position == string::npos){
		cout<<"Data not available."<<endl;
		return;
	}	
	size_t end_position = data.find_last_of("\r\n");
	
	if(end_position == string::npos){
		data = data.substr(position+4);
	}
	else{
		data = data.substr(position+4, (end_position-1 - position - 4));
	}
	
	if(file_name == "/") file_name = "index.html";
	cout<<"File Name: "<<file_name<<endl;
	cout<<"File Content:"<<endl;
	cout<<data<<endl;	
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

void persistentConnection(string server_host, int server_port, string* files, int file_count){
	struct sockaddr_in address, server_address;
	int sock_id = createSocket(server_host, server_port, address, server_address);

	if(connect(sock_id, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
		cout<<"Connection Failed"<<endl;
		exit(EXIT_FAILURE);
	}
	char* buffer = (char*)calloc(BUFFSIZE,sizeof(char));
	string connection_type =  "Connection: keep-alive";
	string httpData = "";
	string tempData = "";
	for(int i = 0; i < file_count; i++){
		if(i == file_count-1) connection_type = "Connection: closed";		
		generateHttpRequest(files[i], buffer, connection_type);		
		send(sock_id, buffer, strlen(buffer), 0);
		bzero(buffer, BUFFSIZE);
		bool flag = false;
		httpData = tempData;
		while(!flag){
			read(sock_id, buffer, BUFFSIZE);					
			tempData = string(buffer);
			httpData+=tempData;
			if(tempData.find("$EOF$") != string::npos){
				flag = true;
				char* tempP = buffer;
				tempP+=tempData.size();
				tempData = string(tempP);
				cout<<httpData.size()<<endl;
			}			
		}
		
		int status = getStatus(httpData);			
		
		if(status == 200){
			saveFile(httpData, files[i]);			
		}
		else if(status == 404){
			cout<<"File "<<files[i]<<" not Found. 404 Error"<<endl;
		}
		else{
			cout<<"Status Not Supported"<<endl;
		}
		
		bzero(buffer, BUFFSIZE);
	}	
	close(sock_id);
	free(buffer);

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
		char* buffer = (char*)calloc(BUFFSIZE,sizeof(char));
		
		generateHttpRequest(files[i], buffer, connection_type);		
		send(sock_id, buffer, strlen(buffer), 0);		
		bzero(buffer, BUFFSIZE);
		string httpData = "";
		while(read(sock_id, buffer, BUFFSIZE) > 0){
			httpData+=string(buffer);
			bzero(buffer, BUFFSIZE);
		}		
		int status = getStatus(httpData);

		if(status == 200){
			saveFile(httpData, files[i]);			
		}
		else if(status == 404){
			cout<<"File "<<files[i]<<" not Found. 404 Error"<<endl;
		}
		else{
			cout<<"Status Not Supported"<<endl;
		}
		close(sock_id);
		free(buffer);
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
