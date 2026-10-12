CPPFLAGS += -I.

all : testConv astBuilder

testConv: testConv.cpp Makefile parsePact.hpp
#	g++ -o test test.cpp --std=c++20 -Wall -ggdb3
	clang++ $(CPPFLAGS) -o testConv testConv.cpp --std=c++20 -Wall -O3 -fconstexpr-steps=100000000

astBuilder: astBuilder.cpp Makefile parsePact.hpp
#	g++ -o test test.cpp --std=c++20 -Wall -ggdb3
	clang++ $(CPPFLAGS) -o astBuilder astBuilder.cpp --std=c++20 -Wall -ggdb3 -fconstexpr-steps=100000000

test: test.cpp Makefile parsePact.hpp parseConstPact.hpp
#	g++ -o test test.cpp --std=c++20 -Wall -ggdb3
	clang++ $(CPPFLAGS) -o test test.cpp --std=c++20 -Wall -ggdb3 -fconstexpr-steps=100000000

.PHONY: check
check: parsePactTest parseConstPactTest
	./parsePactTest
	./parseConstPactTest

parsePactTest: parsePactTest.cpp parsePact.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -o $@ $<

parseConstPactTest: parseConstPactTest.cpp parseConstPact.hpp parsePact.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -o $@ $<
