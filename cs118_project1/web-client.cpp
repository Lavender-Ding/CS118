#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include "HttpAbstract.h"
#include <map>
#include <string>
#define BUFSIZE 100000
#define TIMEOUT 5

using namespace std;


string host="";
string port="80";
string filepath="/";

int sockfd=-1;
// parse host, port and filepath from URL
void parseURL(string url, string &host, string &port,string &filepath) {
// do not support https, http:// can be omitted in the URL
    string http("http://");
// if URL contains http://
    if (url.compare(0, http.size(), http) == 0) {
        //try to find port or filepath
        int pos = url.find_first_of("/:", http.size());
        //no port or filepath
        if (pos == -1) {
            host=url.substr(http.size());
            return;
        }
        // get host
        host=url.substr(http.size(), pos-http.size());
        // if exist :, there exists port
        if (pos < url.size() && url[pos] == ':') {
            // try to find filepath
            int ppos = url.find_first_of("/", pos);
            // no filepath
            if (ppos == -1) {
                port=url.substr(pos+1);
            }
            else
            {
                port=url.substr(pos+1, ppos-pos-1);
                filepath=url.substr(ppos);
                
            }
            
            
        }
        // no port in URL
        else{
            filepath=url.substr(pos);
        }
    }
//if URL does not contain http://
    else {
        int pos = url.find_first_of("/:", 0);
        if (pos == -1) {
            host=url;
            return;
        }
        host=url.substr(0, pos);
        if (pos < url.size() && url[pos] == ':') {
            // A port is provided
            int ppos = url.find_first_of("/", pos);
            if (ppos == -1) {
                port=url.substr(pos+1);
            }
            else
            {
                port=url.substr(pos+1, ppos-pos-1);
                filepath=url.substr(ppos);
            }
            
            
        }
        else{
            filepath=url.substr(pos);
        }
    }
    
}


int main(int argc, char *argv[])
{
    // create a socket using TCP IP
    
    map<string,vector<string>> table;
    map<string,vector<string>>::iterator it;
    string arg;
    if (argc < 2) {
        fprintf(stderr,"usage: client [URL]\n");
        exit(1);
    }
    else
    {
        for(int i=1;i<argc;i++)
        {
            host="";
            port="80";
            filepath="/";
            arg=argv[i];
            parseURL(arg,host,port,filepath);
            string host_and_port=host+":"+port;
            
            it=table.find(host_and_port);
            if(it==table.end())
            {
                vector<string> vs;
                vs.push_back(filepath);
                table.insert(pair<string,vector<string>>(host_and_port,vs));
                
            }
            else
            {
                table[host_and_port].push_back(filepath);
//                cout<<"add "<<filepath<<" to "<<host_and_port<<endl;
                
            }
            
        }
    }
    for (it=table.begin(); it!=table.end(); it++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        string host_and_port=it->first;
        vector<string> vs=it->second;
        
//
//        cout<<"There is"<<vs.size()<<"file request"<<endl;
//
        int pos = host_and_port.find_last_of(":");
        host = host_and_port.substr(0,pos);
        port = host_and_port.substr(pos+1);
        //    cout<<"host: "<<host<<endl;
        //    cout<<"port: "<<port<<endl;
        //    cout<<"filepath: "<<filepath<<endl;
        //    cout<<"filename: "<<filename<<endl;
        
        
        struct addrinfo hints;
        struct addrinfo* res;
            // prepare hints
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET; // IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        
        
        const char *hostchar = host.c_str();
            // get address
        int status = 0;
        if ((status = getaddrinfo(hostchar, "http", &hints, &res)) != 0) {
            cerr << "getaddrinfo: " << gai_strerror(status) << endl;
            return 2;
        }
        
        //    cout << "IP addresses for " << host << ": " << endl;
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
                // convert address to IPv4 address
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
        
                // convert the IP to a string and print it:
        
            inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        //        std::cout << "  " << ipstr << std::endl;
                // std::cout << "  " << ipstr << ":" << ntohs(ipv4->sin_port) << std::endl;
        }
        freeaddrinfo(res); // free the linked list
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(stoi(port));     // short, network byte order
        serverAddr.sin_addr.s_addr = inet_addr(ipstr);
        memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
        
            // connect to the server
        if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("connect");
            return 2;
        }
        
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
            perror("getsockname");
            return 3;
        }
        
//        cout<<"trying to set up connect..."<<endl;
        
        char ipstr2[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr2, sizeof(ipstr2));
        cout << "Set up a connection from: " << ipstr2 << ":" <<
        ntohs(clientAddr.sin_port) << std::endl;
        
        
        for (std::vector<string>::iterator it2 = vs.begin() ; it2 != vs.end(); ++it2)
        {
            
            string filepath=*it2;
            //get filename from filepath, the file is saved at the current path.
            string filename="index.html";
            if(filepath.length()>1)
            {
                int pos = filepath.find_last_of("/");
                filename = filepath.substr(pos+1);
            }else
                filepath="/"+filename;
            
            HttpRequest hr;
            hr.setVersion("HTTP/1.1");
            hr.setUrl(filepath);
            hr.setMethod("GET");
            hr.setHeader("User-Agent","Wget/1.15 (linux-gnu)");
            hr.setHeader("Accept","*/*");
            hr.setHeader("host",host+":"+port);
//            hr.setHeader("Keep-Alive","300");
            ByteBlob request=hr.encode();
            string input=string(request.begin(), request.end());
            cout<<input<<endl;
            if (send(sockfd, input.c_str(), input.size(), 0) == -1)
                {
                    perror("send");
                    return 4;
                }
            
            //refer to http://www.binarytides.com/receive-full-data-with-recv-socket-function-in-c/
            vector<char> data;
            int size_recv=0 , total_size= 0;
            struct timeval begin , now;
            unsigned char buf[BUFSIZE] = {0};
            double timediff;
            
                //make socket non blocking
            fcntl(sockfd, F_SETFL, O_NONBLOCK);
            
                //beginning time
            gettimeofday(&begin , NULL);
            cout<<"start to receive..."<<endl;
            bool first_packet_or_not=true;
            HttpResponse httpresponse;
            ofstream out;
            
            while(1)
            {
                gettimeofday(&now , NULL);
            
                    //time elapsed in seconds
                timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
            
                    //if you got some data, then break after TIMEOUT
                if( total_size > 0 && timediff > TIMEOUT )
                {
                    break;
                }
            
                    //if you got no data at all, wait a little longer, twice the TIMEOUT
                else if( timediff > TIMEOUT*2)
                {
                    break;
                }
            
                memset(buf ,0 , BUFSIZE);  //clear the variable
                if((size_recv =  recv(sockfd, buf , BUFSIZE , 0) ) <= 0)
                {
                 //if nothing was received then we want to wait a little before trying again, 0.1 seconds
                    usleep(100000);
                }
                else
                {
                    gettimeofday(&begin , NULL);
                    if(first_packet_or_not)
                    {
                        total_size += size_recv;
                        data.insert(data.end(),buf,buf+size_recv);////
                        httpresponse.consume(data);
                        ByteBlob payload=httpresponse.getPayload();
                        
                        if(httpresponse.getStatus()==200)
                        {
                            cout<<httpresponse.getVersion()<<" "<<httpresponse.getStatus()<<" "<<httpresponse.getDescription()<<endl;
                            cout<<httpresponse.getAllHeaders()<<endl;
                            cout<<"The file has been saved successfully!"<<endl<<endl;
                            out.open(filename);
                            out << string(payload.begin(), payload.end());
//                         cout << string(payload.begin(), payload.end()) << endl;
                        }
                        else
                        {
                            cout << string(data.begin(), data.end()) << endl;
                            break;
                        }
                        first_packet_or_not=false;
                        
                        
                        
                    }
                    else{
                        total_size += size_recv;
                        data.insert(data.end(),buf,buf+size_recv);////
                        //cout << string(data.begin(), data.end()) << endl;
                        out << string(data.begin(), data.end());

                        
                    }
                    data.clear();
                    
                }
            }
                out.close();
            
                
//                HttpResponse httpresponse;
//                cout<<"for test1"<<endl;
//                httpresponse.consume(data);
//                cout<<"for test2"<<endl;
//                ByteBlob payload=httpresponse.getPayload();
//                cout << "size of data: "<<data.size()<<endl;
//            
//
//                if(httpresponse.getStatus()==200)
//                {
//                    cout << "payload: "<<endl;
//                    cout << string(payload.begin(), payload.end()) << endl;
//                    ofstream out(filename);
//                    out << string(payload.begin(), payload.end());
//                    out.close();
//                }
        }
        close(sockfd);
    }
    
    
   
   
    return 0;
}