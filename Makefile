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

ex: o/server.o o/graphics.o o/input.o o/world.o o/actions.o o/ent.o o/defs.o o/config.o o/client.o
	cd o/
	g++ -Wall -lSDL2 o/server.o o/config.o o/graphics.o o/input.o o/world.o o/chunk.o o/actions.o o/ent.o o/defs.o o/client.o -o ex $(shell pkg-config --cflags --libs sdl2)

o/client.o: src/client/client.cpp src/client.h
	gcc -c src/client/client.cpp -o o/client.o
	
o/server.o: src/server/server.cpp src/server.h o/graphics.o o/input.o o/ent.o o/defs.o o/client.o
	gcc -c src/server/server.cpp -o o/server.o
	
o/config.o: src/config/cfg.cpp src/config.h
	gcc -c src/config/cfg.cpp -o o/config.o
	
o/graphics.o: src/graphics/graphics.cpp src/graphics.h src/animations.h o/world.o o/ent.o o/defs.o
	gcc -c src/graphics/graphics.cpp -o o/graphics.o
	
o/input.o: src/input/input.cpp src/input.h o/ent.o o/defs.o
	gcc -c src/input/input.cpp -o o/input.o
	
o/world.o: src/world/world.cpp src/world.h o/ent.o o/defs.o o/chunk.o
	gcc -c src/world/world.cpp -o o/world.o
	
o/chunk.o: src/world/chunk.cpp src/chunk.h
	gcc -c src/world/chunk.cpp -o o/chunk.o

o/actions.o: src/entities/actions.cpp src/actions.h o/defs.o src/server_constants.h
	gcc -c src/entities/actions.cpp -o o/actions.o
	
o/ent.o: src/entities/ent.cpp src/ent.h o/defs.o
	gcc -c src/entities/ent.cpp -o o/ent.o
	
o/defs.o: src/defs/defs.cpp src/defs.h
	gcc -c src/defs/defs.cpp -o o/defs.o

clean:
	rm o/*.o
	rm ex
