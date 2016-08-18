CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -std=c++11 $(CXXOPTIMIZE)
USERID=cs118_project2
CLASSES=TCPAbstract.cpp

all: clean client server

server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz

tarball: clean
	tar -zcvf $(USERID).tar.gz client.cpp server.cpp TCPAbstract.cpp TCPAbstract.h Makefile Vagrantfile
