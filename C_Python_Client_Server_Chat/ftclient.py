#!/usr/bin/env python

#########################################################################################
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HEADER~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#########################################################################################
#Author: Niza Volair                                                                    #
#Date: 05-29-16                              	                                        #  
#Description: This is a client program that connects to a server to chat                #
#########################################################################################

#########################################################################################
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SOURCES~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#########################################################################################
#Source 1 (general): Lectures, discussions, program discription                         #
#Soucre 2 (args): http://stackoverflow.com/questions/4033723                            # 
#Source 3 (sting to int): http://stackoverflow.com/questions/642154                     #  
#Source 4 (printing): http://learnpythonthehardway.org/book/ex5.html                    #
#Source 5 (sending parsable output):http://stackoverflow.com/questions/7757544          #  
#Source 6 (string length):  http://www.tutorialspoint.com/python/string_len.htm         #
#Source 7 (trucation): http://pythoncentral.io/cutting-and-slicing-strings-in-python/   #
#Source 8 (general): https://docs.python.org/release/2.6.5/library/internet.html		#
#Source 9 (duplicate file check): http://stackoverflow.com/questions/11968976			#
#Source 10 (files): http://www.pythonforbeginners.com/files								#
#Source 11 (types) :https://en.wikibooks.org/wiki/Python_Programming					#
#SOurce 12 (sleep) :http://stackoverflow.com/questions/11552320							#
#Source 13 (duplicate file check): http://stackoverflow.com/questions/82831				#
#Source 14 (flow control): https://docs.python.org/3/tutorial/controlflow.html			#
#########################################################################################



#########################################################################################
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#########################################################################################

#########################################################################################
#Function Name: error																    
#Description: prints optional error messgae and exits								
#PreCond1: Client socket established with API and pass					
#PreCond2: Server name and port gotten from comand line and pass			
#PostCond: Connection established							   
#########################################################################################
def error (errorMessage):
	if errorMessage > 0 :
		print "Error: " + errorMessage + "\n"
		exit(1)
	else :
		print "Error: Something went horribly wrong\n"
		exit(1)
	
#########################################################################################
#Function Name: InitiateContact								  
#Description: Use the socket API to establish a TCP connection to the server	
#PreCond1: Client socket established with API and pass					
#PreCond2: Server name and port gotten from comand line and pass			
#PostCond: Connection established							   
#########################################################################################
def InitiateContact(sock, name, port):
    try :
	sock.connect((name, port))
    except :
	error("socket connect couldn't be established")
    return

#########################################################################################
#Funtion Name: RecieveFile							
#Description: Gets transfered file from server and saves new file in current directory
#PreCond: dataSocket connection established and filename validated					
#PostCond: new transfered file in current working directory	
#########################################################################################
def ReceiveFile (dataSocket, fileName):
    #open a file to write to with the same file name
    dataSocket.send(fileName)
    message = ''
    message = dataSocket.recv(1024)
    if 'file not found' in message :
	print "Error: file not found" 
    else :
	with open(fileName, 'w+') as newFile:
	    print("Transfering to file...")
	    while message:
		#write the server message to client file
		newFile.write(message)
		message = dataSocket.recv(1024)
	newFile.close()
	print("Transfer complete")
    return
	
#########################################################################################
#Funtion Name: MakeRequest								
#Description: Sends the request to the server and lists server directory contents
# or gets transfered file depending upon command
#PreCond: dataSocket connection established and commands verifed 	
#PostCond: server contents are listed or file is created
#########################################################################################
def MakeRequest(clientSocket, serverName, command, fileName, dataPort) :
    #send the initial command and recieve same command then verify 
    clientSocket.send(command)
    response = ''
    response = clientSocket.recv(1024)
    if 'invalid command' in response:
		print "Error: invalid command"
		exit(1)
	   
    #set up data socket connection 
    dataSocket = socket(AF_INET, SOCK_STREAM)
    dataSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

    clientSocket.send(dataPort) 
    print "Attempting to set up data connection..."
    #initiate contact- convert dataport to an int, pause 1/2 second for set up delay
    InitiateContact(dataSocket, serverName, int(dataPort))
    time.sleep(.5)

    #if -l command, just print response (i.e. server directory contents)
    if(command == '-l') :
    	#send message to begin transmission
	print "Attempting directory list transfer..."
	dataSocket.send("begin directory list tranfer")
	response = dataSocket.recv(1024)
	while (response != "") :
		dataSocket.send("transfer confirmation")
		print response 
		response = dataSocket.recv(1024)

    #else response is -g  use RecieveFile to get text file from server to client
    else :
    	print "Attempting file transfer..."
        ReceiveFile(dataSocket, fileName)
  
    #close data socket
    dataSocket.close()
    return
	


	
	
#########################################################################################
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MAIN~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#########################################################################################

#import socket and system libraries
from socket import *
import os.path
import sys
import time 


#check argument length
if (len(sys.argv) < 5) or (6 < len(sys.argv)) :
    error("incorrect number of arguments")
	
#validate server name is a string and the server port is an int
if type(sys.argv[1]) is str :
    serverName = sys.argv[1]
else:
    error("incorrect server name")
if int(sys.argv[2]) > 0 :
    serverPort = int(sys.argv[2])
else:
    error("incorrect server port number")
if type(sys.argv[3]) is str :
    command = sys.argv[3]
else:
    error("incorrect command")
#if there are 5 arguments there is no filename
if len(sys.argv) == 5 :
    #validate the data port
    if int(sys.argv[4]) > 0 :
	dataPort = sys.argv[4]
	#set file name as none to signify there is no file  
	fileName = "none"
    else:
	error("incorrect data port number")
#else validate the additional filename argument and then the port number
else :
    if type(sys.argv[4]) is str :
	fileName = sys.argv[4]
	#ensure there is no file in the directory with the same name
	if os.path.isfile(fileName):
	    print "Error: duplicate file name"
	    exit(1)
	else:
	    error("incorrect file name")
	if int(sys.argv[5]) > 0 :
	    dataPort = sys.argv[5]
	else:
	    error("incorrect data port number")



#set up client and sockets with API
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

#initiate contact 
InitiateContact(clientSocket, serverName, serverPort)

#make a request to the server for directory contents or file transfer
MakeRequest(clientSocket, serverName, command, fileName, dataPort)

#Transfer is complete- close the socket, we're done!
print 'Program has ended. Goodbye!'
clientSocket.close()	

