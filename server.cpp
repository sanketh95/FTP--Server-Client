/*
Authors: Sanketh Mopuru, Sriharsha Bandaru
Description: Implementation of the Server class
*/

#include "server.hpp"

extern int errno;

int cs = 0, commsock = 0, ds = 0, datacommsock = 0 ;

extern int optind;
extern char *optarg;


void help();

int main(int argc, char **argv){

	string rootdir = ROOT_DIR, port = CTRL_PORT;
	bool vflag = false, hflag = false, rflag = false, pflag = false, errflag = false; 
	int hpos, option;

	while( (option = getopt(argc, argv, "hvr:p:")) != -1 ){
		switch(option){
			case 'v':
				vflag = true;
				break;
			case 'h':
				hflag = true;
				hpos = optind;
				break;
			case 'r':
				rflag = true;
				rootdir = optarg;
				break;
			case 'p':
				pflag = true;
				port = optarg;
				break;
			case '?':
				errflag = true;
		}
	}

	if(errflag){
		help();
		return 1;
	}

	if(hflag && hpos == 2){
		help();
		return 0;	
	}


	Server s = Server(rootdir, port, vflag);
	s.start();
	return 0;
}


//Print help
void help(){
	cout << "Usage: server [-h] [-v] [-p <port>] [-r <root directory>]" << endl;
	cout << "Options:" << endl;
	cout << "-h\tThis information" << endl;
	cout << "-v\tSet this option for more verbosity" << endl;
	cout << "-p\tPort to bind the server to" << endl;
	cout << "-r\tRoot directory of the server" << endl;
	cout << "\nDefault root directory:\t" << ROOT_DIR << endl;
	cout <<  "Default port:\t" << CTRL_PORT << endl;
}

void sigchldHandler( int s ){
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void userIntHandler(int s){
	close(cs);
	close(commsock);
	close(ds);
	close(datacommsock);
	exit(200);
}

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

//Get IP Address from sockaddr_storage addr
string getIPFromAddr(struct sockaddr_storage *addr){
	char ip[INET6_ADDRSTRLEN];
	string ipstr;
	if(addr->ss_family == AF_INET){
		struct sockaddr_in * sp = (struct sockaddr_in *)addr;
		inet_ntop(AF_INET, &(sp->sin_addr), ip, sizeof ip );
		ipstr = ip;
	}
	else if(addr->ss_family == AF_INET6){
		struct sockaddr_in6 * sp = (struct sockaddr_in6 *)addr;
		inet_ntop(AF_INET, &(sp->sin6_addr), ip, sizeof ip );
		ipstr = ip;
	}

	return ipstr;
}


//Get port number from the sockaddr_storage structure
unsigned short getPortFromAddr(struct sockaddr_storage *addr){

	unsigned short port;

	if(addr->ss_family == AF_INET){
		struct sockaddr_in * sp = (struct sockaddr_in *) addr;
		port = ntohs(sp->sin_port);
	}
	else if(addr->ss_family == AF_INET){
		struct sockaddr_in6 * sp = (struct sockaddr_in6 *) addr;
		port = ntohs(sp->sin6_port);
	}

	return port;
}

//Overloaded constructor, 
//Takes rootdir and control port number as arguments
Server::Server(string rootdir, string port, bool verb){
	rootDir = rootdir;
	if(rootDir[ rootDir.size() - 1 ] != '/'){
		rootDir = rootDir + "/";
	}
	controlPort = port;
	pwd = "";
	controlSock = dataSock = 0;
	verbose = verb;

}


//Starts the server and listens for connections
void Server::start(){

	//check if directory exists
	//If it doesn't , try to create the directory

	char temp[1024];

	//Get address info purposes
	struct sockaddr_storage otherAddr, otherAddr2;		//Connectors address information
	socklen_t otherAddrLen, otherAddrLen2;
	int rv;
	int i;

	//Socket purposes
	int newfd;
	int newfd2;

	//port numbers
	unsigned short dataPort;

	//Handling child processes
	struct sigaction sa;

	//Char buffers
	char buffer[DATA_SIZE];

	//String vector
	vector<string> tokens;

	//extra string(s)
	string errMsg = INVCOM;


	//Copy the string into character array
	//since chdir() takes char * as argument
	strncpy( temp, rootDir.c_str() , sizeof(temp) );
	temp[ sizeof(temp) - 1 ] = 0;

	//Check if directory is valid
	//If no directory create one
	if( !validDir(temp) ){

		if( mkdir(temp, 0777) != 0 ){
			perror("mkdir");
			exit(1);
		}

	}

	//change directory
	//call chdir()
	if(chdir(temp) != 0){
		//If chdir fails
		perror("chdir");
		exit(1);
	}

	if ( !getPWD() ){
		cout << "Unable to get cwd" << endl;
		exit(1);
	}

	//We have now changed the program's pwd to root directory 
	//Now create a socket and bind to the control port and start listening
	//for connections

	cs = controlSock = createSocket( controlPort);
	if(verbose)
		cout << "Control socket created" << endl;

	ds = dataSock = createSocket("0");
	if(verbose)
		cout << "Data socket created" << endl;

	dataPort = getPort(dataSock);

	//Now we have successfully created the socket 
	//and bound it to the control port
	//Start listening for connections
	//Entering the listening loop

	if ( listen (controlSock , BACKLOG) != 0 ) {
		perror("listen");
		close(controlSock);
		exit(1);
	}

	if( listen(dataSock , BACKLOG) != 0 ){
		perror("listen");
		close(dataSock);
		exit(1);
	}

	//handle dead processes
	sa.sa_handler = sigchldHandler;		//read all dead processes
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = SA_RESTART;
	if( sigaction( SIGCHLD, &sa , NULL ) == -1 ){
		perror("sigaction");
		exit(1);
	}

	//Handle user interrupts
	signal(SIGINT, userIntHandler);

	cout << "Waiting for connections ... " << endl;

	while(1){

		otherAddrLen = sizeof ( otherAddr) ;
		newfd = accept ( controlSock , (struct sockaddr *) &otherAddr , &otherAddrLen );
		cout << "Connection from " << getIPFromAddr(&otherAddr) << ":" << getPortFromAddr(&otherAddr) << endl;
		commsock = newfd;

		send(newfd, (unsigned short *)&dataPort, 2, 0);

		otherAddrLen2 = sizeof otherAddr2;
		newfd2 = accept(dataSock, (struct sockaddr *) &otherAddr2, &otherAddrLen2 );
		datacommsock = newfd2;
		cout << "Connection to data socket from client port " << getPortFromAddr(&otherAddr2) << endl;

		if( fork() == 0 ){
			//The code in this block is executed by the child process
			close(controlSock);  //Child doesn't require the control socket
			close(dataSock);

			if(verbose){
				cout << "Closing listener sockets" << endl;
				cout << "Child process created for communication with client" << endl;
			}

			if (dup2( newfd , 0 ) != 0 ){
				perror("dup2");
				exit(1);
			}

			do{
				
				if(fgets( buffer, DATA_SIZE, stdin ) == NULL){
					close(newfd);
					exit(1);
				}

				//Remove carriage return and new line characters
				for(rv = 0; rv < DATA_SIZE ; rv++){

					if(buffer[rv] == '\r'){
						buffer[rv] = 0;
						break;
					}

					if(buffer[rv] == '\n'){
						buffer[rv] = 0;
						break;
					}
				}
				
				//Read buffer and find what command it is
				//Call the corresponding method

				string s(buffer);		//convert the character array into string
				if(verbose){
					cout << "Received command " + s << endl;
				}

				istringstream iss(s);

				//copy the tokens into the string vector
				copy( istream_iterator<string>(iss),
				 istream_iterator<string>() ,
				 back_inserter<vector<string> >(tokens) );


				//Copied the tokens into tokens
				rv = tokens.size();
				if(rv > 0){
					//If there is atleast one word
					if(tokens[0].compare("ls") == 0){
						//The client requested a file listing
						ls(s, newfd2);
					}
					else if(tokens[0].compare("quit") == 0){
						//The client closed the connection
						break;
					}
					else if(tokens[0].compare("cd") == 0){
						//The client requested a directory change
						if(rv == 2){
							cd(tokens[1], newfd2);
						}
						else{
							sendMessage( errMsg.c_str() , errMsg.length() , ERROR, NF, dataSock );
						}
					}
					else if( tokens[0].compare("pwd") == 0 ){
						//Client requested a pwd
						PWD(newfd2);
					}
					else if(tokens[0].compare("get") == 0){
						string temp="";
						for(i=1; i < rv;i++){
							temp = temp + tokens[i] + " ";
						}
						temp = temp.substr(0, temp.length()-1 );
						getFile(temp, newfd2);
					}
					else if(tokens[0].compare("put") == 0){
						string temp="";
						for(i=1; i < rv;i++){
							temp = temp + tokens[i] + " ";
						}
						temp = temp.substr(0, temp.length()-1 );
						putFile(temp, newfd2);
					}
					else{
						sendMessage( errMsg.c_str() , errMsg.length() , ERROR, NF, newfd2 );
					}
				}

				tokens.clear();		//clear the tokens

			}while (1);

			//The client closed the connection
			//So, close the socket and quit the child process
			close(newfd);		
			close(newfd2);	
			cout << "Connection closed" << endl;
			exit(1);
		}
		close(newfd);

	}

	close(controlSock);
	

}

//Checks if the directory pointed to by dirpath is valid or not
//returns true if valid otherwise returns false
bool Server::validDir(const char * dirpath){
	struct stat dir;
	if ( stat( dirpath, &dir ) != 0 ){
		return false;
	}

	return ( (dir.st_mode & S_IFMT)  == S_IFDIR );
}

//List out the files and folders in the pwd
void Server::ls(string command,int fd) {

	int len;
	string errorMsg;
	FILE *fp;
	char temp[DATA_SIZE];

	if(fd < 0){
		cout << BADF << endl;
		return;
	}

	cout << "Listing all files and folders" << endl;

	command += " 2>&1";

	fp = popen(command.c_str() , "r");
	if( fp == NULL ){
		errorMsg = getError();
		cout << errorMsg << endl;
		sendMessage(errorMsg.c_str() , errorMsg.length(), ERROR, NF, fd );
		return;
	}

	do{
		len = fread(temp, 1, DATA_SIZE, fp );
		if(ferror(fp) != 0){
			errorMsg = getError();
			cout << errorMsg << endl;
			sendMessage(errorMsg.c_str() , errorMsg.length(), ERROR, NF, fd );
			pclose(fp);
			return;			
		}
		if(len < DATA_SIZE){
			sendMessage(temp, len, DATA, NF, fd);
		}
		else sendMessage(temp, len, DATA, MF, fd);
	}while(len == DATA_SIZE );

	pclose(fp);
}


//change directory to dirpath
void Server::cd(string dirpath, int fd){

	string temp = dirpath;
	char t[1024];
	size_t index = 0;
	string errorMsg;

	if(fd < 0){
		cout << BADF << endl;
		return;
	}

	if(dirpath[0] == '/'){
		dirpath.replace(0, 1, rootDir);
	}

	if(dirpath[dirpath.size() - 1] != '/'){
		dirpath = dirpath + "/";
	}

	//If failed to change directory
	if(chdir( dirpath.c_str() ) != 0){
		errorMsg = getError();
		cout << errorMsg << endl;
		sendMessage(errorMsg.c_str(), errorMsg.length() , ERROR, NF, fd );
		return;
	}

	//Get the cwd
	if( getcwd(t, 1024) == NULL ){
		exit(1);
	}

	string t1(t);
	t1 = t1 + "/";

	//Get the index of substring
	index = t1.find(rootDir, index);
	if(index == string::npos){
		//We have gone to a parent of root directory
		//So, change back to root
		if( chdir(rootDir.c_str()) != 0 ){
			perror("chdir");
			exit(1);
		}
	}

	if(!getPWD () ){
		exit(1);
	}

	errorMsg = DIRCOM;
	sendMessage(errorMsg.c_str() , errorMsg.length() , DATA, NF, fd );

	cout << DIRCOM << endl;

}

//Send the pwd to the client
void Server::PWD(int fd){
	if( fd < 0 ){
		cout << BADF << endl;
		return;
	}

	string temp = pwd;

	sendMessage(temp.c_str() , temp.length(), DATA, NF,  fd);
	cout << PWDCOM << endl;
}

//Gets the current working directory by calling getcwd()
//and replaces the substring corresponding to the root directory
//with '/' and sets it to Server::pwd
bool Server::getPWD(){
	char temp[1024];
	int index;

	if( getcwd(temp, 1024) == NULL ){
		return false;
	}

	string t(temp);
	t = t+"/";
	t.replace(0, rootDir.size() , "/");
	pwd = t;
	return true;

}

//Send file to the client

void Server::getFile(string filename, int fd){

	char buffer[DATA_SIZE];
	FILE *fp;
	size_t bytesRead=0;
	string errorMsg;

	if( fd < 0){
		cout << BADF << endl;
		return;
	}

	if(filename[0] == '/'){
		filename.replace(0,1, rootDir);
	}

	fp = fopen(filename.c_str() , "rb");
	if( fp == NULL ){
		errorMsg = getError();
		errorMsg = filename + ": " + errorMsg;
		cout << errorMsg << endl;
		sendMessage(errorMsg.c_str(), errorMsg.length(), ERROR, NF, fd);
		return;
	}

	if(validDir(filename.c_str())){
		errorMsg = ISDIR;
		cout << errorMsg << endl;
		sendMessage(errorMsg.c_str(), errorMsg.length(), ERROR, NF, fd);
		fclose(fp);
		return ;
	}

	sendMessage("OK\n", 3, DATA, NF, fd);
	do
	{	
		bytesRead = fread( buffer, 1, DATA_SIZE, fp );
		if(ferror(fp) != 0){
			errorMsg = getError();
			cout << errorMsg << endl;
			sendMessage(errorMsg.c_str(), errorMsg.length(), ERROR, NF, fd);
			fclose(fp);
			return;
		}
		if(bytesRead < DATA_SIZE)
			sendMessage(buffer, bytesRead, DATA, NF, fd);
		else sendMessage(buffer, bytesRead, DATA, MF, fd);
	}while(bytesRead == DATA_SIZE);

	fclose(fp);

	cout << "File Sent" << endl;

}


//Gets a file from client and saves it
void Server::putFile(string filename, int fd){

	FILE *fp;
	int rv;
	int index = 0;

	index = filename.rfind("/");
	if(index != string::npos){
		filename = filename.substr(index+1);
	}

	cout << filename << endl;

	if(fd < 0){
		cout << BADF << endl;
		return;
	}

	fp = fopen(filename.c_str() , "wb");
	if(fp == NULL){
		perror(filename.c_str());
		return;
	}

	do{
		rv = receiveMessage(fd, fp);
	}while(rv == 0);

	fclose(fp);
	cout << "Put File Completed" << endl;
}

//Creates a socket and binds it to the port
int Server::createSocket(string port){
	//
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int sockfd;

	memset(&hints, 0 , sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if( (rv = getaddrinfo(NULL, port.c_str() ,&hints, &servinfo )) != 0 ){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv) );
	}

	for( p=servinfo; p != NULL ; p = p->ai_next ){

		if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
			perror("socket");
			continue;
		}

		if( bind(sockfd, p->ai_addr, p->ai_addrlen) == -1 ){
			close(sockfd);
			perror("bind");
			continue;
		}

		break;

	}

	if( p == NULL ){
		fprintf(stderr, "Failed to create socket\n" );
		exit(1);
	}

	freeaddrinfo(servinfo);
	return sockfd;

}


//Return the error message
//using errno as the key
string Server::getError(){
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
		case ENOTDIR:
			errMsg = NOTDIR;
			break;
		default:
			errMsg = DEFERR;
	}

	return errMsg;
}

//Send Error messages
//to client
void Server::sendMessage(const char *message, size_t length, int type, int moreFrags, int fd){
	size_t i;

	if(fd < 0){
		cout << BADF << endl;
		return;
	}

	char buffer[length + 4];
	buffer[0] = (char)type;
	buffer[1] = (char)moreFrags;
	*((unsigned short *)&buffer[2]) = (unsigned short) length;
	for(i=0;i<length;i++){
		buffer[4+i] = message[i];
	}
	send(fd, buffer, length+4, 0);
	if(verbose){
		cout << "sent " << length << " bytes of data" << endl;
	}
}

//Receive message from server
int Server::receiveMessage( int fd, FILE *fp){
	unsigned short bytesToRead, bytesRead=0, length;
	char type, moreFrags;
	char *message, *temp;

	if(fd < 0){
		cout << BADF << endl;
		return 1;
	}

	recv(fd, &type, 1, 0);
	recv(fd, &moreFrags, 1, 0);
	recv( fd, (unsigned short *) &bytesToRead, 2 , 0 );
	message = (char *) malloc(sizeof(char) * bytesToRead);
	length = bytesToRead;
	temp = message;
	while(bytesToRead > 0){
		bytesRead = recv( fd, message, bytesToRead, 0 );
		bytesToRead -= bytesRead;
		message += bytesRead;
	}
	message = temp;
	if(type == DATA && fp != NULL)
		fwrite(message, 1, length, fp);
	else {
		fwrite(message, 1, length, stdout);
		if(type == ERROR)
			cout << endl;
	}

	free(message);

	if(verbose){
		cout << "Received " << length << " bytes of data" << endl;
	}

	if(type == ERROR)
		return 1;
	if(moreFrags == NF)
		return 2;

	return 0;

}


