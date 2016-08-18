### UCLA CS118 (Spring 2016)
======
### Project 1: Simple HTTP Client and Server
======
The project contains two parts: a Web server and a Web client. The Web server accepts an HTTP request, parses the request, and looks up the requested file in the local file system. If the file exists, the server returns the content of the file as part of HTTP response, otherwise returning HTTP response with the corresponding error code. After retrieving the response from the Web server, the client saves the retrieved file in the local file system.
The basic part of this project only requires HTTP 1.0: the client and server will talk to each other through non-persistent connections. If client and/or server supports HTTP 1.1, bonus points will be given.

### Project 2: Simple TCP-like Transport Protocol over UDP
======
The developed client should establish connection to the developed server, after which the server will need to transfer data to the client. Individual packets sent by the client and the server may be lost or reordered. Therefore, the task is to make reliable data transfer by implementing parts of TCP such as the sequence number, the acknowledgement, and the congestion control.

### Authors
======
JUN FENG <br/>
WENBO YE <br/>
LONGJIA NIU

