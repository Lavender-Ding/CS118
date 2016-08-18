#include "TCPAbstract.h"

////////////////////////////////////
//////////// Packet ///////////////
Packet::Packet()
{
    sequenceNumber = 0;
    acknowledgmentNumber = 0;
    window = 0;
    A = false;
    S = false;
    F = false;
    payload = "";
}
Packet::Packet(
               uint16_t seqN,
               uint16_t ackN,
               uint16_t win,
               bool a,
               bool s,
               bool f,
               std::string pl
               )
{
    sequenceNumber=seqN;
    acknowledgmentNumber=ackN;
    window=win;
    A=a;
    S=s;
    F=f;
    payload=pl;
}
// set method...
void Packet::setSeqNumber(uint16_t seq)
{
    sequenceNumber = seq;
}
void Packet::setAckNumber(uint16_t ack)
{
    acknowledgmentNumber = ack;
}
void Packet::setWindow(uint16_t win)
{
    window = win;
}
void Packet::setAck( bool a)
{
    A = a;
}
void Packet::setSyn(bool s)
{
    S = s;
}
void Packet::setFin(bool f)
{
    F = f;
}
void Packet::setPayLoad(std::string pl)
{
    payload = pl;
}

// get method...
uint16_t Packet::getSeqNumber()
{
    return sequenceNumber;
}
uint16_t Packet::getAckNumber()
{
    return acknowledgmentNumber;
}
uint16_t Packet::getWindow()
{
    return window;
}
bool Packet::getAck()
{
    return A;
}
bool Packet::getSyn()
{
    return S;
}
bool Packet::getFin()
{
    return F;
}
std::string Packet::getPayLoad()
{
    return payload;
}
void Packet::printPacket()
{
    std::cout << "sequenceNumber: " << sequenceNumber << std::endl;
    std::cout << "acknowledgmentNumber: " << acknowledgmentNumber << std::endl;
    std::cout << "window: " << window << std::endl;
    std::cout << "A: " << A  << std::endl;
    std::cout << "S: " << S << std::endl;
    std::cout << "F: " << F << std::endl;
    std::cout << "payload: " << payload << std::endl;
}
std::string Packet::encode()
{
    /*
     sequenceNumber = 0;
     acknowledgmentNumber = 0;
     window = 0;
     A = false;
     S = false;
     F = false;
     payload = "";
     */
    std::string result;
    
    int seqHigher = sequenceNumber >> 8;
    int seqLower = sequenceNumber - (seqHigher << 8);
    unsigned char sH = seqHigher;
    unsigned char sL = seqLower;
    
    int ackHigher = acknowledgmentNumber >> 8;
    int ackLower = acknowledgmentNumber - (ackHigher << 8);
    unsigned char aH = ackHigher;
    unsigned char aL = ackLower;
    
    int winHigher = window >> 8;
    int winLower = window - (winHigher << 8);
    unsigned char wH = winHigher;
    unsigned char wL = winLower;
    
    int restHigher = 0;
    int restLower = F + (S << 1) + (A << 2);
    unsigned char rH = restHigher;
    unsigned char rL = restLower;
    
    result.push_back(sH);
    result.push_back(sL);
    result.push_back(aH);
    result.push_back(aL);
    result.push_back(wH);
    result.push_back(wL);
    result.push_back(rH);
    result.push_back(rL);
    result += payload;
    
    /*
     std::cout << "sH:" << seqHigher << " sL:" << seqLower << std::endl;
     std::cout << "aH:" << int(aH) << " aL:" << int(aL) << std::endl;
     std::cout << "wH:" << int(wH) << " wL:" << int(wL) << std::endl;
     std::cout << "rH:" << int(rH) << " rL:" << int(rL) << std::endl;
     */
    return result;
}
void Packet::consume(unsigned char encString[]) //modified by ye, string to unsigned char
{
    /*
     sequenceNumber = 0;
     acknowledgmentNumber = 0;
     window = 0;
     A = false;
     S = false;
     F = false;
     payload = "";
     */
    //	if (encString.length() < 8 || encString.length() > 1032) //modified by ye
    //	{
    //		std::cerr << "packet length error! " << encString.length()<< std::endl;
    //		return;
    //	}
    int seqHigher = (unsigned char)(encString[0]); //modified by ye
    int seqLower = (unsigned char)(encString[1]);
    int ackHigher = (unsigned char)(encString[2]);
    int ackLower = (unsigned char)(encString[3]);
    int winHigher = (unsigned char)(encString[4]);
    int winLower = (unsigned char)(encString[5]);
//    int restHigher = (unsigned char)(encString[6]);
    int restLower = (unsigned char)(encString[7]);
    /*
     std::cout << "sH:" << seqHigher << " sL:" << seqLower << std::endl;
     std::cout << "aH:" << ackHigher << " aL:" << ackLower << std::endl;
     std::cout << "wH:" << winHigher << " wL:" << winLower << std::endl;
     std::cout << "rH:" << restHigher << " rL:" << restLower << std::endl;
     */
    sequenceNumber = (seqHigher << 8) + seqLower;
    acknowledgmentNumber = (ackHigher << 8) + ackLower;
    window = (winHigher << 8) + winLower;
    A = (restLower >> 2);
    S = ((restLower - (A << 2)) >> 1);
    F = restLower - (A << 2) - (S << 1);
    
    payload = std::string(reinterpret_cast<char*>(encString+8));//encString.substr(8, 1032); modified by ye
    
    return;
}
////////////////////////////////////
//////////// Segment ///////////////
Segment::Segment()
{
    duplicateACK = 0;
    isTrans = false;
    isRetrans = false;
    isAcked = false;
    sequenceNumber=0;
    acknowledgmentNumber=0;
    sendTime=0;
}
Segment::Segment(int dACK, bool iT, bool iR, bool iA)
{
	duplicateACK=dACK;
	isTrans=iT;
	isRetrans=iR;
	isAcked=iA;
}

Segment::Segment(int dACK, bool iT, bool iR, bool iA, uint16_t seq, uint16_t ack)
{
    duplicateACK=dACK;
    isTrans=iT;
    isRetrans=iR;
    isAcked=iA;
    sequenceNumber= seq;
    acknowledgmentNumber= ack;
    
}


void Segment::resetDupAck()
{
	duplicateACK = 0;
}
//add by JUN FENG
void Segment::addDupAck()
{
    duplicateACK +=1;
}
//add by JUN FENG

void Segment::increaseDupAck()
{
	duplicateACK++;
}
int Segment::getDupAck()
{
	return duplicateACK;
}
void Segment::setIsTrans(bool isT)
{
	isTrans = isT;
}
void Segment::setIsRetrans(bool isRT)
{
	isRetrans = isRT;
}
void Segment::setIsAcked(bool isA)
{
	isAcked = isA;
}
bool Segment::getIsTrans()
{
	return isTrans;
}
bool Segment::getIsRetrans()
{
	return isRetrans;
}
bool Segment::getIsAcked()
{
	return isAcked;
}
void Segment::setPayLoad(std::string pl)
{
	payload = pl;
}
std::string Segment::getPayLoad()
{
	return payload;
}

void Segment::setSequenceNumber(uint32_t seq)
{
    sequenceNumber=seq;
}
void Segment::setAcknowledgmentNumber(uint32_t ack)
{
    acknowledgmentNumber=ack;
}
void Segment::setSendTime(double stime)
{
    sendTime=stime;
}
uint32_t Segment::getSequenceNumber()
{
    return sequenceNumber;
}
uint32_t Segment::getAcknowledgmentNumber()
{
    return acknowledgmentNumber;
}
double Segment::getSendTime()
{
    return sendTime;
}

void Segment::printSegment()
{
    std::cout<<"--This is segment information."<<std::endl;
    std::cout<<"-sequenceNumber: "<<sequenceNumber<<std::endl;
    std::cout<<"-acknowledgmentNumber: "<<acknowledgmentNumber<<std::endl;
    std::cout<<"-isTrans: "<<isTrans<<std::endl;
    std::cout<<"-isRetrans: "<<isRetrans<<std::endl;
    std::cout<<"-isAcked: "<<isAcked<<std::endl;
    std::cout<<"-duplicateACK: "<<duplicateACK<<std::endl;
    std::cout<<"-SendTime: "<<sendTime<<std::endl;
//    std::cout<<"-payload: "<<payload<<std::endl;

    
}
/*
Packet Segment::getPacket()
{
	return p;
}
void Segment::buildPacket(
						uint16_t sequenceNumber,
						uint16_t acknowledgmentNumber,
						uint16_t window,
						bool A,
						bool S,
						bool F,
						std::string payLoad
						)
{
	p.setSeqNumber(sequenceNumber);
	p.setAckNumber(acknowledgmentNumber);
	p.setWindow(window);
	p.setAck(A);
	p.setSyn(S);
	p.setFin(F);
	p.setPayLoad(payLoad);
}
*/
/////////////////////////////////////
//////////// fileName ///////////////
FileReader::FileReader()
{
    chunk_size = 1024;
    lastChunkSize = 0;
    fileName = "";
    file.clear(); file.close();
    topString="";
    chunkNum = 0;
    chunkCursor = 0;
}
void FileReader::setChunkSize(int chunkSize)
{
    chunk_size = chunkSize;
}
int FileReader::getChunkSize()
{
    return chunk_size;
}
int FileReader::getLastChunkSize()
{
    return lastChunkSize;
}
int FileReader::getChunkNum()
{
    return chunkNum;
}
int FileReader::getChunkCursor()
{
    return chunkCursor;
}
void FileReader::readChunk(std::string FILENAME)
{
    fileName = FILENAME;
    
    file.open(fileName);
    
    chunk_size = 1024;
    lastChunkSize = 0;
    topString = "";
    chunkNum = 0;
    chunkCursor = 0;
    
    if (!file)
    {
        std::cerr << "ERROR: CAN NOT FIND FILE:" << fileName << std::endl;
        file.clear();
        return ;
    }
    
    struct stat filestatus;
    stat(fileName.c_str(), &filestatus);
    
    int dataLength = filestatus.st_size;
    
    //std::cout << "total_size:" << total_size << std::endl;
    //std::cout << "chunk_size:" << chunk_size << std::endl;
    
    chunkNum = dataLength / chunk_size;// initialize chunkNum in class FileReader
    lastChunkSize = dataLength % chunk_size;
    
    if (lastChunkSize != 0) /* if the above division was uneven */
    {
        ++chunkNum; /* add an unfilled final chunk */
    }
    else /* if division was even, last chunk is full */
    {
        lastChunkSize = chunk_size;
    }
    
    int this_chunk_size = (chunkNum == 1 ? lastChunkSize : chunk_size);
    
    std::string chunk_data(this_chunk_size, ' ');
    file.read(&chunk_data[0], this_chunk_size);
    
    topString = chunk_data;// initialize topString in class FileReader
    
    return ;
}
void FileReader::reset()
{
    chunk_size = 1024;
    lastChunkSize = 0;
    fileName = "";
    file.clear(); file.close();
    topString = "";
    chunkNum = 0;
    chunkCursor = 0;
}
std::string FileReader::top()
{
    return topString;
}
int FileReader::pop()
{
    chunkCursor++;
    
    if (chunkCursor >= chunkNum)
    {
//        std::cerr << "nothing to pop!" << std::endl;
        return -1;
    }
    
    int this_chunk_size = ((chunkCursor == chunkNum - 1) ? lastChunkSize : chunk_size);
    
    std::string chunk_data(this_chunk_size, ' ');
    file.read(&chunk_data[0], this_chunk_size);
    
    
    topString = chunk_data;
    return 0;
}
bool FileReader::hasMore()
{
    return (chunkCursor <= chunkNum - 1);
}

/////////////////////////////////////////
//////////// ServerBuffer ///////////////
ServerBuffer::ServerBuffer()
{
    nextSegSeq=0;
    initSeq=0;
    cwnd=1024;
    SSThresh=1024;
}

void ServerBuffer::setInitSeq(uint32_t seqNo)
{
    initSeq=seqNo;
}

uint32_t ServerBuffer::getInitSeq()
{
    return initSeq;
}

int ServerBuffer::ack(uint32_t ackNo,double rtime)
{
    bool normal=false;
    std::vector<Segment>::iterator it;
//    std::cout<<"Enter in  ServerBuffer::ack "<<std::endl;
    for ( it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if(((*it).getAcknowledgmentNumber())%MAX_SEQ==ackNo)
        {
            (*it).setIsAcked(true);
            //RTT estimation and adaptive RTO
            SampleRTT=rtime-(*it).getSendTime();
            difference=SampleRTT-SRTT;
            SRTT=SRTT+difference/8;
            DevRTT=DevRTT+(fabs(difference)-DevRTT)/4;
            RTO=SRTT+4*DevRTT;
            //RTT estimation and adaptive RTO
            normal=true;
            break;
        }
    }
//    std::cout<<"Enter in  ServerBuffer::ack stage 1 "<<std::endl;
    if(!normal)
    {
//        std::cout<<"Enter in  ServerBuffer::ack stage 2 "<<std::endl;
        for (it = buffer.begin() ; it != buffer.end(); ++it)
        {
            if(((*it).getSequenceNumber())%MAX_SEQ==ackNo)
            {
                (*it).addDupAck();
//                std::cout<<"DupAck with ACK= "<<ackNo<<std::endl;
                if(((*it).getDupAck())>=3)
                {
                    (*it).resetDupAck();
                    return 2;
                }
                return 1;
            }
        }
        return -1;
        
    }
    else
    {
//        std::cout<<"Enter in  ServerBuffer::ack stage 3 "<<std::endl;
        std::vector<Segment>::iterator i = buffer.begin();
        while(i!=it)
            {
                (*i).setIsAcked(true);
                i++;
            }
//        std::cout<<"Enter in  ServerBuffer::ack stage 4 "<<std::endl;
        while ((buffer.front()).getIsAcked()==true) {
            if(buffer.size()>=2) buffer.erase(buffer.begin());
            else
            {
                buffer.clear();
                break;
            }
        }
        return 0;
    }
    return 0;
}


bool ServerBuffer::timeout(double ctime, uint32_t &seq)
{
    //
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if((ctime-(*it).getSendTime()>=RTO)&&(!(*it).getIsAcked()))
        {
            seq=(*it).getSequenceNumber();
            return true;
        }
    }
    return false;
}


bool ServerBuffer::hasSpace(uint16_t size)
{
    if(buffer.size()>0)
    {
        return ((buffer.back()).getAcknowledgmentNumber()-(buffer.front()).getSequenceNumber()+size<=cwnd);
    }
    else
    {
        return size<=cwnd;
    }
}

void ServerBuffer::insert(Segment seg)
{
    buffer.push_back (seg);
    nextInsertSeq=seg.getAcknowledgmentNumber();
}

uint32_t ServerBuffer::getNextSegSeq()
{
    return nextSegSeq;
}
Segment *ServerBuffer::getSeg(uint32_t seq)
{
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if((*it).getSequenceNumber()==seq)
        {
            return &(*it);
        }
    }
//    std::cout<<"getSeg() failed: "<<"seq=" <<seq<<std::endl;
    return nullptr;
}

Segment *ServerBuffer::getSeg(uint16_t seq)
{
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if((*it).getSequenceNumber()%MAX_SEQ==seq)
        {
            return &(*it);
        }
    }
//    std::cout<<"getSeg() failed: "<<"seq=" <<seq<<std::endl;
    return nullptr;
}

bool ServerBuffer::hasNext()
{
    return nextSegSeq<=(buffer.back()).getSequenceNumber();
}

bool ServerBuffer::empty()
{
    return buffer.size()==0;
}

void ServerBuffer::SetNextSegSeq(uint32_t seqNo)
{
//    std::cout<<"SetNextSegSeq: "<<seqNo<<std::endl;
    nextSegSeq=seqNo;
}

void ServerBuffer::printSegment(uint32_t seqNo)
{
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if(((*it).getSequenceNumber())==seqNo)
        {
            (*it).printSegment();
            break;
        }
    }
}

void ServerBuffer::printBuffer()
{
    std::cout<<"This is Buffer information."<<std::endl;
    std::cout<<"nextSegSeq: "<<nextSegSeq<<std::endl;
    std::cout<<"initSeq: "<<initSeq<<std::endl;
    std::cout<<"cwnd: "<<cwnd<<std::endl;
    
    
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        (*it).printSegment();
    }
}
uint16_t ServerBuffer::getcwnd()
{
    return cwnd;
}
uint16_t ServerBuffer::getSSThresh()
{
    return SSThresh;
}
void ServerBuffer::setcwnd(uint16_t c)
{
    cwnd=c;
    SSThresh=c/2;
    if(SSThresh<1024)SSThresh=1024;
}

uint32_t ServerBuffer::getnextInsertSeq()
{
    return nextInsertSeq;
}
void ServerBuffer::setSendTime(uint32_t seqNo,double ctime)
{
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if(((*it).getSequenceNumber())==seqNo)
        {
            (*it).setSendTime(ctime);
            break;
        }
    }
}

void ServerBuffer::setSendTime(uint16_t seqNo,double ctime)
{
    for (std::vector<Segment>::iterator it = buffer.begin() ; it != buffer.end(); ++it)
    {
        if(((*it).getSequenceNumber())%MAX_SEQ==seqNo)
        {
            (*it).setSendTime(ctime);
            break;
        }
    }
}

void ServerBuffer::setInitRTO(double R, double SR, double DR)
{
    RTO=R;
    SRTT=SR;
    DevRTT=DR;
}
/********************** client class implementation ********************/
recvSegment::recvSegment(std::string data,uint16_t seqNo, uint16_t ackNo){
    m_data=data;
    sequenceNumber=seqNo;
    acknowledgmentNumber=ackNo;
    
}
std::string recvSegment::getData(){
    return m_data;
}
uint16_t recvSegment::getSeqNumber(){
    return sequenceNumber;
}
uint16_t recvSegment::getAckNumber(){
    return acknowledgmentNumber;
}


recvBuffer::recvBuffer(uint16_t recvWindow,uint16_t maxSeq){
    ack_no=0;
    recv_window_size=recvWindow;
    max_seq=maxSeq;
    is_retransmit=false;
}

void recvBuffer::setInitAck(uint16_t initSeqNo, uint16_t dataSize){
    ack_no=initSeqNo+dataSize;
}
uint16_t recvBuffer::getSize(){
    uint16_t size=0;
    for(unsigned int i=0;i<recv_buffer.size();i++){
        size+=recv_buffer[i].getData().length();
    }
    return size;
}
uint16_t  recvBuffer::remainWindowSize(uint16_t seqNo){
    if(recv_buffer.empty())
        return recv_window_size;
    uint16_t min=ack_no;
    uint16_t max=(ack_no+recv_window_size)%max_seq;
    
    uint16_t last=recv_buffer[recv_buffer.size()-1].getAckNumber();
    if(min<=max){
        return max-last;
    }
    else{
        if(last>min)
            return max_seq-last+max;
        else
            return max-last;
    }
}

uint16_t recvBuffer::insert(uint16_t seqNo, std::string data){
    //    std::cout<<"///////////insert////////////"<<std::endl;
    is_retransmit=false;
    bool insert=false;
    uint16_t min=ack_no;
    uint16_t max=(ack_no+recv_window_size)%max_seq;
    
    if(recv_buffer.empty()){
        uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
        //std::cout<<"seg ack no"<<seg_ack_no<<" "<<seqNo<<" "<<data.length()<<std::endl;
        //        std::cout<<"insert empty"<<std::endl;
        recvSegment recvseg(data,seqNo,seg_ack_no);
        recv_buffer.push_back(recvseg);
        if(ack_no==seqNo)
            ack_no=seg_ack_no;
        return ack_no;
    }
    
    std::vector<recvSegment>::iterator it,it_next;
    for(it = recv_buffer.begin();it < recv_buffer.end();it++){
        if(seqNo==it->getSeqNumber()){ //if there is already a packet with the same seqNo, throw it, set is_retransmit true
            is_retransmit=true;
            //            std::cout<<"retransmit: file already in buffer "<<std::endl;
            return ack_no;
            //return it->getAckNumber();
        }
        
        if(ack_no==it->getSeqNumber()) //advance the ackNo to the next unacked segment
            ack_no=it->getAckNumber();
        
        if(min<max)
        {
            if(it->getSeqNumber()>seqNo&&!insert){
                uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
                recvSegment recvseg(data,seqNo,seg_ack_no);
                it=recv_buffer.insert(it,recvseg); //add at the begin of buffer
                //                std::cout<<"insert at the begin"<<std::endl;
                if(ack_no==seqNo)
                    ack_no=seg_ack_no;
                insert=true;
            }
            
            if(it->getAckNumber()<=seqNo&&it==recv_buffer.end()-1&&!insert){
                uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
                recvSegment recvseg(data,seqNo,seg_ack_no);
                recv_buffer.push_back(recvseg); //add to the back of buffer
                //                std::cout<<"insert at the end"<<std::endl;
                if(ack_no==seqNo)
                    ack_no=seg_ack_no;
                insert=true;
                break;
            }
            if(it==recv_buffer.end()-1)
                break;
            it_next=it+1;
            if(it->getAckNumber()<=seqNo&&it_next->getSeqNumber()>seqNo&&!insert){
                uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
                recvSegment recvseg(data,seqNo,seg_ack_no);
                it=recv_buffer.insert(it_next,recvseg); //add in between the buffer, reset it since it's no longer valid
                //                std::cout<<"insert in between"<<std::endl;
                if(ack_no==seqNo)
                    ack_no=seg_ack_no;
                insert=true;
            }
        }else{ //max<min
            if(((it->getSeqNumber()>seqNo&&((it->getSeqNumber()>=min&&seqNo>=min)||(it->getSeqNumber()<max&&seqNo<max)))||(it->getSeqNumber()<max&&seqNo>=min))&&!insert){
                uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
                recvSegment recvseg(data,seqNo,seg_ack_no);
                it=recv_buffer.insert(it,recvseg); //add at the begin of buffer
                //                std::cout<<"insert at the begin"<<std::endl;
                if(ack_no==seqNo)
                    ack_no=seg_ack_no;
                insert=true;
            }
            
            if(((it->getSeqNumber()<seqNo&&((it->getSeqNumber()>=min&&seqNo>=min)||(it->getSeqNumber()<max&&seqNo<max)))||(it->getSeqNumber()>=min&&seqNo<max))&&it==recv_buffer.end()-1&&!insert){
                uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
                recvSegment recvseg(data,seqNo,seg_ack_no);
                recv_buffer.push_back(recvseg); //add to the back of buffer
                //                std::cout<<"insert at the end"<<std::endl;
                if(ack_no==seqNo)
                    ack_no=seg_ack_no;
                insert=true;
                break;
            }
            if(it==recv_buffer.end()-1)
                break;
            it_next=it+1;
            
            bool inbetween1=(it->getSeqNumber()<seqNo&&it_next->getSeqNumber()>seqNo)&&((it->getSeqNumber()<max&&seqNo<max&&it_next->getSeqNumber()<max)||(it->getSeqNumber()>=min&&seqNo>min&&it_next->getSeqNumber()>=min));  //it, seqNo, it_next all <max or >min
            bool inbetween2=it->getSeqNumber()<seqNo&&(it->getSeqNumber()>=min&&seqNo>=min)&&it_next->getSeqNumber()<max;  //it, seqNo >min it_next <max
            bool inbetween3=it_next->getSeqNumber()>seqNo&&it->getSeqNumber()>=min&&(seqNo<max&&it_next->getSeqNumber()<max);   //it >min, seqNo,it_next <max
            
            if((inbetween1||inbetween2||inbetween3)&&!insert){
                uint16_t seg_ack_no=seqNo+data.length()>max_seq?seqNo+data.length()-max_seq:seqNo+data.length();
                recvSegment recvseg(data,seqNo,seg_ack_no);
                it=recv_buffer.insert(it_next,recvseg); //add in between the buffer, reset it since it's no longer valid
                //                std::cout<<"insert in between"<<(it-1)->getSeqNumber()<<" "<<it->getSeqNumber()<<std::endl;
                if(ack_no==seqNo)
                    ack_no=seg_ack_no;
                insert=true;
            }
            
        }
    }
    
    return ack_no;
}
void recvBuffer::initFile(){
    output.open("received.data", std::ofstream::out|std::ofstream::trunc);
    output.close();
}
void recvBuffer::writeToFile(){
    //    std::cout<<"/////////wirte to file///////////"<<std::endl;
    std::vector<recvSegment>::iterator it,it_next;
    
    if(!recv_buffer.empty()){ //when ackno is before the seqno of first segment, break
        uint16_t begin=recv_buffer.begin()->getSeqNumber();
        uint16_t end=(recv_buffer.end()-1)->getAckNumber();
        
        if(end<begin){
            if((ack_no>=max_seq-(recv_window_size-end))&&ack_no<begin)//when seqNo wrap around, we need to consider the case ackno< first seqNo
                return;
        }
        else{
            if((ack_no>=max_seq-(recv_window_size-end))||ack_no<begin)// we need to consider the case ack>first seqNo
                return;
        }
        
    }
    for(it = recv_buffer.begin();it < recv_buffer.end();){
        
        
        //        std::cout<<"write to file "<<it->getSeqNumber()<<std::endl;
        output.open("received.data", std::ofstream::out|std::ofstream::app);
        output<<it->getData();
        output.close();
        if(it->getAckNumber()==ack_no){ //when encounter the last segment before ackno, break
            
            recv_buffer.erase(it);
            break;
        }
        it=recv_buffer.erase(it);
    }
}

uint16_t recvBuffer::getACK(){
    return ack_no;
}
bool recvBuffer::isRetransmit(){
    return is_retransmit;
}
bool recvBuffer::isOutOfBuffer(uint16_t seqNo){
    is_retransmit=false;
    uint16_t min=ack_no;
    uint16_t max=(ack_no+recv_window_size)%max_seq;
    if(min<=max){
        if(seqNo<min||seqNo>=max){
            //            std::cout<<"retransmit: file not in buffer "<<max<<std::endl;
            is_retransmit=true;
            return true;
        }
    }
    else{
        if(seqNo>=max&&seqNo<min){
            //            std::cout<<"retransmit: file not in buffer "<<max<<std::endl;
            is_retransmit=true;
            return true;
        }
    }
    return false;
    
}
void recvBuffer::print(){
    std::cout<<"////////////print////////////"<<std::endl;
    for(unsigned int i=0;i<recv_buffer.size();i++)
        std::cout<<recv_buffer[i].getSeqNumber()<<std::endl;
}
/*****************************************************************************/


