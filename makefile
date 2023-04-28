linux/mulc: linux/bin/main.o
	g++ -o linux/mulc linux/bin/main.o

linux/bin/main.o: src/main.cpp
	g++ -c -o linux/bin/main.o -std=c++17 -I ChaiScript/include src/main.cpp