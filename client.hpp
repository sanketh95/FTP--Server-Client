/*
Authors: Sanketh Mopuru, Sriharsha Bandaru
Description: Contains the constants for client class
*/

//Includes
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <fstream>

using namespace std;

//Constants
#define SRV_PORT "22"
#define SRV_IP "localhost"
#define DATA_SIZE 1024
#define BUFFER_SIZE 1028

//Flags
#define DATA 1
#define ERROR 0
#define MF 1
#define NF 0

//Error messages
#define BADF "Bad file descriptor"
#define OPINT "Operation interrupted"
#define NOSPC "No space available on disk"
#define ISDIR "Is a directory"
#define DEFERR "Error Occured"
#define NODIR "No such directory"
#define NOTFOUND "No such file or directory"

class Client
{
public:
	/*Attributes*/
	int controlSock;									//Socket descriptor
	int dataSock;
	string serverIp;						//Ip of server
	string serverPort;							//Server port

	/*Methods*/
	Client(string, string);									//Constructor
	void Connect();		//Connect to the server
	void putFile(string);			//Send file <filename> to the server
	void getFile(string);			//Receive the file <filename> from the server
	void ls_srv(string);							//List directories in the server
	void ls(string);								//List directories in the client
	void cd_srv(string);					//Change directory in the server
	void cd(string);								//Change directory in the client
	void pwd_srv();							//PWD of server
	void pwd();								//PWD of client
	void quit();							//quit the client program
	int receiveMessage(FILE *);				//receive message through data port
	string getError();						//Get the error message based on type of error
	void sendMessage(const char *, size_t , int , int);	//send Message to server
	int createSocket(string);		//create socket and connect to the server
	bool validDir(const char *);		//is the directory valid ?
};