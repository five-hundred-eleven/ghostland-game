ghostland: glad.o ghostland.o stb_image.o collisions.o
	g++ -o ghostland glad.o ghostland.o stb_image.o collisions.o -ldl -lglfw -std=c++17 -O3

glad.o: glad.c
	g++ -c -o glad.o glad.c -O3

ghostland.o: ghostland.cpp
	g++ -c -o ghostland.o ghostland.cpp -std=c++17 -O3

stb_image.o: stb_image.c
	g++ -c -o stb_image.o stb_image.c -std=c++17 -O3

collisions.o: collisions.cpp
	g++ -c -o collisions.o collisions.cpp -std=c++17 -O3

clean:
	rm -f *.o ghostland
