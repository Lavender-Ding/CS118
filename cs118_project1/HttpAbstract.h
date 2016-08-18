#ifndef HttpAbstract_h
#define HttpAbstract_h
/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
*/
#include <iostream>
#include <vector>
#include <string>
#include <map>

//using namespace std;

typedef std::vector<char> ByteBlob;
// temporary see ByteBlob as string
typedef std::string HttpVersion;
typedef std::string HttpMethod;
typedef int HttpStatus;



////////// This is HttpMessage Class ///////////

class HttpMessage
{
private:

	HttpVersion m_version;
	std::map<std::string, std::string> m_headers;
	ByteBlob m_payload;
public:
	HttpMessage();
	virtual bool decodeFirstLine(ByteBlob firstLine) = 0;// firstLine is a vector WITH \r \n
	HttpVersion getVersion();// return current httpversion
	void setVersion(HttpVersion httpversion);// set current httpversion
	void setHeader(std::string key, std::string value);// set m_headers
	std::string getHeader(std::string key);//get value of key from m_headers
	std::string getAllHeaders();
	void decodeHeaderLine(ByteBlob headerLines);//headerLines include \r\n\r\n
	void setPayLoad(ByteBlob payLoadLines);// set m_payload with out previous \r\n
	ByteBlob getPayload();// get m_payload
};



////////// This is HttpRequest Class ///////////

class HttpRequest : public HttpMessage
{
private:

	HttpMethod m_method;
	std::string m_url;

public:
	HttpRequest();
	virtual bool decodeFirstLine(ByteBlob firstLine); // firstLine includes "\r\n"
	HttpMethod getMethod(); //get m_method
	void setMethod(HttpMethod method);//set m_method
	std::string getUrl();// get m_url
	void setUrl(std::string url);//set m_url
	ByteBlob encode();
	bool consume(ByteBlob wire);
};




////////// This is HttpResponse Class ///////////

class HttpResponse : public HttpMessage
{
private:
	HttpStatus m_status;
	std::string m_statusDescription;
public:
	HttpResponse();
	virtual bool decodeFirstLine(ByteBlob firstLine); // firstLine includes \r\n
	HttpStatus getStatus();//get m_status
	void setStatus(HttpStatus status);//set m_status
	std::string getDescription();//get m_statusDescription
	void setDescription(std::string description);//set m_statusDescription
	ByteBlob encode();
	void consume(ByteBlob wire);
};
#endif