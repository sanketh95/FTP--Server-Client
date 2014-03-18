/*
Authors: Sanketh Mopuru, Sriharsha Bandaru
Description: Contains the constants for Server class
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

//Constants
#define ROOT_DIR "/srv/ftp"
#define CTRL_PORT "22"
#define BACKLOG 10
#define DATA_SIZE 1024

//Error constants
#define INVALID_OP 0

//Namespace uses
using namespace std;

//Some Flags
#define ERROR 0
#define DATA 1
#define MF 1
#define NF 0

//Error Messages
#define BADF "Bad file descriptor"
#define OPINT "Operation interrupted"
#define NOSPC "No space available on disk"
#define ISDIR "Is a directory"
#define DEFERR "Error Occured"
#define NOTDIR "Not a directory"
#define NOTFOUND "No such file or directory"
#define BADF "Bad file descriptor"
#define INVCOM "Invalid FTP Command"
#define INV_ARGS "Invalid arguments"

//Some prompt messages
#define DIRCOM "Directory change successful"
#define PWDCOM "PWD successful"

//The Server class definition
//Attributes: server root directory and control port

class Server
{
public:
	string rootDir;						//Root Directory
	string controlPort;		//Control Port	
	Server(string , string, bool);			//Constructor
	void start();						//start the server

private:
	string pwd;
	int controlSock;
	int dataSock;
	bool verbose;
	bool validDir(const char *);		//Returns true if valid directory otherwise returns false
	void getFile(string, int);		//Send a file to the client
	void putFile(string, int);		//Receives file from client and saves in the server
	void cd(string, int);					//Changes the PWD to dirpath if valid path
	void ls(string, int);					//List all the files and folders in the directory
	void PWD(int);			 	//Prints the present working directory
	bool getPWD();				//Gets the current pwd by calling getcwd()
	int createSocket( string );		//create socket and bind to the port passed as argument
	string getError();			//Returns error message strings based on type of error
	void sendMessage(const char *, size_t , int , int, int);		//Send message to server 
	int receiveMessage(int,FILE *);		//Receive message from server
};

