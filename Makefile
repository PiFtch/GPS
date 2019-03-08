# target: debug/client.exe debug/server.exe
# 	echo target
# debug/sever.exe: build/server.o
# 	g++ build/server.o -o debug/server.exe -std=c++11 -lwsock32 -lws2_32
# debug/client.exe: build/client.o
# 	g++ build/client.o -o debug/client.exe 
# build/server.o: src/server.cpp include/common.h
# 	g++ src/server.cpp -o build/server.o -std=c++11 -lwsock32 -lws2_32
# build/client.o:	src/client.cpp src/sgngen.cpp include/common.h
# 	g++ src/client.cpp src/sgngen.cpp -o build/client.o -std=c++11 -lwsock32 -lws2_32
# # debug/client.exe: src/client.cpp src/sgngen.cpp
# 	# g++ src/client.cpp src/sgngen.cpp -o debug/client.exe -std=c++11 -lwsock32 -lws2_32

target: build/client.o build/huffman.o build/sgngen.o build/server.o
	g++ build/client.o build/sgngen.o build/huffman.o -std=c++11 -lwsock32 -lws2_32 -o debug/client.exe
	g++ build/server.o build/sgngen.o -std=c++11 -lwsock32 -lws2_32 -o debug/server.exe

build/client.o: include/common.h src/client.cpp src/sgngen.cpp src/huffman.cpp
	g++ -c src/client.cpp -o build/client.o -std=c++11

build/huffman.o: src/huffman.cpp
	g++ -c src/huffman.cpp -o build/huffman.o -std=c++11

build/sgngen.o: include/common.h src/sgngen.cpp
	g++ -c src/sgngen.cpp -o build/sgngen.o -std=c++11

build/server.o: include/common.h src/server.cpp
	g++ -c src/server.cpp -o build/server.o -std=c++11
# main:
# 	g++ src/server.cpp src/sgngen.cpp -o debug/server.exe -std=c++11 -lwsock32 -lws2_32
# 	g++ src/client.cpp src/sgngen.cpp src/huffman.cpp -o debug/client.exe -std=c++11 -lwsock32 -lws2_32
clean:
	rm -rf build/* debug/* log/server/* log/client/*