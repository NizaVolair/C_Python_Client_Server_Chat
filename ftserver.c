/**************************************************************************************************
** Suthor: Niza Voalir
** Date: 05-29-16
** Description: Server program which sends a list of directory content or file data over 
** a socket connection to a client program
**************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>

/**************************************************************************************************
** Source 1 (sockets): http://easy-tutorials.net/c/linux-c-socket-programming/
** Source 2 (compilation): https://gcc.gnu.org/onlinedocs/gcc-2.95.2/gcc_2.html
** Source 3 (sockets): http://tldp.org/LDP/LG/issue74/tougher.html#3.1
** Source 4 (strings): http://stackoverflow.com/questions/2162758
** Source 5 (bzero/memset): http://pubs.opengroup.org/onlinepubs/009695399/functions/memset.html
** Source 6 (dir): http://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
** Source 7 (files): http://www.cprogramming.com/tutorial/cfileio.html
** Source 8 (files): http://www.tutorialspoint.com/cprogramming/c_file_io.htm
** Source 9 (dir): http://pubs.opengroup.org/onlinepubs/007908775/xsh/dirent.h.html
**************************************************************************************************/

void StartUp(int portNum);
void HandleRequest(int dataConnection, int commandNum);
void SendDirContents(int dataConnection);
/**************************************************************************************************
** Function Name: error
** Description: Handles major errors by printing message and exiting program
** PreCond: error message is passed
** PostCond: error message is displayed and program exits
**************************************************************************************************/
void error(errorMessage){
	printf("Error: %s\n", errorMessage);
	exit(1);
}

int main(int argc, char **argv){
    //Check for correct number of arguments
    if(argc != 2){
		error("incorrect number of arguments");	
    }

    //Validate port number
    int serverPort = atoi(argv[1]);
    if(!serverPort){
		error("incorrect server port number");	
    }

    //startUp the main connection and the data connection
    StartUp(serverPort);
}


/**************************************************************************************************
** Function Name: StartUp
** Description: Establishes main connection and data connection and gets/validates arguments
** PreCond: server port number has been passed through the command line
** PostCond: will loop recieving commands and arguments until error or quit
**************************************************************************************************/
void StartUp(serverPort){
	//variables for the data port connection to be set up, the two servers and connections
    int dataPort, mainServer, dataServer, mainConnection, dataConnection, commandNum;
	//structs for the 
	struct sockaddr_in mainAddr, dataAddr;
  	int addyLen = sizeof(struct sockaddr_in);

    //initiate the main connection 
    mainServer = socket(AF_INET, SOCK_STREAM, 0);
    mainAddr.sin_family = AF_INET;
    mainAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mainAddr.sin_port = htons(serverPort);
    if(bind(mainServer, (struct sockaddr *) &mainAddr, addyLen) < 0){
		error("main server could not bind");	
    }

    //listen and accept all connections
	
    listen(mainServer, 5);

    while(1){
        printf("Listening on main connection...\n");
		mainConnection = accept(mainServer, (struct sockaddr *) &mainAddr, &addyLen);

        //variables for message and to save the command (-l or -g) for use after connection
        char message[256];
        char dataPortString[256];


        //recieve first message (command) validate and write back same message to confirm 
        bzero( message, sizeof(message));
        if( read(mainConnection, message, sizeof(message)) < 0) {
			error("main connection couldn't read client message");
        }
		
		//to signify type of command use 1 for -1 and 2 for -g
		if(strcmp(message, "-l") == 0){
			commandNum = 1;
		}
		else if (strcmp(message, "-g") == 0) {
			commandNum = 2;
		}
		//if the command is invalid replace with an error message before return
		else {
			bzero(message, sizeof(message));
			strcpy(message, "invalid command");
			//send error message
			write(mainConnection, message, sizeof(message));
			commandNum = 0;
		}
		//test print of command from client
        printf("Command from Client: %s\n", message);	
		
		//confirm command or send error message
        write(mainConnection, message, sizeof(message));
		
		//if the command is valid continue
		if(commandNum != 0) {

			//recieve second message (port number) validate and write back same message to confirm 
			bzero(message, sizeof(message));
			if( read(mainConnection, message, sizeof(message)) < 0) {
				error("main connection couldn't read client message");
			}
			
			//test print of port number from client
			printf("Port Number from Client: %s\n", message);

			//establish data port connection
			dataPort = atoi(message);
			dataServer = socket(AF_INET, SOCK_STREAM, 0);
			dataAddr.sin_family = AF_INET;
			dataAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			dataAddr.sin_port = htons(dataPort);
			if(bind(dataServer, (struct sockaddr *) &dataAddr, addyLen) < 0){
				error("data server couldn't bind");
			}
			
			listen(dataServer, 1);
			dataConnection = accept(dataServer, (struct sockaddr *) &dataAddr, &addyLen);
			printf("Listening on data connection..\n");

			//sned the data connection and command number to handle the two request types
			HandleRequest(dataConnection, commandNum);		
			
			//close the data connection and keep listening on the main connection
		
			printf("Data connection closed.\n");
			close(dataConnection);
		}
	}
	return;
}

/**************************************************************************************************
** Function Name: HandleRequest
** Description: Determines request 1 is -l send directory contents 
**  2 is - g send file which is done locally by opening the file and sending to client
** PreCond: connection established and command 1 or 2 passed
** PostCond: either directory contents or file into will be sent to client
**************************************************************************************************/
void HandleRequest(dataConnection, commandNum) {
	char message[256];
	if(commandNum == 1) {
		SendDirContents(dataConnection);
	}
	//else command was -g so first message will be filename
	else if (commandNum == 2) {
		//receive initial message of filename
		bzero(message, sizeof(message));
		if(read(dataConnection, message, sizeof(message)) < 0 ) {
			error("data connection couldn't read message");
		}
		//Test print of client data connection confirmation
		printf("File Name from Client: %s\n", message);
		FILE *filePointer;
		filePointer = fopen(message, "r");
		bzero(message, sizeof(message));
		if(filePointer == NULL){
			strcpy(message, "file not found");
			printf("Sending Error: %s\n", message);
			write(dataConnection, message, sizeof(message));
		}
		else {
			printf("Transfering files...\n");
			while(fgets(message, sizeof(message), filePointer)){
				write(dataConnection, message, sizeof(message));
			}
			fclose(filePointer);
		}
	}
	else {
		error("horribly wrong command");
	}
	return;
}

/**************************************************************************************************
** Function Name: SendDirectoryContents
** Description: send the contents of the current working directory over a socket connection
** PreCond: socket connection establishes
** PostCond: directory contents will be sent to client
**************************************************************************************************/
void SendDirContents(dataConnection) {
	//message variable
	char message[256];	
	//receive initial message to verify connection to data socket 
	bzero(message, sizeof(message));
	if(read(dataConnection, message, sizeof(message)) < 0 ) {
		error("data connection couldn't read message");
	}
	//Test print of client data connection confirmation
	printf("Client Message: %s\n", message);
	
	//create directory pointer and struct to hold directory content info
	DIR *dp;
	struct dirent *ep;

	//set the pointer to the current working directory
	dp = opendir ("./");
	if (dp != NULL)
	{
		//loop to read all contents and send to client
		while (ep = readdir (dp)) {
			//puts (ep->d_name);
			
			bzero(message, sizeof(message));
			strcpy(message, ep->d_name);
			//send directory content to client
			write(dataConnection, message, sizeof(message));
			
			//recieve client confirmation
			bzero(message, sizeof(message));
			if(read(dataConnection, message, sizeof(message)) < 0 ) {
				error("data connection couldn't read message");
			}	
		}
		(void) closedir (dp);
	}
	else {
		error ("couldn't open the directory");
	}
	//Send and recieve final confirmations
	bzero(message, sizeof(message));
	stpcpy(message, "Directory listing complete");
	write(dataConnection, message, sizeof(message));
	bzero(message, sizeof(message));
	if(read(dataConnection, message, sizeof(message)) < 0 ) {
		error("data connection couldn't read message");
	}
	printf("Client Message: %s\n", message);
	return;
}
