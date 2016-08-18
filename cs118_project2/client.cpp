#include "TCPAbstract.h"

using namespace std;



int main(int argc, char **argv)
{
    struct sockaddr_in myaddr;	/* our address */
    struct sockaddr_in serveraddr;	/* server address */
    socklen_t addrlen = sizeof(serveraddr);		/* length of addresses */
    int recvlen=0;			/* # bytes received */
    int fd;				/* our socket */
    unsigned char recvbuf[BUFSIZE];	/* receive buffer */
    string sendmessage, recvmessage,seq,ack;
    uint16_t ackno=0,seqno=0;
    struct timeval begin, now, begin_fin;
    double timediff;
    bool isFin=false, isSynAck=false;
    Packet sendpacket,recvpacket;
    recvBuffer clientbuf(RCV_WINDOW_SIZE,MAX_SEQ_NO);
    int port, counter=0;
    const char* serverip;
    int remainWindowSize;
    
    if(argc!=3)
    {
        fprintf(stdout, "Usage: ./client SERVER-HOST-OR-IP PORT-NUMBER\n");
        fprintf(stdout, "Using default settings: SERVER-HOST-OR-IP=10.0.0.1 PORT-NUMBER=4000\n");
        port=SERVER_PORT;
        serverip="10.0.0.1";
        
    }
    else
    {
        serverip=argv[1];
        port=atoi(argv[2]);
        fprintf(stdout, "Using settings: SERVER-HOST-OR-IP=%s PORT-NUMBER=%d\n",serverip,port);
        
    }
    
    /* create a UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return 0;
    }
    
    /* bind the socket to valid IP address and a specific port */
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(CLIENT_PORT);
    myaddr.sin_addr.s_addr = inet_addr("10.0.0.2");
    
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }
    
    /* define serveraddr, the address to whom we want to send messages */
    memset((char *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(serverip);
    
    gettimeofday(&begin , NULL);//get beginning time
    
    /* send SYN packet to the server to establish the connection */
    uint16_t initseq=(uint16_t) rand()%(MAX_SEQ_NO+1);//clientbuf.initSeq(MAX_SEQ_NO);
    
    cout<<"Sending packet "<<to_string(initseq)<<" SYN"<<endl;
    
    sendpacket.setSeqNumber(initseq);
    sendpacket.setWindow(RCV_WINDOW_SIZE);
    sendpacket.setSyn(true);
    sendmessage=sendpacket.encode();
    //cout<<"send: "<<sendmessage<<endl;
    //cout<<"sendmessage size:"<<sendmessage.size()<<endl;
    if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
        perror("sendto");
    
    //make socket non blocking
    fcntl(fd, F_SETFL, O_NONBLOCK);
    
    /* wait for SYN-ACK packet from the server. If timeout occurs, resend SYN packet */
    while(1)
    {
        //if receive FIN packet, wait till timeout and close the connection
        if(isFin){
            //get current time
            gettimeofday(&now , NULL);
            //time elapsed in seconds plus microseconds
            timediff = (now.tv_sec - begin_fin.tv_sec) + 1e-6 * (now.tv_usec - begin_fin.tv_usec);
            //std::cout<<std::to_string(timediff)<<std::endl;
            
            //close connection after TIMEOUT
            if( timediff > TIMEOUT )
            {
                std::cout<<"FIN timeout: close the connection!\n"<<std::endl;
                close(fd);
                break;
            }
            
        }
        //get current time
        gettimeofday(&now , NULL);
        //time elapsed in seconds plus microseconds
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
        
        //resend SYN packet after TIMEOUT
        if( timediff > SYN_RTO && !isSynAck )
        {
            counter++;
            if(counter>10){
                cout<<"Connection setup failed. Abort client!"<<endl;
                return 0;
            }
            cout<<"Sending packet "<<initseq<<" Retransmission SYN"<<endl;
            //sendmessage="SYN "+std::to_string(initseq);
            if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
                perror("sendto");
            gettimeofday(&begin , NULL);//reset beginning time
        }
        
        //if receive SYN-ACK, send ACK packet; otherwise wait till timeout
        memset(&recvbuf, '\0', BUFSIZE);
        if ((recvlen=recvfrom(fd, recvbuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &addrlen)) >0)
        {
            if(!isSynAck){
            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(AF_INET, &(serveraddr.sin_addr), ipstr, sizeof(ipstr));
//            cout << "Receiving from:  " << ipstr << ":" << ntohs(serveraddr.sin_port) << endl;
            }
            recvmessage=string(reinterpret_cast<char*>(recvbuf));
            //cout<<"recv: "<<recvmessage<<endl;
            recvpacket.consume(recvbuf);
            
            if(recvpacket.getSyn()&&recvpacket.getAck()){
                //cout << "Receiving SYN-ACK packet "<<recvpacket.getAckNumber()<<" "<<recvpacket.getSeqNumber()<<endl;
                cout << "Receiving packet "<<recvpacket.getSeqNumber()<<endl;
                clientbuf.setInitAck(recvpacket.getSeqNumber(),1);
                seqno=recvpacket.getAckNumber();
                sendpacket.setSeqNumber(seqno);
                sendpacket.setAckNumber(clientbuf.getACK());
                sendpacket.setSyn(false);
                sendpacket.setAck(true);
                sendmessage=sendpacket.encode();
                
                if(isSynAck)
                   cout<<"Sending packet "<<clientbuf.getACK()<<" Retransmission"<<endl;
                else
                   cout<<"Sending packet "<<clientbuf.getACK()<<endl;
                
                if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
                    perror("sendto");
                isSynAck=true;
                clientbuf.initFile();
                
            }
            
        else
            if(!recvpacket.getSyn()&&!recvpacket.getFin()&&isSynAck){
                
                cout << "Receiving packet "<<recvpacket.getSeqNumber()<<endl;
                
                remainWindowSize=clientbuf.remainWindowSize(recvpacket.getSeqNumber());
                if(remainWindowSize>0||!clientbuf.isOutOfBuffer(recvpacket.getSeqNumber())){
                    
                    if(!clientbuf.isOutOfBuffer(recvpacket.getSeqNumber())){
                        ackno= clientbuf.insert(recvpacket.getSeqNumber(), recvpacket.getPayLoad());
                        clientbuf.writeToFile();
                    }else
//                        if(remainWindowSize==0&&clientbuf.isOutOfBuffer(recvpacket.getSeqNumber())){
//                            cout<<"Receive Window is zero!"<<endl;
//                        cout<<"Packet already received!"<<endl;
                    
//                    clientbuf.print();
                    
                    sendpacket.setSeqNumber(seqno);
                    sendpacket.setAckNumber(ackno);
                    
                    remainWindowSize=clientbuf.remainWindowSize(recvpacket.getSeqNumber());
//                    cout<<"Remain window size: "<<remainWindowSize<<endl;
                    sendpacket.setWindow(remainWindowSize);
                    
                    sendpacket.setAck(true);
                    sendmessage=sendpacket.encode();
                    
                    if(clientbuf.isRetransmit())
                        cout<<"Sending packet "<<ackno<<" Retransmission "<<endl<<endl;
                    else
                        cout<<"Sending packet "<<ackno<<endl<<endl;
                    //cout<<"send: "<<sendmessage<<endl;
                    
                    if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
                        perror("sendto");
                }
               
            }
        else
            if(recvpacket.getFin()&&isSynAck){
                cout << "Receiving packet "<<recvpacket.getSeqNumber()<<endl;
                
                ackno= recvpacket.getSeqNumber()+1;
                
                gettimeofday(&begin_fin , NULL);//get beginning time
                
                sendpacket.setSeqNumber(seqno);
                sendpacket.setAckNumber(ackno);
                sendpacket.setWindow(clientbuf.remainWindowSize(recvpacket.getSeqNumber()));
                sendpacket.setAck(true);
                sendmessage=sendpacket.encode();
                
                if(isFin)
                    cout<<"Sending packet "<<ackno<<" Retransmission"<<endl;
                else
                    cout<<"Sending packet "<<ackno<<endl;
                
                //cout<<"send: "<<sendmessage<<endl;
                
                if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
                    perror("sendto");
                isFin=true;
            }
        }
        //else
        //std::cout<<"Error receiving data packet!\n"<<std::endl;
    }
    
    return 0;
}