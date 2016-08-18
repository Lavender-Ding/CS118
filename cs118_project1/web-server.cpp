#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <map>
#include <stdio.h>
#include <iostream>
#include <sstream>
//#include <string.h>
#include "HttpAbstract.h"

#define BUFSIZE 2048
#define TIMEOUT 30

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
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

    const char* hostname;
    const char* port;
    std::string file_dir;
    
    if(argc<4||argc>4){
        if(argc!=1)
            fprintf(stdout, "Usage:web_server [host] [port] [file_dir]\n");
        hostname="localhost";
        port="4000";
        file_dir=".";
    }else{
        hostname=argv[1];
        port=argv[2];
        file_dir="."+std::string(argv[3]);
    }

    
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop serverinfo until we bind successfully
    for(p = servinfo; p != NULL; p = p->ai_next) {
        
        // create a socket using TCP IP
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }
        // allow others to reuse the address
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        // bind address to socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen)==-1) {
            close(sockfd);
            perror("bind");
            continue;
        }
        
        break; // if we get here, we must have binded successfully
    }
    
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    freeaddrinfo(servinfo); // free the linked list

  
  if (listen(sockfd, 10) == -1) { // set socket to listen status
    perror("listen");
    exit(1);
  }
    sa.sa_handler = sigchld_handler; //clean zombie processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

   fprintf(stdout,"server: waiting for connections...\n");
    
  
  while(1){// accept a new connection
    
      struct sockaddr_in clientAddr;
      socklen_t clientAddrSize = sizeof(clientAddr);
      int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

      if (clientSockfd == -1) {
        perror("accept");
        return 4;
      }

      char ipstr[INET_ADDRSTRLEN] = {'\0'};
      inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
      //convert IP addresses to human-readable form and store in ipstr
      std::cout << "Accept a connection from: " << ipstr << ":"<<ntohs(clientAddr.sin_port) << std::endl;
      //convert network byte order to host byte order

      // read/write data from/into the connection
     if(!fork())//child process
     {
         close(sockfd);
         struct timeval begin, now;
         double timediff;
         char recvrequest[BUFSIZE];
         char sendresponse[BUFSIZE];
         size_t  recv_size;
         
         //beginning time
         gettimeofday(&begin , NULL);
         
     while(1) //keep the connnection for http/1.1
     {
         //get current time
         gettimeofday(&now , NULL);
         //time elapsed in seconds plus microseconds
         timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
         
         //close the connection after TIMEOUT
         if( timediff > TIMEOUT )
         {
             std::cout<<"HTTP/1.1: Connection closed after 30s.\n";
             close(clientSockfd);
             exit(0);
         }
         
         memset(&recvrequest, '\0', sizeof(recvrequest));
         
//        if ((recv_size=recv(clientSockfd, &recvrequest, sizeof(recvrequest), 0)) ==-1) {
//          perror("recv");
//          return 5;
//            
//        }
        if ((recv_size=recv(clientSockfd, &recvrequest, sizeof(recvrequest), 0)) >0)
        { //if we receive something, we process the request and send response. Otherwise we wait till timeout
            
            gettimeofday(&begin , NULL);
            //std::cout<<recv_size<<std::endl;
            std::cout << "receive: "<<std::endl;
            std::string recv(recvrequest);
            std::cout<<recv<<std::endl;
            
            FILE * pFile;
            long long lSize=0;
            unsigned char * buffer;
            std::string sbuffer;
            size_t result;
            ByteBlob enrequest;
            ByteBlob payload;
            HttpRequest request;
            HttpResponse response;
            
            enrequest.assign(recvrequest, recvrequest+recv_size);
            
            if(!request.consume(enrequest)||request.getHeader("host")=="none")
            { //if we cannot consume the request or the request contains no host headerline, we return 400 bad request
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
                    lSize = ftell (pFile);//tell the file size
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
                else{ //if we cannot open the file, we return 404 not found

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
            
            //if the version of request is http/1.1, we keep the connection for 30s
            if(request.getVersion().compare("HTTP/1.1")==0)
            {
                response.setHeader("Connection", "Keep-Alive");
                std::string value="timeout="+std::to_string(TIMEOUT)+", max=100";
                response.setHeader("Keep-Alive", value);
            }
            else
                response.setHeader("Connection", "close");
            ByteBlob enresponse=response.encode();
            
            int num=0;
            std::cout << "send: "<<std::endl;
      
            //divide the response into pieces of certain length and send in a loop in case the response is too large to send in one time
             while (num<enresponse.size())
             {
                memset(&sendresponse, '\0', sizeof(sendresponse));
                 
                for(int i=0;i<BUFSIZE;i++){
                    if(num>=enresponse.size())
                        break;
                    sendresponse[i]=enresponse[num];
                    num++;
                }
                 std::cout << sendresponse <<std::endl;
                if (send(clientSockfd, &sendresponse, sizeof(sendresponse), 0) == -1) {
                    perror("send");
                    return 6;
                }
             }
            
            //if the version of request is http/1.0, we close the connection immediately upon sending the response
            if(request.getVersion().compare("HTTP/1.1")!=0)
            {
                std::cout<<"Http 1.0: Data received. Connection closed.\n";
                close(clientSockfd);
                exit(0);
             }
        }

       }
     }
     else
       close(clientSockfd); //parent process
  }
  return 0;
}