#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <map>

#include "HttpAbstract.h"

#define TIMEOUT 3 // 3s timeout for select
#define CONNECTIMEOUT 30 //30s timeout for connection

std::string getContentType(std::string url){
    
    std::map<std::string,std::string> content_type;
    content_type["htm"]="text/html";
    content_type["html"]="text/html";
    content_type["txt"]="text/plain";
    content_type["xml"]="text/xml";
    content_type["css"]="text/css";
    content_type["png"]="image/png";
    content_type["gif"]="image/gif";
    content_type["jpg"]="image/jpg";
    content_type["jpeg"]="image/jpeg";
    content_type["zip"]="application/zip";
    content_type["mp3"]="audio/mp3";
    content_type["wav"]="audio/wav";
    content_type["aiff"]="audio/aiff";
    content_type["mp4"]="video/mp4";
    content_type["mpg"]="video/mpg";
    content_type["wmv"]="video/wmv";
    
    size_t pos = url.find_last_of(".");
    if(pos==-1)
        return "unknown type" ;
    std::string postfix=url.substr(pos+1);
    if(content_type.find(postfix)==content_type.end())
        return "unknown type" ;
    
    return content_type[postfix];
    
}
int main(int argc, const char* argv[])
{
	// get parameters from console.
	const char* hostname;
    const char* port;
    std::string file_dir;
    if(argc<4||argc>4)
	{
        if(argc!=1)
            fprintf(stdout, "Usage:web_server [host] [port] [file_dir]\n");
        hostname="localhost";
        port="4000";
        file_dir=".";
    }
	else
	{
        hostname=argv[1];
        port=argv[2];
        file_dir="."+std::string(argv[3]);
    }
		
  fd_set readFds;
  fd_set errFds;
  fd_set watchFds;
  FD_ZERO(&readFds);
  FD_ZERO(&errFds);
  FD_ZERO(&watchFds);

  // get the address hostname port
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	
	 int listenSockfd;
	 int clientSockfd;
    
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{      
        // create a socket using TCP IP
        if ((listenSockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
            perror("socket setup error !\n");
            continue;
        }
        // allow others to reuse the address
        int yes = 1;
        if (setsockopt(listenSockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
            perror("setsockopt address reusing error !\n");
            continue;
        }       
        // bind address to socket
        if (bind(listenSockfd, p->ai_addr, p->ai_addrlen)==-1) 
		{
            close(listenSockfd);
            perror("bind error !");
            continue;
        }     
        break; // if we get here, we must have binded successfully
    }
	// After iterating all the servinfo, we find nothing qualified
    if (p == NULL) 
	{
        fprintf(stderr, "server: failed to bind ! Begin exitting\n");
        return 2;
    }
    freeaddrinfo(servinfo); // free the linked list
	
  int maxSockfd = listenSockfd;

  // put the socket in the socket set
  FD_SET(listenSockfd, &watchFds);

  // set the socket in listen status
  if (listen(listenSockfd, 10) == -1) 
  {
    perror("listen error! Begin exitting\n");
    return 2;
  }
  
  fprintf(stdout,"asynchronous server: waiting for connections...\n");
  // initialize timer (3s)
    
    struct timeval tv;
    struct timeval begin, now;
    double timediff;
    std::map<int,long long> connection;
    std::map<int,long long>::iterator it;
    
  while (true) 
  {
    // set up watcher

    int nReadyFds = 0;
    readFds = watchFds;
    errFds = watchFds;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
      
    if ((nReadyFds = select(maxSockfd + 1, &readFds, NULL, &errFds, &tv)) == -1) 
	{
      perror("select error! Begin exitting\n");
      return 3;
    }

    if (nReadyFds == 0) 
	{
       //std::cout << "no data is received for 3 seconds!" << std::endl;
        for(int fd = 0; fd <= maxSockfd; fd++)
        {
            gettimeofday(&now , NULL);
            it=connection.find(fd);
            if(it!=connection.end()){
                timediff = now.tv_sec+ 1e-6 * now.tv_usec-it->second;
                
                if( timediff > CONNECTIMEOUT )
                {
                    std::cout<<"HTTP/1.1: Connection closed with socket ID "+std::to_string(fd)+" after 30s.\n";
                    close(fd);
                    FD_CLR(fd, &readFds);
                    connection.erase(it);
                    maxSockfd--;
                }
            }
        }
    }
    else 
	{
      for(int fd = 0; fd <= maxSockfd; fd++) 
	  {
          //beginning time
          gettimeofday(&begin , NULL);
          
        // get one socket for reading
        if (FD_ISSET(fd, &readFds)) 
		{
          if (fd == listenSockfd) 
		  { // this is the listen socket
            struct sockaddr_in clientAddr;
            socklen_t clientAddrSize = sizeof(clientAddr);
            clientSockfd = accept(fd, (struct sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSockfd == -1) 
			{
              perror("accept error\n");
              return 5;
            }

            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
            std::cout << "Accept a connection from: " << ipstr << ":" <<
              ntohs(clientAddr.sin_port) << " with Socket ID: " << clientSockfd << std::endl;

            // update maxSockfd
            if (maxSockfd < clientSockfd)
              maxSockfd = clientSockfd;

            // add the socket into the socket set
            FD_SET(clientSockfd, &watchFds);
              
              //add beginning time to the new socket
              connection[clientSockfd]=begin.tv_sec+ 1e-6 * begin.tv_usec;
          }
          else 
		  { // this is the normal socket
            char recvRequest[2048];
			memset(&recvRequest, '\0', sizeof(recvRequest));
            int recvLen = 0;
            
            if ((recvLen = (int)recv(fd, recvRequest, sizeof(recvRequest), 0)) == -1) 
			{
              perror("recv error! begin exitting");
              return 6;
            }

            std::string output(recvRequest);
            if (recvLen == 0) 
			{
              close(fd);
              FD_CLR(fd, &watchFds);

            }
            else 
			{
              //std::cout << "Socket " << fd << " receives: " <<std::endl;
			  std::cout << "****************** Server: what we get from the client ******************* "<<std::endl;
			  std::cout << output << std::endl;

                FILE * pFile;
                long lSize=0;
                unsigned char * buffer;
                std::string sbuffer;
                size_t result;
                ByteBlob enrequest;
                ByteBlob payload;
                HttpRequest request;
                HttpResponse response;
                
                enrequest.assign(recvRequest, recvRequest+recvLen);
                if(!request.consume(enrequest)||request.getHeader("host")=="none"){
                    sbuffer="Your browser sent a request that this server could not understand.";
                    payload.assign(sbuffer.begin(),sbuffer.end());
                    response.setPayLoad(payload);
                    response.setStatus(400);
                    response.setDescription("Bad request");
                    response.setHeader("Content-Type", "text/html");
                    response.setHeader("Content-Length", std::to_string(sbuffer.length()));
                }
                else{
                    std::string sfile=file_dir+request.getUrl();
                    //std::cout<<sfile<<std::endl;
                    const char *cfile=sfile.c_str();
                    
                    
                    if((pFile=fopen(cfile, "r"))!=nullptr)
                    {
                        response.setStatus(200);
                        response.setDescription("OK");
                        
                        // obtain file size:
                        fseek (pFile , 0 , SEEK_END);
                        lSize = ftell (pFile);
                        rewind (pFile);//set position of stream to the beginning
                        
                        // allocate memory to contain the whole file:
                        if ((buffer = (unsigned char*) malloc (lSize)) == NULL)
                        {
                            fputs ("Memory error",stderr);
                            return 6;
                        }
                        memset(buffer,0,lSize);
                        
                        // copy the file into the buffer:
                        if ((result = fread (buffer,1,lSize,pFile)) != lSize)
                        {
                            fputs ("Reading error",stderr);
                            return 7;
                        }
                        //std::cout<<"read buffer:"<<buffer<<std::endl;
                        payload.insert(payload.end(),buffer,buffer+lSize);
                        response.setPayLoad(payload);
                        
                        fclose (pFile);
                        response.setHeader("Content-Type", getContentType(request.getUrl()));
                        response.setHeader("Content-Length", std::to_string(lSize));
                        
                    }
                    else{
                        sbuffer="The requested URL "+request.getUrl()+" was not found on this server.";
                        payload.assign(sbuffer.begin(),sbuffer.end());
                        response.setPayLoad(payload);
                        response.setStatus(404);
                        response.setDescription("Not found");
                        response.setHeader("Content-Type", "text/html");
                        response.setHeader("Content-Length", std::to_string(sbuffer.length()));
                    }
                }
                
                response.setVersion(request.getVersion());
                response.setHeader("Server", "Apache/2.4.18 (FreeBSD)");
                if(request.getVersion().compare("HTTP/1.1")==0)
                {
                    response.setHeader("Connection", "Keep-Alive");
                    std::string value="timeout="+std::to_string(CONNECTIMEOUT)+", max=100";
                    response.setHeader("Keep-Alive", value);
                }
                else
                    response.setHeader("Connection", "close");
                ByteBlob enResponse=response.encode();

				 int num=0;
				 char sendResponse[2048];

				std::cout << std::endl;
				std::cout << "****************** Server: what we send to the client ******************* "<<std::endl;
                while(num<enResponse.size())
                {
                    memset(&sendResponse, '\0', 2048);
                    
                    for(int i=0;i < 2048;i++)
                    {
                        if(num >= (int)enResponse.size())
                            break;
                        sendResponse[i]=enResponse[num];
                        num++;
                    }
                    std::cout << sendResponse <<std::endl;
                    
                    if (send(fd, &sendResponse, sizeof(sendResponse), 0) == -1) 
                    {
                        perror("send error ! begin exitting\n");
                        return 8;
                    }
                }
                
                //reset beginning time of the socket
                connection[fd]=begin.tv_sec+ 1e-6 * begin.tv_usec;
                
                //if the version of request is http/1.0, close the connection and remove the socket from the readfd set and connection map
                if(request.getVersion().compare("HTTP/1.1")!=0)
                {
                    std::cout<<"Http 1.0: Data received. Connection closed.\n";
                    close(fd);
                    FD_CLR(fd, &readFds);
                    it=connection.find(fd);
                    connection.erase(it);
                    maxSockfd--;
                }
            }

			
          }
        }
      }
    }
      
  }

  return 0;
}