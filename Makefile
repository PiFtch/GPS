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

target:
	g++ src/server.cpp -o debug/server.exe -std=c++11 -lwsock32 -lws2_32
	g++ src/client.cpp src/sgngen.cpp -o debug/client.exe -std=c++11 -lwsock32 -lws2_32

clean:
	rm build/* debug/*