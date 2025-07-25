/* this program aims to model afterRubik's cube using C++.
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
*/
#include "vector3.h"
#include "matrix3.h"
#include "string.h"
#include "math.h"

/*
in Python,we call the code as following:
```python
import asyncio, math ,re
import numpy as np

class Piece:
    def __init__(self, dimensions, position, direction):
        self.dimensions = dimensions
        self.position = position
        self.direction = direction

    rotting_matrix = {
        "x": lambda angle: np.array([[1, 0, 0],
                                      [0, math.cos(angle), -math.sin(angle)],
                                      [0, math.sin(angle), math.cos(angle)]]),
        "y": lambda angle: np.array([[math.cos(angle), 0, math.sin(angle)],
                                      [0, 1, 0],
                                      [-math.sin(angle), 0, math.cos(angle)]]),
        "z": lambda angle: np.array([[math.cos(angle), -math.sin(angle), 0],
                                      [math.sin(angle), math.cos(angle), 0],
                                      [0, 0, 1]])
    }
    crease = {
        "F": ("z", 1),
        "B": ("z", 3),
        "L": ("x", 1),
        "R": ("x", 3),
        "U": ("y", 1),
        "D": ("y", 3)
    }
    position_to_face = {
        "F": (3, 1),
        "B": (3, -1),
        "L": (1, -1),
        "R": (1, 1),
        "U": (2, 1),
        "D": (2, -1)
    } # Direction of each face, the first element is the axis, the second is the direction
    # for example, "F" is on the positive z-axis, so its direction is (3, 1)
    # and if the piece is to be rotated, it should be within the range of {diension}, {dimension}-layer_num
    
    TICK = 0.02  # Time increment for each step
    STEP = 10  # Number of steps for a full rotation
    def identify(self, face):
        return self.direction.get(face, None)
    
    def _step(self, face,theta):
        matrix = self.rotting_matrix[self.crease[face][0]](theta)
        for key in self.direction:
            self.direction[key] = matrix.cross(self.direction[key])
        self.position = matrix.cross(self.position).reshape(3)
    
    async def makeMove(self, face, t, layer_num=1):
        """Perform a move on the cube asynchronously,
        t is the time to perform the move;
        layer_num is the layer to perform the move on, default is 1.
        """
        if face not in list(self.crease.keys()):
            raise ValueError(f"Invalid face: {face}")
        elif tt not in [1,2,3]:
            raise ValueError(f"Invalid time: {t}")
        elif face in ["x","y","z"]:
            increment = t/STEP
            theta = 0
            if t == 3:
                increment = -1/STEP
            for i in range(STEP):
                self._step(face, theta)
                await asyncio.sleep(TICK)
                theta += increment
        else:
            if face in self.position_to_face:
                axis, direction = self.position_to_face[face]
                if self.position[axis] * direction :
                    # the piece is on the right face
                    await self.makeMove(crease[face][0], t*crease[face][1]%4, layer_num)
                else:
                    await asyncio.sleep(TICK*STEP)
            else:
                raise ValueError(f"Invalid face: {face}")

```
*/