// This file is part of the Rubik's Cube C++ library.
// It provides a C interface to the Cube class for use in other languages.
// therefore, we should rewrite sth
extern "C"{
    #include "Rubik.cpp"
    Cube *createCube(int size, int seed) {
        return new Cube(size, seed);
    }
    void makeMove(Cube *cube) {
        cube->makeMove();
    }
    void resetCube(Cube *cube) {
        cube->reset();
    }
    void destroyCube(Cube *cube) {
        delete cube;
    }
    
}