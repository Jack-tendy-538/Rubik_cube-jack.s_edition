# Rubik's cube(JT's edition)
With the number of classmates who plays Cube scaling up, we concerns more about questions like
- how many times do the formula `RULD` take to get the Cube recovered?
- is there an effective way to recobver a Cube?
Of course, these can be solved by experiments, and it definitely takes lots of time.
What if automate it with computers?This repo might give you a different answer.
## doc
 this program aims to model after Rubik's cube using C++.
    with features including:
    - inifinitive dimensions
    - fine tuning of the GUI using OpenGL

terminology:
- cube: a single Rubik's cube
- piece: a single piece of the cube, which can be a corner or an edge, 
        or a center piece, with attributes like dimensions, position, and direction(a dict like{"F":vector3(1,0,0)...}).
- makeMove: a function to make a move on the cube (A basic move using async) with concepts as following:
- - rotting_matrix: a function that generates a matrix that represents the rotation of the cube(Only for formulars like x,y,z)
- - crease: When we call F on the cube, we'll rotate the front face like performing a z rotation, and
        crease is a dict that maps the face to the rotation matrix in the form of {"F":("z", 1), "B":("z", 3)...}
- - identify: a tool to tell if the piece is on the right face.

- move: a function to make a move on the cube, which will call makeMove with the right parameters.
- moveList: a list of moves to be performed on the cube, which can be used to perform a sequence of moves.
- moveListToString: a function to convert the move list to a string, which
                can be used to display the moves in a human-readable format or to save the moves to a file.

## implete status
I'm just a student, rather than prof developer.
This means *I'll never finish it*!