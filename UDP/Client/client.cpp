#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>
#include <string>

#define BUFFSIZE 10000
#define TIMEOUT_MS 100000

using namespace std;

int getStatus(string httpResponse){
	size_t pos = httpResponse.find(" ");
	size_t endpos = httpResponse.find(" ", pos+1);
	int status = stoi(httpResponse.substr(pos+1, endpos - pos - 1));
	return status;
}

void saveFile(string httpResponse, string file_name){	
	size_t position = httpResponse.find("\r\n\r\n");
	if(position == string::npos){
		cout<<"Data not available."<<endl;
		return;
	}
	size_t end_position = httpResponse.find_last_of("\r\n");
	string data;
	if(end_position == string::npos){
		data = httpResponse.substr(position+4);
		cout<<"Here"<<endl;
	}
	else{
		data = httpResponse.substr(position+4, end_position-1-position-4);
		cout<<"Not here"<<endl;
	}	
	if(file_name == "/") file_name = "index.html";
	cout<<"File Name: "<<file_name<<endl;
	cout<<"File Content:"<<endl;
	cout<<data<<endl;
}

void generateHttpRequest(string file_name, char* buffer){
	if(file_name == "/") file_name = "";
	string tempBuffer = "GET /" + file_name + " HTTP/1.1\r\n";
	tempBuffer+="User-Agent: CustomClient\r\n";
	tempBuffer+="Host: 129.79.247.5\r\n";
	tempBuffer+="Connection: closed";
	tempBuffer+="\r\n\r\n";
	tempBuffer+="EOF";
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
	if((sock_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		cout<<"Error in creating the socket."<<endl;
		exit(EXIT_FAILURE);
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT_MS;
	if (setsockopt(sock_id, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    perror("Error");
	}

	memset((char*)&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	
	if(inet_pton(AF_INET, server_host.c_str(), &server_address.sin_addr) <= 0){
		cout<<"Invalid address. This address is not supported."<<endl;
		exit(EXIT_FAILURE);
	}
	
	char* buffer = (char*)calloc(BUFFSIZE,sizeof(char));
	generateHttpRequest(file_name, buffer);	
	
	if(sendto(sock_id, buffer, strlen(buffer), 0, (struct sockaddr*) &server_address, addrlen) < 0){
		cout<<"Sending Failed"<<endl;
		close(sock_id);
		exit(EXIT_FAILURE);
	}
		
	
	bzero(buffer, BUFFSIZE);
	bool flag = false;
	int count_bytes = 0;
	string httpResponse = "";
	string tempResponse = "";
	while((!flag) && (recvfrom(sock_id, buffer, BUFFSIZE, 0, (struct sockaddr*)&server_address, &addrlen) > 0)){
		tempResponse = string(buffer);
		httpResponse+=tempResponse;
		count_bytes+=tempResponse.size();
		if(tempResponse.find("EOF") != string::npos){
			flag = true;
		}
	}
	
	
	int status = getStatus(httpResponse);	
	if(status == 200){
		saveFile(httpResponse, file_name);
		cout<<"File "<<file_name<<" successfully saved."<<endl;
		cout<<"Total Bytes received including the header and last special character: "<<count_bytes<<endl;
	}
	else if(status == 404){
		cout<<"File "<<file_name<<" not Found. 404 Error"<<endl;
	}
	else{
		cout<<"Status Not Supported"<<endl;
	}

	close(sock_id);
	free(buffer);
}
