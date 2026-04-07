CXXFLAGS=-g -O2 -MMD -std=c++17
code: code.o lang.o
	$(CXX) -o $@ $^

code.o: code.cpp
lang.o: lang.cpp

clean:
	rm -f code code.o lang.o *.d

-include *.d
