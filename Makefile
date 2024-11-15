# This makefile puts all the object files in the /o directory
#
# you can also compile manually with this command:
#		g++ -Wall -lSDL2 *.c -o ex $(pkg-config --cflags --libs sdl2)
# 
# 
#
# Dependencies:
# server
# |
# +--> graphics --> world --> ent --> defs
# |
# +--> input ---------------> ent --> defs

CC = gcc
CFLAGS = -g -Wall -Werror -Wpedantic -std=c99
INCLUDES=-Io/includes
LIBS = -lm -lSDL2 -lSDL2_ttf -lSDL2_mixer
OBJECTS = o/vars.o o/server.o o/audio.o o/graphics.o o/input.o o/world.o o/chunk.o o/ent.o o/defs.o o/client.o o/keyEnum.o

ex: ${OBJECTS} Makefile
	cd o/
	${CC} ${CFLAGS} ${OBJECTS} -o ex ${LIBS} ${INCLUDES}

o/keyEnum.o: src/config/keyEnum.c src/config/keyEnum.h
	${CC} ${CFLAGS} -c src/config/keyEnum.c -o o/keyEnum.o ${INCLUDES}
o/vars.o: src/config/vars.h src/config/vars.c o/keyEnum.o
	${CC} ${CFLAGS} -c src/config/vars.c -o o/vars.o ${INCLUDES}
o/client.o: src/client/client.c src/client/client.h src/graphics/animations.h
	${CC} ${CFLAGS} -c src/client/client.c -o o/client.o ${INCLUDES}
o/server.o: src/server/server.c src/server/server.h o/graphics.o o/input.o o/ent.o o/defs.o o/client.o o/audio.o
	${CC} ${CFLAGS} -c src/server/server.c -o o/server.o ${INCLUDES}
o/audio.o: src/audio/audio.c src/audio/audio.h src/audio/sfx.h src/audio/music.h
	${CC} ${CFLAGS} -c src/audio/audio.c -o o/audio.o ${INCLUDES}
src/audio/sfx.h: assets/audio/sfx src/buildScripts/makeSFXEnum.sh
	# sfx dir was modified!
	bash src/buildScripts/makeSFXEnum.sh
src/audio/music.h: assets/audio/music src/buildScripts/makeMusicEnum.sh
	# music dir was modified!
	bash src/buildScripts/makeMusicEnum.sh
o/graphics.o: src/graphics/graphics.c src/graphics/graphics.h src/graphics/animations.h o/world.o o/ent.o o/defs.o
	${CC} ${CFLAGS} -c src/graphics/graphics.c -o o/graphics.o ${INCLUDES}
src/graphics/animations.h: assets/graphics/animations src/buildScripts/makeAnimationEnum.sh
	# animations dir was modified!
	bash src/buildScripts/makeAnimationEnum.sh
o/input.o: src/input/input.c src/input/input.h o/ent.o o/defs.o
	${CC} ${CFLAGS} -c src/input/input.c -o o/input.o ${INCLUDES}
o/world.o: src/world/world.c src/defs.h o/ent.o o/defs.o o/chunk.o
	${CC} ${CFLAGS} -c src/world/world.c -o o/world.o ${INCLUDES}
o/chunk.o: src/world/chunk.c src/defs.h
	${CC} ${CFLAGS} -c src/world/chunk.c -o o/chunk.o ${INCLUDES}
o/ent.o: src/ent/ent.c src/ent/ent.h o/defs.o o/audio.o
	${CC} ${CFLAGS} -c src/ent/ent.c -o o/ent.o ${INCLUDES}
o/defs.o: src/defs/defs.c src/defs.h
	${CC} ${CFLAGS} -c src/defs/defs.c -o o/defs.o ${INCLUDES}

clean:
	rm o/*.o
	rm ex
