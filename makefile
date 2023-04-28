linux/mulc: linux/bin/main.o linux/bin/Flags.o linux/bin/Mulc.o
	g++ -o linux/mulc linux/bin/main.o linux/bin/Flags.o linux/bin/Mulc.o

linux/bin/main.o: src/main.cpp
	g++ -c -o linux/bin/main.o -std=c++17 src/main.cpp

linux/bin/Flags.o: src/Flags.cpp
	g++ -c -o linux/bin/Flags.o -std=c++17 src/Flags.cpp

linux/bin/Mulc.o: src/Mulc.cpp
	g++ -c -o linux/bin/Mulc.o -std=c++17 src/Mulc.cpp