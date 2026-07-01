all: obj/ build/ obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/fluid.c.o build/fluid obj/2d_3d_scene.c.o build/2d_3d_scene 
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

obj/types.c.o: engine/src/types.c 
ifeq (${OSMODE}, l)
	${CC} engine/src/types.c -c -o obj/types.c.o -Iengine/include/ -O3 -march=native
else
	${CC} engine/src/types.c -c -o obj/types.c.o -Iengine/include/ -O3 -march=native
endif


obj/physics.c.o: engine/src/physics.c 
ifeq (${OSMODE}, l)
	${CC} engine/src/physics.c -c -o obj/physics.c.o -Iengine/include/ -O3 -march=native
else
	${CC} engine/src/physics.c -c -o obj/physics.c.o -Iengine/include/ -O3 -march=native
endif


obj/shape_generators.c.o: engine/src/shape_generators.c 
ifeq (${OSMODE}, l)
	${CC} engine/src/shape_generators.c -c -o obj/shape_generators.c.o -Iengine/include/ -O3 -march=native
else
	${CC} engine/src/shape_generators.c -c -o obj/shape_generators.c.o -Iengine/include/ -O3 -march=native
endif


obj/helpers.c.o: demos/helpers/helpers.c 
ifeq (${OSMODE}, l)
	${CC} demos/helpers/helpers.c -c -o obj/helpers.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
else
	${CC} demos/helpers/helpers.c -c -o obj/helpers.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
endif


obj/matrix.c.o: demos/helpers/matrix.c 
ifeq (${OSMODE}, l)
	${CC} demos/helpers/matrix.c -c -o obj/matrix.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
else
	${CC} demos/helpers/matrix.c -c -o obj/matrix.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
endif


obj/fluid.c.o: demos/fluid.c 
ifeq (${OSMODE}, l)
	${CC} demos/fluid.c -c -o obj/fluid.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
else
	${CC} demos/fluid.c -c -o obj/fluid.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
endif


build/fluid: obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/fluid.c.o 
ifeq (${OSMODE}, l)
	${CC} obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/fluid.c.o -o build/fluid -lm -lSDL3
else
	${CC} obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/fluid.c.o -o build/fluid -lm -lSDL3
endif


obj/2d_3d_scene.c.o: demos/2d_3d_scene.c 
ifeq (${OSMODE}, l)
	${CC} demos/2d_3d_scene.c -c -o obj/2d_3d_scene.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
else
	${CC} demos/2d_3d_scene.c -c -o obj/2d_3d_scene.c.o -Iengine/include/ -O3 -march=native -Idemos/helpers/
endif


build/2d_3d_scene: obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/2d_3d_scene.c.o 
ifeq (${OSMODE}, l)
	${CC} obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/2d_3d_scene.c.o -o build/2d_3d_scene -lm -lSDL3
else
	${CC} obj/types.c.o obj/physics.c.o obj/shape_generators.c.o obj/helpers.c.o obj/matrix.c.o obj/2d_3d_scene.c.o -o build/2d_3d_scene -lm -lSDL3
endif


clean:
	rm -r obj
	rm -r build
.PHONY: clean


gencommands:
	mkdir emmgtemp
ifeq (${OSMODE}, l)
	clang engine/src/types.c  -Iengine/include/ -O3 -march=native -MJ emmgtemp/0.json -fsyntax-only
else
	clang engine/src/types.c  -Iengine/include/ -O3 -march=native -MJ emmgtemp/0.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang engine/src/physics.c  -Iengine/include/ -O3 -march=native -MJ emmgtemp/1.json -fsyntax-only
else
	clang engine/src/physics.c  -Iengine/include/ -O3 -march=native -MJ emmgtemp/1.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang engine/src/shape_generators.c  -Iengine/include/ -O3 -march=native -MJ emmgtemp/2.json -fsyntax-only
else
	clang engine/src/shape_generators.c  -Iengine/include/ -O3 -march=native -MJ emmgtemp/2.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang demos/helpers/helpers.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/3.json -fsyntax-only
else
	clang demos/helpers/helpers.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/3.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang demos/helpers/matrix.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/4.json -fsyntax-only
else
	clang demos/helpers/matrix.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/4.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang demos/fluid.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/5.json -fsyntax-only
else
	clang demos/fluid.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/5.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang demos/2d_3d_scene.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/6.json -fsyntax-only
else
	clang demos/2d_3d_scene.c  -Iengine/include/ -O3 -march=native -Idemos/helpers/ -MJ emmgtemp/6.json -fsyntax-only
endif
# not cross platform here sad i think
	echo [ > emmgtemp/[
	echo ] > emmgtemp/]
	cat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json
	rm -r emmgtemp
