CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
SOURCES = main.cpp commands.cpp database.cpp array.cpp singly.cpp doubly.cpp tree.cpp stack.cpp queue.cpp
HEADERS = commands.h database.h array.h singly.h doubly.h tree.h stack.h queue.h

dbms: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o dbms

clean:
	rm -f dbms
