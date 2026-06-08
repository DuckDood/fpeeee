all: obj/ build/ obj/main.c.o obj/types.c.o obj/physics.c.o obj/matrix.c.o obj/shape_generators.c.o build/physics 
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
	${CC} src/main.c -c -o obj/main.c.o -Iinclude/ -O2 -march=native
else
	${CC} src/main.c -c -o obj/main.c.o -Iinclude/ -O2 -march=native
endif


obj/types.c.o: src/types.c 
ifeq (${OSMODE}, l)
	${CC} src/types.c -c -o obj/types.c.o -Iinclude/ -O2 -march=native
else
	${CC} src/types.c -c -o obj/types.c.o -Iinclude/ -O2 -march=native
endif


obj/physics.c.o: src/physics.c 
ifeq (${OSMODE}, l)
	${CC} src/physics.c -c -o obj/physics.c.o -Iinclude/ -O2 -march=native
else
	${CC} src/physics.c -c -o obj/physics.c.o -Iinclude/ -O2 -march=native
endif


obj/matrix.c.o: src/matrix.c 
ifeq (${OSMODE}, l)
	${CC} src/matrix.c -c -o obj/matrix.c.o -Iinclude/ -O2 -march=native
else
	${CC} src/matrix.c -c -o obj/matrix.c.o -Iinclude/ -O2 -march=native
endif


obj/shape_generators.c.o: src/shape_generators.c 
ifeq (${OSMODE}, l)
	${CC} src/shape_generators.c -c -o obj/shape_generators.c.o -Iinclude/ -O2 -march=native
else
	${CC} src/shape_generators.c -c -o obj/shape_generators.c.o -Iinclude/ -O2 -march=native
endif


build/physics: obj/main.c.o obj/types.c.o obj/physics.c.o obj/matrix.c.o obj/shape_generators.c.o 
ifeq (${OSMODE}, l)
	${CC} obj/main.c.o obj/types.c.o obj/physics.c.o obj/matrix.c.o obj/shape_generators.c.o -o build/physics -lm -lSDL3
else
	${CC} obj/main.c.o obj/types.c.o obj/physics.c.o obj/matrix.c.o obj/shape_generators.c.o -o build/physics -lm -lSDL3
endif


clean:
	rm -r obj
	rm -r build
.PHONY: clean


gencommands:
	mkdir emmgtemp
ifeq (${OSMODE}, l)
	clang src/main.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/0.json -fsyntax-only
else
	clang src/main.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/0.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/types.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/1.json -fsyntax-only
else
	clang src/types.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/1.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/physics.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/2.json -fsyntax-only
else
	clang src/physics.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/2.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/matrix.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/3.json -fsyntax-only
else
	clang src/matrix.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/3.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/shape_generators.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/4.json -fsyntax-only
else
	clang src/shape_generators.c  -Iinclude/ -O2 -march=native -MJ emmgtemp/4.json -fsyntax-only
endif
# not cross platform here sad i think
	echo [ > emmgtemp/[
	echo ] > emmgtemp/]
	cat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json
	rm -r emmgtemp
