/*
Authors: Sanketh Mopuru, Sriharsha Bandaru
Description: Implementation of the Client class
*/

#include "client.hpp"

extern int errno;

void help();


int main(int argc, char **argv){

	string hostname = SRV_IP, port = SRV_PORT;
	if(argc <= 3){
		if(argc >= 2)
			hostname = argv[1];
			if(hostname.compare("-h") == 0){
				help();
				return 1;
			}
		if(argc == 3)
			port = argv[2];
		Client client = Client(hostname, port);
		client.Connect();

	}
	else{
		help();
	}

}

//Print help
void help(){
	cout << "Usage: client [-h] [hostname/IP] [port]" << endl;
	cout << "Default hostname/IP is " << SRV_IP << endl;
	cout << "Default port is " << SRV_PORT << endl;
}

//Get port from socket descriptor
unsigned short getPort(int sockfd){
	struct sockaddr_storage addr;
	socklen_t len = sizeof addr;
	unsigned short port;

	getsockname(sockfd, (struct sockaddr *) &addr, &len );
	if(addr.ss_family = AF_INET){
		struct sockaddr_in *sp = (struct sockaddr_in *) &addr;
		port = ntohs(sp->sin_port);
	}
	else if(addr.ss_family == AF_INET6){
		struct sockaddr_in6 *sp = (struct sockaddr_in6 *) &addr;
		port = ntohs(sp->sin6_port);
	}

	return port;
}

//Constructor
//Initializes the class attributes
Client::Client(string serverip, string serverport){
	serverIp = serverip;
	serverPort = serverport;
	controlSock = 0;
	dataSock = 0;
}

//Connects to the server with ip == serverIp and port == serverPort
//Then shows the promp ftp> and prompts the user for commands
//Allowed commands are 
//get filename
//put filename
//ls, !ls
//cd, !cd
//pwd, !pwd
//quit
void Client::Connect(){

	//For get addrinfo() purposes 
	int rv;
	int dataport;
	int i;

	//buffer
	char temp[128];
	char dataport_string[6];

	//String vector
	vector<string> tokens;

	controlSock = createSocket(serverPort);

	recv(controlSock, (unsigned short *) &dataport, 2, 0);
	sprintf(dataport_string, "%hu", (unsigned short)dataport);
	string dpStr(dataport_string);

	dataSock = createSocket(dpStr);
	cout << "Connected to dataport "<< endl;

	//Now show the prompt and get user input
	while(1){
		cout << "ftp>";
		fgets(temp, 128, stdin);

		string input(temp);
		istringstream iss(input);
		tokens.clear();

		copy( istream_iterator<string>(iss),
		istream_iterator<string>() ,
		back_inserter<vector<string> >(tokens) );

		rv = tokens.size();
		if(rv > 0){
			if(tokens[0].compare("quit") == 0){
				quit();
			}
			else if(tokens[0].compare("get") == 0){
				string temp="";
				for(i=1; i < rv;i++){
					temp = temp + tokens[i] + " ";
				}
				temp = temp.substr(0, temp.length()-1 );
				getFile(temp);
			}
			else if(tokens[0].compare("put") == 0){
				string temp="";
				for(i=1; i < rv;i++){
					temp = temp + tokens[i] + " ";
				}
				temp = temp.substr(0, temp.length()-1 );
				putFile(temp);
			}
			else if(tokens[0].compare("ls") == 0){
				ls_srv(input);
				continue;
			}
			else if(tokens[0].compare("cd") == 0){
				if(rv == 2){
					cd_srv(tokens[1]);
					continue;
				}
			}
			else if(tokens[0].compare("pwd") == 0){
				pwd_srv();
				continue;
			}
			else if(tokens[0].compare("!ls") == 0){
				ls(input.substr(1));
				continue;
			}
			else if(tokens[0].compare("!cd") == 0){
				if(rv == 2){
					cd(tokens[1]);
					continue;
				}
			}
			else if(tokens[0].compare("!pwd") == 0){
				pwd();
				continue;
			}
			else{
				cout << "Invalid FTP Command" << endl;
			}
		}

	}

}


//Quit the client
void Client::quit(){
	cout << "Exiting client ..." << endl;
	send(controlSock, "quit\n", 5, 0);
	close(controlSock);
	exit(1);
}

//List directories client-side
void Client::ls(string command){

	char temp[1024];
	command = command + " 2>&1";
	FILE *fp = popen(command.c_str() , "r");
	if(fp == NULL){
		perror("popen");
		return;
	}

	while(fgets(temp,1024,fp) != NULL){
		printf("%s", temp);
	}

	fclose(fp);
}

//get file from server
void Client::getFile(string filepath){

	FILE *fp;
	int rv;
	int index = 0;
	string filename;

	index = filepath.rfind("/");
	filename = filepath;
	if(index != string::npos){
		filename = filepath.substr(index+1);
	}

	if(dataSock < 0){
		cout << BADF << endl;
		return;
	}

	string command = "get " + filepath + "\n";	
	send(controlSock, command.c_str(), command.length() , 0);	
	rv = receiveMessage(NULL);
	if(rv == 1){
		return;
	}

	fp = fopen(filename.c_str() , "wb");

	if(fp == NULL){
		perror("fopen");
		return;
	}
	
	do{
		rv = receiveMessage(fp);
	}while(rv == 0);

	fclose(fp);
}	

//Change directory of server
void Client::cd_srv (string dirpath){
	int rv;

	if(dataSock < 0){
		cout << BADF <<endl;
		return;
	}

	if(dirpath.compare("") == 0){
		cout << "Invalid directory path." <<endl;
		return;
	}

	string command = "cd " + dirpath + "\n";
	send(controlSock, command.c_str(), command.length(),0 );
	do{
		rv = receiveMessage(NULL);
	}while(rv == 0);
	if(rv == 2)
		cout << endl;
}

//List directories of server
void Client::ls_srv(string command){

	int rv;

	if(dataSock < 0){
		cout << BADF << endl;
		return;
	}

	command += "\n";
	send(controlSock,command.c_str() , command.length() , 0);
	do{
		rv = receiveMessage(NULL);
	}while(rv == 0);
	
}

//PWD of server
void Client::pwd_srv(){
	int rv;
	if(dataSock < 0){
		cout << BADF << endl;
		return;
	}

	send(controlSock, "pwd\n", 4, 0);
	do{
	rv = receiveMessage(NULL);
	}while(rv == 0);
	if(rv == 2)
		cout << endl;
}

//Change the directory of client
void Client::cd(string dirpath){
	if(chdir(dirpath.c_str()) != 0){
		perror("chdir");
		return;
	}
}

//PWD of client
void Client::pwd(){
	char temp[1024];
	if( getcwd(temp, 1024) == NULL ){
		perror("getcwd");
		return;
	}

	string s(temp);
	cout << temp << endl;

}

//Put the file in the server
void Client::putFile(string filename){

	FILE *fp;
	char temp[DATA_SIZE];
	size_t bytesRead;
	string message;

	if(dataSock < 0){
		cout << BADF << endl;
		return;
	}

	fp = fopen(filename.c_str() , "rb");
	if( fp == NULL ){
		perror("fopen");
		return;
	}

	if(validDir(filename.c_str())) {
		cout << ISDIR << endl;
		fclose(fp);
		return;
	}

	string command = "put " + filename + "\n" ;
	send(controlSock, command.c_str(), command.length(), 0);
	do{
		bytesRead = fread(temp, 1, DATA_SIZE, fp);
		if(ferror(fp) != 0){
			command = getError();
			sendMessage(command.c_str(), command.length() , ERROR, NF );
			fclose(fp);
			return;
		}
		if(bytesRead < DATA_SIZE)
			sendMessage(temp, bytesRead, DATA, NF);
		else sendMessage(temp, bytesRead, DATA, MF);
	}while(bytesRead == DATA_SIZE);

	fclose(fp);

}

//Receieve a message from the server
//Either data message or error message
int Client::receiveMessage(FILE *fp){
	unsigned short bytesToRead, bytesRead=0, length;
	char *message, *temp;
	char type, moreFrags;

	recv(dataSock, &type, 1, 0);
	recv(dataSock, &moreFrags, 1, 0);
	recv( dataSock, (unsigned short *) &bytesToRead, 2 , 0 );
	message = (char *)malloc(sizeof(char) * bytesToRead);
	temp = message;
	length = bytesToRead;
	while(bytesToRead > 0){
		bytesRead = recv( dataSock, message, bytesToRead, 0 );
		bytesToRead -= bytesRead;
		message += bytesRead;
	}

	message = temp;
	if(type == DATA && fp != NULL)
		fwrite(message, 1, length, fp);
	else{ 
		fwrite(message, 1, length, stdout);
		if(type == ERROR)
			cout << endl;
	}

	free(message);

	if(type == ERROR){
		return 1;
	}

	if(moreFrags == NF ){
		return 2;
	}

	return 0;

}

//Send message to the server through the datasocket
void Client::sendMessage(const char * message, size_t length, int type , int moreFrags){

	size_t i;

	if(dataSock < 0){
		cout << BADF << endl;
		return;
	}

	char buffer[length + 4];
	buffer[0] = (char)type;
	buffer[1] = (char)moreFrags;
	*((unsigned short *)&buffer[2]) = (unsigned short) length;
	for(i=0;i<length;i++)
		buffer[i+4] = message[i];
	send(dataSock, buffer, length+4, 0);

}

//Find error
string Client::getError(){
	string errMsg = "";
	switch(errno){
		case EBADF:
			errMsg = BADF;
			break;
		case EINTR:
			errMsg = OPINT;
			break;
		case ENOSPC:
			errMsg = NOSPC;	
			break;
		case EISDIR:
			errMsg = ISDIR;
			break;
		case ENOENT:
			errMsg = NOTFOUND;
			break;
		default:
			errMsg = DEFERR;
	}

	return errMsg;
}

 
//Create socket for communication with the server
//Args : Port
//Return: Socket descriptor
int Client::createSocket(string port){
	addrinfo hints, *servinfo, *p;
	int rv, sockfd;

	//Initialize hints
	memset(&hints, 0, sizeof hints);
	//Fill up hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//Call getaddrinfo()

	if( (rv = getaddrinfo(serverIp.c_str(), port.c_str(), &hints, &servinfo)) != 0 ){
		//Some error occured
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv) );
		exit(1);
	}

	for( p = servinfo; p != NULL; p = p->ai_next ){

		if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
			perror("socket");
			continue;
		}

		if( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1 ){
			close(sockfd);
			perror("connect");
			continue;
		}

		break;
	}

	//Check if the socket created successfully
	if( p == NULL ){
		fprintf(stderr, "Failed to create socket\n");
		exit(1);
	}

	//Free the memory
	freeaddrinfo(servinfo);
	return sockfd;

}

//Checks if the directory pointed to by dirpath is valid or not
//returns true if valid otherwise returns false
bool Client::validDir(const char * dirpath){
	struct stat dir;
	if ( stat( dirpath, &dir ) != 0 ){
		return false;
	}

	return ( (dir.st_mode & S_IFMT)  == S_IFDIR );
}