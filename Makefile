# really simple Makefile

CXX=g++
CXXFLAGS=-W -Wall -pedantic -std=c++14 -Itlx/

PROGRAMS=move-only-buffer virtual-override-final variadic-templates

all: $(PROGRAMS)

clean:
	rm -f *.o $(PROGRAMS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

move-only-buffer: move-only-buffer.o
	$(CXX) $(CXXFLAGS) -o $@ $^

virtual-override-final: virtual-override-final.o
	$(CXX) $(CXXFLAGS) -o $@ $^

variadic-templates: variadic-templates.o
	$(CXX) $(CXXFLAGS) -o $@ $^

