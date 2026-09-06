# FPE(eee)
Funky Physics Engine(eee) is a portable, fast, and flexible position-based physics engine.

![Cloth showcase](cloth.gif)

## Try it
Try some demos of the project [here](https://duckdood.github.io/physics_demo) (cloth cutter demo recommended)

## Features
* Physics using second-order verlet integration
* 2D and 3D environments
* Distance and spring constraints
* Spatial partitioning for handling thousands of balls at once
* Dynamic walls made of particles (penetration constraints)
* Written fully in C for portability, interoperability, and speed.

## Building locally (Linux and MinGW)
* ### Dependencies
* A C compiler (I've tested with GCC and Clang)
* Make
* SDL3
* ### Building
`git clone https://github.com/duckdood/fpeeee && cd fpeeee && make`\
This will put demos in a the build/ directory for you to run.

## Credits
* [Advanced character physics](https://www.cs.cmu.edu/afs/cs/academic/class/15462-s13/www/lec_slides/Jakobsen.pdf) by Thomas Jakobsen for verlet integration
* [Position based dynamics](https://www.cs.toronto.edu/~jacobson/seminar/mueller-et-al-2007.pdf) by Matthias Müller et al. for more complex constraints
* [Coding a Physics Engine from scratch!](https://www.youtube.com/watch?v=nXrEX6j-Mws) by Zanzlanz for an simple instroduction to ball based physics simulations
* [Stack Overflow](https://stackoverflow.com/) in general
* And finally, [Stardance](https://stardance.hackclub.com/) for motivating me to continue this project!

## AI
I used some AI as a fancy search engine to find papers and stuff, and double checking stuff in my code if there is a blatant but hard to spot mistake, but I always take its responses with a grain of salt and I do absolutely NO copy pasting straight from it.
