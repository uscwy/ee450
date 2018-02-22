INTRODUCTION
------------

The prject contains a simple TCP server and client implemented in C language.
The client is an interactive program. It will ask user to enter a number and 
then send the number to server, display the result server returned. 

The server listens on specified TCP port. When receiving a number sent by 
client, server decomposite it into prime factors and send the results back 
to client.

For better performance, server use epoll and pthread to handle requests from 
client. 

 * The project is located at:
   https://github.com/uscwy/ee450project1
   
   client.cpp     Implementation of client
   server.cpp     Implementation of server
   test.sh	  A simple script to test memory leak and exceptions


REQUIREMENTS
------------

This project need specified VM to compile and run.


MAINTAINER
------------

Yong Wang <yongw@usc.edu>

