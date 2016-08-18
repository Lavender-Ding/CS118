#ifndef __TCPABSTRACT_H_INCLUDED__   // if x.h hasn't been included yet...
#define __TCPABSTRACT_H_INCLUDED__   //   #define this so the compiler knows it has been included

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <queue>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <thread>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>

#define SERVICE_PORT	4000	/* hard-coded port number */
#define TIMEOUT 6
#define BUFLEN 1032
#define BUFSIZE 1032
#define CLIENT_PORT 4000
#define SERVER_PORT 4000
#define RCV_WINDOW_SIZE 15360
#define MAX_SEQ_NO 30720
#define MAX_SEQ 30720
#define SYN_RTO 0.5

class Packet
{
private:
	uint16_t sequenceNumber;
	uint16_t acknowledgmentNumber;
	uint16_t window;
	bool A;
	bool S;
	bool F;
	std::string payload; //1024 B data.
public:
	Packet();
	Packet(uint16_t seqN,uint16_t ackN,uint16_t win,bool a,bool s,bool f,std::string pl);

	void setSeqNumber(uint16_t seq);
	void setAckNumber(uint16_t ack);
	void setWindow(uint16_t win);
	void setAck(bool A);
	void setSyn(bool S);
	void setFin(bool F);
	void setPayLoad(std::string pl);

	uint16_t getSeqNumber();
	uint16_t getAckNumber();
	uint16_t getWindow();
	bool getAck();
	bool getSyn();
	bool getFin();
	std::string getPayLoad();

	void printPacket();

	std::string encode();
//	void consume(std::string encString);
    void consume(unsigned char encString[]);
};


class Segment
{
private:
	int duplicateACK;
	bool isTrans;// true if this packet is in transimission but not yet acked.
	bool isRetrans;// true if this packet is in re-transmission 
	bool isAcked;// true if this packet is acked.
	std::string payload;// payload data
	//Packet p; // Segment also stores packet
    
    // add by JUN FENG
    uint32_t sequenceNumber;
    uint32_t acknowledgmentNumber;
    double sendTime;
    // add by JUN FENG
    
public:
	Segment();
	Segment(int duplicateACK,bool isTrans, bool isRetrans, bool isAcked);
    Segment(int duplicateACK,bool isTrans, bool isRetrans, bool isAcked, uint16_t seq, uint16_t ack);
	void resetDupAck();
    //add by JUN FENG
    void addDupAck();
    //add by JUN FENG
	void increaseDupAck();
	void setIsTrans(bool isT);
	void setIsRetrans(bool isRT);
	void setIsAcked(bool isA);
	void setPayLoad(std::string pl);

	bool getIsTrans();
	bool getIsRetrans();
	bool getIsAcked();
	int getDupAck();
	std::string getPayLoad();
    //add by JUN FENG
    void setSequenceNumber(uint32_t seq);
    void setAcknowledgmentNumber(uint32_t ack);
    void setSendTime(double stime);
    uint32_t getSequenceNumber();
    uint32_t getAcknowledgmentNumber();
    double getSendTime();
    //add by JUN FENG
    
	//Packet getPacket();
	//void buildPacket(uint16_t sequenceNumber,uint16_t acknowledgmentNumber,uint16_t window,bool A,bool S,bool F,std::string payLoad					);
	void printSegment();
};

class FileReader
{
private:
    
    int chunk_size;// initialize to 1024 B
    int lastChunkSize;
    std::string fileName;
    // The following parameters will be set after readChunk(string FILENAME) function
    std::ifstream file;
    std::string topString;
    int chunkNum;
    int chunkCursor;
public:
    FileReader();
    void setChunkSize(int chunkSize);
    int getChunkSize();
    int getLastChunkSize();
    int getChunkNum();
    int getChunkCursor();
    
    // read in the data from FILENAME and set data length and topString
    void readChunk(std::string FILENAME);
    // reset all the private
    void reset();
    
    std::string top();
    int pop();
    bool hasMore();
};

class ServerBuffer
{
private:
    std::vector<Segment> buffer;
    uint32_t nextSegSeq;
    uint32_t initSeq;
    uint16_t cwnd;
    uint16_t SSThresh;
    uint32_t nextInsertSeq;
    double RTO=0.6;
    double SRTT=0.5;
    double SampleRTT=0;
    double DevRTT=0.12;
    double difference=0;
public:
    ServerBuffer();
    void setInitSeq(uint32_t seqNo);
    uint32_t getInitSeq();
    int ack(uint32_t ackNo,double rtime);
    bool timeout(double ctime, uint32_t &seq);
    bool hasSpace(uint16_t size = 1024);
    void insert(Segment seg);
    uint32_t getNextSegSeq();// do not use it
    Segment *getSeg(uint32_t);
    Segment *getSeg(uint16_t);
    bool hasNext();// return buffersize<congestion window
    bool empty();// return size==0
    void SetNextSegSeq(uint32_t seqNo);// do not use it
    void printSegment(uint32_t seqNo);
    void setSendTime(uint32_t seqNo,double ctime);
    void setSendTime(uint16_t seqNo,double ctime);
    void printBuffer();
    uint16_t getcwnd();
    uint16_t getSSThresh();
    void setcwnd(uint16_t c);
    uint32_t getnextInsertSeq();
    void setInitRTO(double R, double SR, double DR);
};

/********************  client class ***********************/
class recvSegment
{
private:
    std::string m_data;
    uint16_t sequenceNumber;
    uint16_t acknowledgmentNumber;
public:
    std::string getData();
    uint16_t getSeqNumber();
    uint16_t getAckNumber();
    recvSegment(std::string data,uint16_t seqNo,uint16_t ackNo);
};

class recvBuffer{
public:
    recvBuffer(uint16_t recvWindow,uint16_t maxSeq);
    void setInitAck(uint16_t initSeqNo, uint16_t dataSize);
    uint16_t insert(uint16_t seqNo, std::string data);
    void initFile();
    void writeToFile();
    uint16_t getACK();
    uint16_t remainWindowSize(uint16_t seqNo);
    bool isRetransmit();
    bool isOutOfBuffer(uint16_t seqNo);
    void print();
    
private:
    uint16_t ack_no;
    uint16_t recv_window_size;
    uint16_t max_seq;
    std::vector<recvSegment> recv_buffer;
    std::ofstream output;
    uint16_t getSize();
    bool is_retransmit;
};
/*************************************************************/

#endif