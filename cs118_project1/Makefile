CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=cs118_project1
CLASSES=HttpAbstract.cpp

all: web-server web-client web-server-async

web-server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

web-server-async: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp
	
web-client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-server-async web-client  *.tar.gz

tarball: clean
	tar -zcvf $(USERID).tar.gz *
