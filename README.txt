Author: Niza Volair
Date: 05-29-2916
Description: Instructions to begin server/client transfer program


1) TO START SERVER on HOST A:
Connect to: flip1.engr.oregonstate.edu, flip2.engr.oregonstate.edu, etc.
If needed, get server name: hostname 

Compile: gcc ftserver.c -o ftserver

Run: “ftserver SERVER_PORT

SERVER_PORT numbers 20020, 20025, 20035, etc. were tested
For example, try “ftserver 20020”

If there is an exception, try another port number

When server starts the following message will display
“Listening on main connection..."


2) TO START CLIENT on HOST B:

Add executable permission: “chmod +x ftclient.py”
Run: “ftclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND> (optional FILENAME) <DATA_PORT>”

SERVER_PORT must be the same as that of the servers and server must be running
SERVER_HOST should be the same as what was found in step 1
For example, “ftclient.py flip3.engr.oregonstate.edu 20020 -l yourFile.txt 20021”

If there is a connection problem check the flip number

When client is started sucessfully the following messages will display:
"Command from Client: <your command>"
"Port Number from Client: <your port number>"

Server will then display:
"Listening on data connection.."

Depending upon the command, messages about errors, file transfer, or directory list transfer will apppear.
There is no need for additional input.

If there basic errors in the commands, the client will not establish a connection.
If the file name is a duplicate, the client program will not attempt a connection.
If the file name is wrong, the server will send a message to the client and the client will exit.
If the command is wrong, the server will send a message to the client and the client will exit.
If port numbers are reused too soon, you may get a broken pipe error. Just try another port number.
