snowland: glad.o snowland.o stb_image.o
	g++ -o snowland glad.o snowland.o stb_image.o -ldl -lglfw -std=c++17

glad.o: glad.c
	g++ -c -o glad.o glad.c

snowland.o: snowland.cpp
	g++ -c -o snowland.o snowland.cpp -std=c++17

stb_image.o: stb_image.c
	g++ -c -o stb_image.o stb_image.c -std=c++17

clean:
	rm -f *.o snowland
