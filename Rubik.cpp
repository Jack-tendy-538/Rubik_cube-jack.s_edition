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
    def _on_the_right_face(self, coord, layer_num, reverse_num=1):
        """Check if the piece is on the right face for a given layer."""
        n = self.dimensions
        return (n+1) / 2 - layer_num <= coord* reverse_num < (n+1) / 2

    def _step(self, face,theta):
        matrix = self.rotting_matrix[self.crease[face][0]](theta)
        for key in self.direction:
            self.direction[key] = (matrix@self.direction[key])
        self.position = (matrix@self.position).reshape(3)
    
    async def makeMove(self, face, t, layer_num=1):
        """Perform a move on the cube asynchronously,
        t is the time to perform the move;
        layer_num is the layer to perform the move on, default is 1.
        """
        if face not in list(self.crease.keys()):
            raise ValueError(f"Invalid face: {face}")
        elif t not in [1,2,3]:
            raise ValueError(f"Invalid time: {t}")
        elif face in ["x","y","z"]:
            increment = (math.pi/2)*t/STEP
            theta = 0
            if t == 3:
                increment = (math.pi/2)*(-1)/STEP
            for i in range(STEP):
                self._step(face, theta)
                await asyncio.sleep(TICK)
                theta += increment
        else:
            if face in self.position_to_face:
                axis, direction = self.position_to_face[face]
                if self._on_the_right_face(self.position[axis-1], layer_num, direction):
                    # the piece is on the right face
                    await self.makeMove(crease[face][0], t*crease[face][1]%4, layer_num)
                else:
                    await asyncio.sleep(TICK*STEP)
            else:
                raise ValueError(f"Invalid face: {face}")

```
*/
/* And the code for corner piece is similar, with the addition of a direction attribute.
   The code for edge piece is similar, with the addition of a direction attribute.
   The code for center piece is similar, with the addition of a direction attribute.
   The code for cube is similar, with the addition of a list of pieces and a method to perform moves on the cube.
   ```python
   class Cube:
       def __init__(self, dimensions):
           self.dimensions = dimensions
           self.pieces = self._initialize_pieces()

       def _initialize_pieces(self):
           # Initialize pieces based on the dimensions of the cube
           pieces = []
           # Add corner, edge, and center pieces based on the dimensions
           # all the coords will be in the range of [-dimensions/2, dimensions/2] 
           for x in range(-self.dimensions//2, self.dimensions//2 + 1):
               for y in range(-self.dimensions//2, self.dimensions//2 + 1):
                   for z in range(-self.dimensions//2, self.dimensions//2 + 1):
                       if (abs(x) == self.dimensions//2 or abs(y) == self.dimensions//2 or abs(z) == self.dimensions//2):
                           direction = {}
                           if z == self.dimensions//2:
                               direction["F"] = np.array([0, 0, 1])
                           if z == -self.dimensions//2:
                               direction["B"] = np.array([0, 0, -1])
                           if x == -self.dimensions//2:
                               direction["L"] = np.array([-1, 0, 0])
                           if x == self.dimensions//2:
                               direction["R"] = np.array([1, 0, 0])
                           if y == self.dimensions//2:
                               direction["U"] = np.array([0, 1, 0])
                           if y == -self.dimensions//2:
                               direction["D"] = np.array([0, -1, 0])
                           pieces.append(Piece(self.dimensions, np.array([x, y, z]), direction))
           return pieces

       async def makeMove(self, face, t, layer_num=1):
            tasks = []
           for piece in self.pieces:
               tasks.append(asyncio.create_task(piece.makeMove(face, t, layer_num))
            await asyncio.gather(*tasks)

        regex = re.compile(r"([FRBLUDxyz])(\d*)")
        def moveListToString(self, move_list):
            """Convert a list of moves to a string."""
            move_str = ""
            for move in move_list:
                match = self.regex.match(move)
                if match:
                    face, layer_num = match.groups()
                    if not layer_num:
                        layer_num = 1
                    else:
                        layer_num = int(layer_num)
                    move_str += f"{face}{layer_num} "
            return move_str.strip()
   ```
*/