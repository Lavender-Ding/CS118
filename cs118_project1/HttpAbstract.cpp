#include "HttpAbstract.h"


////////// This is HttpMessage Class ///////////

HttpMessage::HttpMessage()
{
	m_version = "";
	m_headers.clear();
	m_payload.clear();
}
void HttpMessage::setVersion(HttpVersion httpversion)
{
	m_version = httpversion;
}
HttpVersion HttpMessage::getVersion()
{
	return m_version;
}
void HttpMessage::setHeader(std::string key, std::string value)
{
	m_headers[key] = value;
}
std::string HttpMessage::getHeader(std::string key)
{
    std::map<std::string, std::string>::iterator it=m_headers.find(key);
    if(it!=m_headers.end())
	     return m_headers[key];
    else
        return "none";
}
std::string HttpMessage::getAllHeaders()
{
	std::string result = "";
    std::map<std::string, std::string>::iterator iter = m_headers.begin();
	while (iter != m_headers.end())
	{
		result = result + iter->first + ":" + iter->second + "\r\n";
		//cout << iter->first << endl;
		iter++;
	}
	result = result + "\r\n";
	return result;
}
void HttpMessage::decodeHeaderLine(ByteBlob headerLines)
{
	// Code Here !
	int i = 0;
	int j = 0;
	std::string key = "";
	std::string val = "";

	while (i<headerLines.size())
	{

		if (headerLines[i] == ':')
		{
			j = i + 1;
			while (headerLines[j] != '\r')
			{

				val = val + headerLines[j];
				j++;
			}
			m_headers[key] = val;
			key = "";
			val = "";
			if (headerLines[j] == '\r' && headerLines[j + 1] == '\n' && headerLines[j + 2] == '\r' && headerLines[j + 3] == '\n') break;
			i = j + 2;
			continue;
		}

		key = key + headerLines[i];

		i++;
	}
	return;
}
void HttpMessage::setPayLoad(ByteBlob payLoadLines)
{
	// Code Here !
	while (payLoadLines.back() == '\r' || payLoadLines.back() == '\n')
		payLoadLines.pop_back();

	m_payload = payLoadLines;
	return;
}
ByteBlob HttpMessage::getPayload()
{
	return m_payload;
}

////////// This is HttpRequest Class ///////////

HttpRequest::HttpRequest()
{
	m_method = "";
	m_url = "";

}
bool HttpRequest::decodeFirstLine(ByteBlob firstLine)
{
	//Parse [method] [url] [version]
	int i = 0;
	std::string temp = "";
	int whiteSpace = 0;

	while (firstLine[i] != '\r')
	{

		if (firstLine[i] == ' ') // Consider only one whitespace as separator. Any exceptions?
		{
			whiteSpace++;
			if (whiteSpace == 1) { setMethod(temp); temp = ""; i++; continue; }
			if (whiteSpace == 2) { setUrl(temp); temp = ""; i++; continue; }
            if (firstLine[i-1] == ' '||whiteSpace > 2) return false;
		}
		temp = temp + firstLine[i];
		i++;
	}
	setVersion(temp);
    return true;

}
HttpMethod HttpRequest::getMethod()
{
	return m_method;
}
void HttpRequest::setMethod(HttpMethod method)
{
	m_method = method;
}
std::string HttpRequest::getUrl()
{
	return m_url;
}
void HttpRequest::setUrl(std::string url)
{
	m_url = url;
}
ByteBlob HttpRequest::encode()
{
	ByteBlob result;
	std::string encodeString = "";
	encodeString = encodeString + getMethod() + " " + getUrl() + " " + getVersion() + "\r\n";
	encodeString = encodeString + getAllHeaders();

	result.assign(encodeString.begin(), encodeString.end());
	return result;
}
bool HttpRequest::consume(ByteBlob wire)
{
	// decode firstLine, decode headerLine, decode content
	ByteBlob firstLine;
	ByteBlob headerLines;
	ByteBlob payLoadLines;
	int separator = 0;
	int i = 0;
	while (i < wire.size())
	{
		if (i >= 4 && wire[i - 4] == '\r' && wire[i - 3] == '\n' && wire[i - 2] == '\r' && wire[i - 1] == '\n')
		{
			separator = 2;
		}
		else if (i >= 2 && wire[i - 2] == '\r' && wire[i - 1] == '\n')
		{
			separator = 1;
		}
		switch (separator)
		{
		case 0:
			firstLine.push_back(wire[i]);
			break;
		case 1:
			headerLines.push_back(wire[i]);
			break;
		case 2:
			//payLoadLines.push_back(wire[i]);
			break;
		}
		i++;
	}
	if(!decodeFirstLine(firstLine)) return false;
	decodeHeaderLine(headerLines);
	//setPayLoad(payLoadLines);
	return true;
}
////////// This is HttpResponse Class ///////////

HttpResponse::HttpResponse()
{
	m_status = 0;
	m_statusDescription = "";
}

HttpStatus HttpResponse::getStatus()
{
	return m_status;
}
void HttpResponse::setStatus(HttpStatus status)
{
	m_status = status;
}
std::string HttpResponse::getDescription()
{
	return m_statusDescription;
}
void HttpResponse::setDescription(std::string description)
{
	m_statusDescription = description;
}
bool HttpResponse::decodeFirstLine(ByteBlob firstLine)
{
	//Parse [version] [200] [OK]
	int i = 0;
	std::string temp = "";
	int whiteSpace = 0;

	while (firstLine[i] != '\r')
	{

		if (firstLine[i] == ' ') // Consider only one whitespace as separator. Any exceptions?
		{
			whiteSpace++;
			if (whiteSpace == 1) { setVersion(temp); temp = ""; i++;  continue; }
			if (whiteSpace == 2) { setStatus(atoi(temp.c_str())); temp = ""; i++;  continue; }
		}
		temp = temp + firstLine[i];
		i++;
	}
	setDescription(temp);

	return true;
}
ByteBlob HttpResponse::encode()
{
	ByteBlob result;
	std::string encodeString = "";
	encodeString = encodeString + getVersion() + " " + std::to_string(getStatus()) + " " + getDescription() + "\r\n";
	encodeString = encodeString + getAllHeaders();

	result.assign(encodeString.begin(), encodeString.end());
	ByteBlob payLoadLines = getPayload();

	for (int i = 0; i < payLoadLines.size(); i++)
		result.push_back(payLoadLines[i]);

	return result;

}
void HttpResponse::consume(ByteBlob wire)
{
	// decode firstLine, decode headerLine, decode content
	ByteBlob firstLine;
	ByteBlob headerLines;
	ByteBlob payLoadLines;
	int separator = 0;
	int i = 0;
	while (i < wire.size())
	{
            if (i >= 4 && wire[i - 4] == '\r' && wire[i - 3] == '\n' && wire[i - 2] == '\r' && wire[i - 1] == '\n'&&separator!=2)
            {
                separator = 2;
            }
            else if (i >= 2 && wire[i - 2] == '\r' && wire[i - 1] == '\n'&&separator!=1&&separator!=2)
            {
                separator = 1;
            }
        
		
		switch (separator)
		{
		case 0:
			firstLine.push_back(wire[i]);
			break;
		case 1:
			headerLines.push_back(wire[i]);
			break;
		case 2:
			payLoadLines.push_back(wire[i]);
			break;
		}
		i++;
	}
	decodeFirstLine(firstLine);
	decodeHeaderLine(headerLines);
	setPayLoad(payLoadLines);
	return;
}