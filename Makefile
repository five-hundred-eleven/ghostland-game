CC := g++
CFLAGS := -std=c++17 -O3
LDFLAGS := -ldl -lglfw

ghostland: glad.o ghostland.o stb_image.o collisions.o player.o shader.o
	$(CC) -o ghostland glad.o ghostland.o stb_image.o collisions.o player.o shader.o $(LDFLAGS) $(CFLAGS)

glad.o: glad.c
	$(CC) -c -o glad.o glad.c $(CFLAGS)

ghostland.o: ghostland.cpp
	$(CC) -c -o ghostland.o ghostland.cpp $(CFLAGS)

stb_image.o: stb_image.c
	$(CC) -c -o stb_image.o stb_image.c $(CFLAGS)

collisions.o: collisions.cpp collisions.h
	$(CC) -c -o collisions.o collisions.cpp $(CFLAGS)

player.o: player.cpp player.h
	$(CC) -c -o player.o player.cpp $(CFLAGS)

shader.o: shader.cpp shader.h
	$(CC) -c -o shader.o shader.cpp $(CFLAGS)

clean:
	rm -f *.o ghostland
