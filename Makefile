all: obj/ build/ obj/main.c.o obj/types.c.o obj/physics.c.o build/physics 
OSMODE := l

obj/: 
ifeq (${OSMODE}, l)
	mkdir -p obj
else
	mkdir obj
endif

build/: 
ifeq (${OSMODE}, l)
	mkdir -p build
else
	mkdir build
endif

obj/main.c.o: src/main.c 
ifeq (${OSMODE}, l)
	gcc src/main.c -c -o obj/main.c.o -Iinclude/
else
	gcc src/main.c -c -o obj/main.c.o -Iinclude/
endif


obj/types.c.o: src/types.c 
ifeq (${OSMODE}, l)
	gcc src/types.c -c -o obj/types.c.o -Iinclude/
else
	gcc src/types.c -c -o obj/types.c.o -Iinclude/
endif


obj/physics.c.o: src/physics.c 
ifeq (${OSMODE}, l)
	gcc src/physics.c -c -o obj/physics.c.o -Iinclude/
else
	gcc src/physics.c -c -o obj/physics.c.o -Iinclude/
endif


build/physics: obj/main.c.o obj/types.c.o obj/physics.c.o 
ifeq (${OSMODE}, l)
	${CXX} obj/main.c.o obj/types.c.o obj/physics.c.o -o build/physics -lSDL3
else
	${CXX} obj/main.c.o obj/types.c.o obj/physics.c.o -o build/physics -lSDL3
endif


clean:
	rm -r obj
	rm -r build
.PHONY: clean


gencommands:
	mkdir emmgtemp
ifeq (${OSMODE}, l)
	clang src/main.c  -Iinclude/ -MJ emmgtemp/0.json -fsyntax-only
else
	clang src/main.c  -Iinclude/ -MJ emmgtemp/0.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/types.c  -Iinclude/ -MJ emmgtemp/1.json -fsyntax-only
else
	clang src/types.c  -Iinclude/ -MJ emmgtemp/1.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/physics.c  -Iinclude/ -MJ emmgtemp/2.json -fsyntax-only
else
	clang src/physics.c  -Iinclude/ -MJ emmgtemp/2.json -fsyntax-only
endif
# not cross platform here sad i think
	echo [ > emmgtemp/[
	echo ] > emmgtemp/]
	cat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json
	rm -r emmgtemp
