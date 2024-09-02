# This makefile puts all the object files in the /o directory
#
# you can also compile manually with this command:
#		g++ -Wall -lSDL2 *.cpp -o ex $(pkg-config --cflags --libs sdl2)
# 
# 
#
# Dependencies:
# server
# |
# +--> graphics --> world --> ent --> defs
# |
# +--> input ---------------> ent --> defs

CC = g++
CFLAGS = -g -Wall -Werror -Wpedantic -std=gnu++11
INCLUDES=-Io/includes
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_mixer
OBJECTS = o/vars.o o/server.o o/audio.o o/graphics.o o/input.o o/world.o o/chunk.o o/ent.o o/defs.o o/client.o

ex: ${OBJECTS} Makefile
	cd o/
	${CC} ${CFLAGS} ${OBJECTS} -o ex ${LIBS} ${INCLUDES}

o/vars.o: src/config/vars.h src/config/vars.c
	${CC} ${CFLAGS} -c src/config/vars.c -o o/vars.o ${INCLUDES}
o/client.o: src/client/client.cpp src/client/client.h
	${CC} ${CFLAGS} -c src/client/client.cpp -o o/client.o ${INCLUDES}
o/server.o: src/server/server.cpp src/server/server.h o/graphics.o o/input.o o/ent.o o/defs.o o/client.o
	${CC} ${CFLAGS} -c src/server/server.cpp -o o/server.o ${INCLUDES}
o/audio.o: src/audio/audio.cpp src/audio/audio.h src/audio/sfx.h src/audio/music.h
	${CC} ${CFLAGS} -c src/audio/audio.cpp -o o/audio.o ${INCLUDES}
o/graphics.o: src/graphics/graphics.cpp src/graphics/graphics.h src/graphics/animations.h o/world.o o/ent.o o/defs.o
	${CC} ${CFLAGS} -c src/graphics/graphics.cpp -o o/graphics.o ${INCLUDES}
o/input.o: src/input/input.cpp src/input/input.h o/ent.o o/defs.o
	${CC} ${CFLAGS} -c src/input/input.cpp -o o/input.o ${INCLUDES}
o/world.o: src/world/world.cpp src/defs.h o/ent.o o/defs.o o/chunk.o
	${CC} ${CFLAGS} -c src/world/world.cpp -o o/world.o ${INCLUDES}
o/chunk.o: src/world/chunk.cpp src/defs.h
	${CC} ${CFLAGS} -c src/world/chunk.cpp -o o/chunk.o ${INCLUDES}
o/ent.o: src/ent/ent.cpp src/ent/ent.h o/defs.o
	${CC} ${CFLAGS} -c src/ent/ent.cpp -o o/ent.o ${INCLUDES}
o/defs.o: src/defs/defs.cpp src/defs.h
	${CC} ${CFLAGS} -c src/defs/defs.cpp -o o/defs.o ${INCLUDES}

clean:
	rm o/*.o
	rm ex
