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