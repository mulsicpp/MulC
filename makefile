linux/mulc: linux/bin/main.o linux/bin/Flags.o linux/bin/Mulc.o linux/bin/Mulc.runScript.o linux/bin/SystemInterface.o linux/bin/ScopePath.o
	g++ -o linux/mulc linux/bin/main.o linux/bin/Flags.o linux/bin/Mulc.o linux/bin/Mulc.runScript.o linux/bin/SystemInterface.o linux/bin/ScopePath.o

linux/bin/main.o: src/main.cpp
	g++ -c -o linux/bin/main.o -std=c++17 src/main.cpp

linux/bin/Flags.o: src/Flags.cpp
	g++ -c -o linux/bin/Flags.o -std=c++17 src/Flags.cpp

linux/bin/Mulc.o: src/Mulc.cpp
	g++ -c -o linux/bin/Mulc.o -std=c++17 src/Mulc.cpp

linux/bin/Mulc.runScript.o: src/Mulc.runScript.cpp
	g++ -c -o linux/bin/Mulc.runScript.o -std=c++17 -IChaiScript/include src/Mulc.runScript.cpp

linux/bin/SystemInterface.o: src/SystemInterface.cpp
	g++ -c -o linux/bin/SystemInterface.o -std=c++17 src/SystemInterface.cpp

linux/bin/ScopePath.o: src/ScopePath.cpp
	g++ -c -o linux/bin/ScopePath.o -std=c++17 src/ScopePath.cpp