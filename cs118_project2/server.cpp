#include "TCPAbstract.h"
int state;
#define WAIT_FOR_SYN 0
#define SYNACK_SENT 1
#define NORMAL 2
#define FIN_SENT 3
int stage;
#define SLOW_START 0
#define CONGESTION_AVOIDANCE 1


int main(int argc, const char* argv[])
{
	struct sockaddr_in myaddr, remaddr;
    unsigned int slen=sizeof(remaddr);
	unsigned char* buf;	/* message buffer */
	socklen_t recvlen;		/* # bytes in acknowledgement message */
    buf=(unsigned char*)malloc(BUFSIZE*sizeof(unsigned char));
    const char* file;
    FileReader reader;
    std::string sendmessage;
    ServerBuffer oBuf;
    Segment segment;
    Segment* seg;
    seg=(Segment*)malloc(BUFSIZE*sizeof(Segment));
    uint16_t seqnum=0;
    uint16_t finseq=0;
    uint16_t initAck=0;
    uint16_t initSeq=0;
    double cwnd_packet=1;
    int port;
    if(argc!=3)
    {
        fprintf(stdout, "Usage: ./server PORT-NUMBER FILE-NAME.\n");
        fprintf(stdout, "Using default settings: PORT-NUMBER=4000 FILE-NAME=large.txt\n");
        port=4000;
        file="large.txt";
    }
    else
    {
        port=atoi(argv[1]);
        file=argv[2];
        fprintf(stdout, "Using settings: PORT-NUMBER=%d FILE-NAME=%s\n",port,file);
    }
    reader.readChunk(file);
//    std::cout<<"-----------------------------------------------------------------------------------"<<std::endl;
//    std::cout<<"ANTENTION: Since we have implemented TCP Reno fast retransmission and recovery"<<std::endl;
//    std::cout<<"ANTENTION: SSThresh is not needed and omitted."<<std::endl;
//    std::cout<<"-----------------------------------------------------------------------------------"<<std::endl;
    /* create a socket */
    int fd;
    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        printf("socket created\n");
    
    /* bind it to all local addresses and pick any port number */
    
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
//    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_addr.s_addr = inet_addr("10.0.0.1");
    myaddr.sin_port = htons(port);
    
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        exit(1);
    }

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(SERVICE_PORT);
    
    fd_set readFds;
    fd_set errFds;
    fd_set watchFds;
    FD_ZERO(&readFds);
    FD_ZERO(&errFds);
    FD_ZERO(&watchFds);
    int maxSockfd = fd;
    
    // put the socket in the socket set
    FD_SET(fd, &watchFds);
    
    struct timeval begin, now, tmp, tv;
    double timediff;
    tv.tv_sec=TIMEOUT;
    tv.tv_usec=0;
    
    gettimeofday(&begin , NULL);
    state=WAIT_FOR_SYN;
    Packet packet;
    oBuf.setInitRTO(0.5,0.5,0.12);
    while (true)
    {
        // set up watcher.
        
        int nReadyFds = 0;
        readFds = watchFds;
        errFds = watchFds;
        
        if ((nReadyFds = select(maxSockfd + 1, &readFds, NULL, &errFds, &tv)) == -1)
        {
            perror("Select error!\n");
            exit(3);
        }
        
        if (nReadyFds == 0)
        {
            // timeout happenes.
            
            if(state==WAIT_FOR_SYN) // nothing to do.
            {
                continue;
            }
            else if(state==SYNACK_SENT) //ACK timeout for SYN-ACK.
            {
                gettimeofday(&now , NULL);
                timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
                packet.setAckNumber(initAck);
                packet.setSeqNumber(initSeq);
                packet.setAck(true);
                packet.setFin(false);
                packet.setSyn(true);
                sendmessage=packet.encode();
                if(timediff>SYN_RTO)   ////modified by ye
                {
                    if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                        perror("sendto");
                        exit(1);
                    }
                    oBuf.setcwnd(1024);
                    state=SYNACK_SENT;
                    std::cout<<"Sending packet "<<initSeq<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<<" Retransmission"<<" SYN"<<std::endl; //resent SYN-ACK.  ////modified by ye
//                    packet.printPacket();
                    gettimeofday(&begin , NULL);
                    continue;
                }
                else continue;
            }
            else if(state==FIN_SENT)// ACK timeout for FIN
            {
                gettimeofday(&now , NULL);
                timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
                if(timediff>TIMEOUT)
                {
                    if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                        perror("sendto");
                        exit(1);
                    }
                    state=FIN_SENT;
                    std::cout<<"Sending packet "<<packet.getSeqNumber()<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<<" Retransmission"<<" FIN"<<std::endl;// resent FIN. ////modified by ye
//                    packet.printPacket();
                    gettimeofday(&begin , NULL);
                    continue;
                }
                else continue;
            }
            else if(state==NORMAL)// ACK timeout for data packets.
            {
                gettimeofday(&now , NULL);
                timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
                uint32_t seq=0;
                double ctime=now.tv_sec+1e-6 * now.tv_usec;
                if(oBuf.timeout(ctime,seq))// find timeout data packet.
                {
//                    std::cout<<"ACK timeout!"<<std::endl;
                    seg=oBuf.getSeg(seq);
//                    std::cout<<"ctime: "<<ctime<<std::endl;
//                    seg->printSegment();
//                    if(seg==nullptr) oBuf.printBuffer();
                    packet.setAckNumber(seqnum%MAX_SEQ_NO);
                    packet.setSeqNumber(seg->getSequenceNumber()%MAX_SEQ_NO);
                    packet.setAck(false);
                    packet.setFin(false);
                    packet.setSyn(false);
                    packet.setPayLoad(seg->getPayLoad());
                    gettimeofday(&tmp , NULL);
                    oBuf.setSendTime(seq,tmp.tv_sec+1e-6 * tmp.tv_usec);
                    sendmessage=packet.encode();
                    if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                        perror("sendto");
                        exit(1);
                    }
                    oBuf.setcwnd(1024);
                    std::cout<<"Sending packet "<<packet.getSeqNumber()<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<<" Retransmission"<< std::endl; // resent timeout data packets.
//                    packet.printPacket();
//                    gettimeofday(&begin , NULL);
                    stage=SLOW_START;
                    
                    continue;
                }
                else continue;
            }
            
            
        }
        else // receive packets for client.
        {
            memset(buf, 0, BUFSIZE*sizeof(unsigned char));
            recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *)&remaddr, &slen);
            if (recvlen < 0) {
                continue;
            }
            
            packet.consume(buf);
            if(packet.getSyn()&&state==WAIT_FOR_SYN&&!packet.getAck()&&!packet.getFin()) // if it is the SYN packet.
            {
                std::cout<<"Receiving packet "<<packet.getSeqNumber()<<std::endl;
//                packet.printPacket();
                //for testing
                initSeq = rand()%MAX_SEQ;
//                initSeq = 0; ///////modified by ye
                oBuf.setInitRTO(0.5,0.5,0.12);

                initAck = packet.getSeqNumber()+1;
                packet.setAckNumber(initAck);
                packet.setSeqNumber(initSeq);
                packet.setAck(true);
                packet.setFin(false);
                packet.setSyn(true);
                sendmessage=packet.encode();
                if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                    perror("sendto");
                    exit(1);
                }
                oBuf.setcwnd(1024);
                state=SYNACK_SENT;
                std::cout<<"Sending packet "<<initSeq<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<<" SYN"<<std::endl; // send the SYB-ACK
//                packet.printPacket();
                gettimeofday(&begin , NULL);
            }
            else if(packet.getAck())
            {
//                // handle ACK
//                // ACK to SYN-ACK
                if (state==SYNACK_SENT&&!packet.getFin()&&!packet.getSyn()) // if is is the ACK for SYN-ACK
                {
                    std::cout<<"Receiving packet "<<packet.getAckNumber()<<std::endl;
//                    packet.printPacket();
                    // init output buffer
                    oBuf.setInitSeq(packet.getAckNumber());
                    seqnum=packet.getSeqNumber();
                    if (reader.hasMore())  // insert one packet into buffer
                    {
                        segment.resetDupAck();
                        segment.setIsTrans(false);
                        segment.setIsRetrans(false);
                        segment.setIsAcked(false);
                        segment.setPayLoad(reader.top());
                        segment.setSequenceNumber(oBuf.getInitSeq());
                        segment.setAcknowledgmentNumber(oBuf.getInitSeq()+(reader.top()).size());
                        oBuf.insert(segment);
                        reader.pop();
                        oBuf.SetNextSegSeq(oBuf.getInitSeq());
                    }
                    state=NORMAL;
                    stage=SLOW_START;
                }
                // ACK to FIN
                else if (state==FIN_SENT&&!packet.getSyn()) // if it is the ACK for FIN
                {
                    if(packet.getAckNumber()!=finseq+1) continue;
                    state=WAIT_FOR_SYN; // refresh all the state and waiting for client.
                    stage=SLOW_START;
                    std::cout<<"Receiving packet "<<packet.getAckNumber()<<std::endl;
//                    packet.printPacket();
                    reader.reset();
                    reader.readChunk(file);
//                    std::cout<<"The server is now waiting for new connection ..."<<std::endl;
//                    continue;
                    std::cout<<"The server is closing ..."<<std::endl;
                    close(fd);
                    return 0;
                    
                }
                else if(state==NORMAL){ // ACK to normal packet
                       // update output buffer
                    std::cout<<"Receiving packet "<<packet.getAckNumber()<<std::endl;
//                    packet.printPacket();
                    gettimeofday(&tmp , NULL);
                    int re=oBuf.ack(packet.getAckNumber(),tmp.tv_sec+1e-6 * tmp.tv_usec);
                    if(re==-1)// ACK has some problems
                    {
                        std::cout<<"ACK failed: AckNumber= "<<packet.getAckNumber()<<std::endl;
                    }
                    else if(re==2)// some packets receive 3 duplicate ACKs.
                    {
//                        std::cout<<"Receiving duplicate ACK packet 3 times "<<packet.getAckNumber()<<std::endl;
                        
                        
                        if(stage==SLOW_START)
                        {
                            stage=CONGESTION_AVOIDANCE;
                            cwnd_packet=(oBuf.getcwnd())/1024;
                        }
                        
                        cwnd_packet=cwnd_packet/2;
                        cwnd_packet=ceil(cwnd_packet);
                        oBuf.setcwnd(cwnd_packet*1024); // change the cwnd
                        
                        
                        
                        seg=oBuf.getSeg(packet.getAckNumber());
                        if(seg==nullptr)
                        {
                            std::cout<<"Cannot get packet "<<packet.getAckNumber()<<std::endl;
//                            oBuf.printBuffer();
                        }
                        packet.setSeqNumber(packet.getAckNumber());
                        packet.setAckNumber(seqnum%MAX_SEQ_NO);
                        packet.setAck(false);
                        packet.setFin(false);
                        packet.setSyn(false);
                        packet.setPayLoad(seg->getPayLoad());
                        gettimeofday(&tmp , NULL);
                        oBuf.setSendTime(packet.getSeqNumber(),tmp.tv_sec+1e-6 * tmp.tv_usec);
                        sendmessage=packet.encode();
                        if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                            perror("sendto");
                            exit(1);
                        }
                        std::cout<<"Sending packet "<<packet.getSeqNumber()<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<<" Retransmission"<< std::endl; // resent timeout data
//                        std::cout<<"Send a packet"<<std::endl;
//                        packet.printPacket();

                        
                        
                    }
                    else if(re==0)// ACK is OK. Update cwnd;
                    {
//                        std::cout<<"ACK is OK: AckNumber= "<<packet.getAckNumber()<<std::endl;
//                        oBuf.printBuffer();
                        if(stage==SLOW_START)
                        {
                            oBuf.setcwnd(oBuf.getcwnd()+1024);
                            if(oBuf.getcwnd()>=RCV_WINDOW_SIZE)oBuf.setcwnd(RCV_WINDOW_SIZE);
                        }
                        else
                        {
                            cwnd_packet=cwnd_packet+1/(floor(cwnd_packet));
                            oBuf.setcwnd(cwnd_packet*1024);
                            if(oBuf.getcwnd()>=RCV_WINDOW_SIZE)oBuf.setcwnd(RCV_WINDOW_SIZE);
                        }
                    }
                    while (reader.hasMore() && oBuf.hasSpace(reader.top().size()))// insert new packets to send into buffer.
                    {
                        segment.resetDupAck();
                        segment.setIsTrans(false);
                        segment.setIsRetrans(false);
                        segment.setIsAcked(false);
                        segment.setPayLoad(reader.top());
                        segment.setSequenceNumber(oBuf.getnextInsertSeq());
                        segment.setAcknowledgmentNumber(oBuf.getnextInsertSeq()+(reader.top()).size());
                        oBuf.insert(segment);
                        if(reader.pop()==-1)
                        {
//                            oBuf.printBuffer();
                        };
                    }
//                    oBuf.printBuffer();
                }
                // send packet if necessary
                if (!oBuf.empty())// there existes some packets to send.
                {
                    while (oBuf.hasNext())
                    {
                        seg=oBuf.getSeg(oBuf.getNextSegSeq());
                        if(seg==nullptr)
                        {
                            std::cout<<"buffer cannot get "<<oBuf.getNextSegSeq()<<std::endl;
                        }
                        packet.setAckNumber(seqnum%MAX_SEQ_NO);
                        packet.setSeqNumber(seg->getSequenceNumber()%MAX_SEQ_NO);
                        packet.setAck(false);
                        packet.setFin(false);
                        packet.setSyn(false);
                        packet.setPayLoad(seg->getPayLoad());
                        
                        gettimeofday(&tmp , NULL);
                        oBuf.setSendTime(seg->getSequenceNumber(),tmp.tv_sec+1e-6 * tmp.tv_usec);
                        
                        sendmessage=packet.encode();
                        if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                            perror("sendto");
                            exit(1);
                        }
                        oBuf.SetNextSegSeq(seg->getAcknowledgmentNumber());
                        std::cout<<"Sending packet "<<packet.getSeqNumber()<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<< std::endl; // resent timeout data
//                        packet.printPacket();
                    }
//                    std::cout<<"oBuf does not hasNext"<<"nextsegseq: "<<oBuf.getNextSegSeq()<<std::endl;
                }
                else //nothing remains in the buffer, so send the FIN.
                {
//                    sendFin();
                    packet.setSeqNumber(packet.getAckNumber());
                    packet.setAckNumber(seqnum%MAX_SEQ_NO);
                    
                    packet.setAck(false);
                    packet.setFin(true);
                    packet.setSyn(false);
                    packet.setPayLoad("");
                    
                    sendmessage=packet.encode();
                    if (sendto(fd, sendmessage.c_str(), sendmessage.size(), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                        perror("sendto");
                        exit(1);
                    }
                    state=FIN_SENT;
                    std::cout<<"Sending packet "<<packet.getSeqNumber()<<" "<<oBuf.getcwnd()<<" "<<oBuf.getSSThresh()<<" FIN"<< std::endl;// resent FIN.
//                    packet.printPacket();
                    finseq=packet.getSeqNumber();
                    gettimeofday(&begin , NULL);
                }
                
            }
            
        }
    }
	close(fd);
	return 0;
}
