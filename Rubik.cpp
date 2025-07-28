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
#include "matrix3.h"
#include "string.h"
#include "math.h"
#include <iostream>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>
#include <regex>

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
class Piece
{
private:
    /* attributes */
    int dimensions; // The dimensions of the cube
    Vector3 position; // The position of the piece in 3D space
    std::map<std::string, Vector3> direction; // The direction of the piece, represented as a 3x3 matrix
    // The direction is a dict like {"F":Vector3(1,0,0), "B":Vector3(-1,0,0), "L":Vector3(0,1,0), "R":Vector3(0,-1,0), "U":Vector3(0,0,1), "D":Vector3(0,0,-1)}
    // where F, B, L, R, U, D are the faces of the cube.
    // The position is a Vector3 object representing the position of the piece in 3D space.
    // The dimensions is an integer representing the dimensions of the cube.
    // The rotting_matrix is a dict that maps the face to the rotation matrix in the form of {"F":("z", 1), "B":("z", 3), "L":("x", 1), "R":("x", 3), "U":("y", 1), "D":("y", 3)}
    // The crease is a dict that maps the face to the rotation matrix in the form of
    std::map<std::string, std::pair<std::string, int>> crease = {
        {"F", {"z", 1}},
        {"B", {"z", 3}},
        {"L", {"x", 1}},
        {"R", {"x", 3}},
        {"U", {"y", 1}},
        {"D", {"y", 3}}
    };
    std::map<std::string, std::pair<int, int>> position_to_face = {
        {"F", {3, 1}},
        {"B", {3, -1}},
        {"L", {1, -1}},
        {"R", {1, 1}},
        {"U", {2, 1}},
        {"D", {2, -1}}
    };
    std::map<std::string, std::function<Matrix3(float)>> rotting_matrix = {
        {"x", [](float angle) {
            return Matrix3::rotationX(angle);
        }},
        {"y", [](float angle) {
            return Matrix3::rotationY(angle);
        }},
        {"z", [](float angle) {
            return Matrix3::rotationZ(angle);
        }}
    };

     // Direction of each face, the first element is the axis, the second is the direction
      // for example, "F" is on the positive z-axis, so its direction is (3, 1)
      // and if the piece is to be rotated, it should be within the range of {diension}, {dimension}-layer_num
    /* data */
public:
 Piece(/* args */);
     Piece();
    int getDimensions() const { return dimensions; };
    Vector3 getPosition() const { return position; };
    bool identify(const std::string& face);
    bool _on_the_right_face(float coord, int layer_num, int reverse_num = 1) const {
        // Check if the piece is on the right face for a given layer.
        int n = dimensions;
        return (n + 1) / 2 - layer_num <= coord * reverse_num < (n + 1) / 2;
    };
    void _step(const std::string& face, float theta) {
        // Perform a step of rotation for the piece
        Matrix3 matrix = rotting_matrix[crease[face].first](theta);
        for (auto& dir : direction) {
            dir.second = matrix * dir.second; // Apply rotation to each direction vector
        }
        position = matrix * position; // Update position
    };
    void makeMove(const std::string& face, int t, int layer_num = 1) {
        // Perform a move on the cube (synchronously for now)
        if (crease.find(face) == crease.end()) {
            throw std::invalid_argument("Invalid face: " + face);
        }
        if (t < 1 || t > 3) {
            throw std::invalid_argument("Invalid time: " + std::to_string(t));
        }
        if (face == "x" || face == "y" || face == "z") {
            float increment = (3.14159265 / 2) * t / 10; // Assuming STEP is 10
            float theta = 0;
            if (t == 3) {
                increment = (3.14159265 / 2) * (-1) / 10; // Reverse direction for t=3
            }
            for (int i = 0; i < 10; ++i) { // Assuming STEP is 10
                _step(face, theta);
                // std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Uncomment if you want a delay
                theta += increment;
            }
        } else {
            auto it = position_to_face.find(face);
            if (it != position_to_face.end()) {
                // The piece is on a face, check if it is on the right face
                int axis = it->second.first;
                int direction_val = it->second.second;
                float coord = 0.0f;
                if (axis == 1) coord = position.x;
                else if (axis == 2) coord = position.y;
                else if (axis == 3) coord = position.z;
                if (_on_the_right_face(coord, layer_num, direction_val)) {
                    // The piece is on the right face
                    makeMove(crease[face].first, t * crease[face].second % 4, layer_num);
                } else {
                    // std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Uncomment if you want a delay
                }
            } else {
                throw std::invalid_argument("Invalid face: " + face);
            }
        }
    };
};
int Piece::getDimensions() const {
    return dimensions;
};
Vector3 Piece::getPosition() const {
    return position;
};
bool Piece::identify(const std::string& face){
    return direction.find(face) != direction.end();
};



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
class Cube {
private:
    int dimensions; // The dimensions of the cube
    std::vector<Piece> pieces; // List of pieces in the cube
    // Initialize pieces based on the dimensions of the cube
    std::vector<Piece> _initialize_pieces() {
        std::vector<Piece> pieces;
        // Add corner, edge, and center pieces based on the dimensions
        for (int x = -dimensions / 2; x <= dimensions / 2; ++x) {
            for (int y = -dimensions / 2; y <= dimensions / 2; ++y) {
                for (int z = -dimensions / 2; z <= dimensions / 2; ++z) {
                    if (std::abs(x) == dimensions / 2 || std::abs(y) == dimensions / 2 || std::abs(z) == dimensions / 2) {
                        std::map<std::string, Vector3> direction;
                        if (z == dimensions / 2) {
                            direction["F"] = Vector3(0, 0, 1);
                        }
                        if (z == -dimensions / 2) {
                            direction["B"] = Vector3(0, 0, -1);
                        }
                        if (x == -dimensions / 2) {
                            direction["L"] = Vector3(-1, 0, 0);
                        }
                        if (x == dimensions / 2) {
                            direction["R"] = Vector3(1, 0, 0);
                        }
                        if (y == dimensions / 2) {
                            direction["U"] = Vector3(0, 1, 0);
                        }
                        if (y == -dimensions / 2) {
                            direction["D"] = Vector3(0, -1, 0);
                        }
                        pieces.emplace_back(dimensions, Vector3(x, y, z), direction);
                    }
                }
            }
        }
        return pieces;
    };
public:
    Cube(int dimensions) : dimensions(dimensions), pieces(_initialize_pieces()) {};
    int getDimensions() const { return dimensions; };
    std::vector<Piece> getPieces() const { return pieces; };
    void makeMove(const std::string& face, int t, int layer_num = 1) {
        // Perform a move on the cube (synchronously for now)
        std::vector<std::function<void()>> tasks;
        for (auto& piece : pieces) {
            tasks.push_back([&piece, face, t, layer_num]() {
                piece.makeMove(face, t, layer_num);
            });
        }
        for (auto& task : tasks) {
            task(); // Execute each task
        }
    };
    std::string moveListToString(const std::vector<std::string>& move_list) {
        // Convert a list of moves to a string
        std::string move_str;
        for (const auto& move : move_list) {
            std::smatch match;
            if (std::regex_match(move, match, std::regex(R"(([FRBLUDxyz])(\d*))"))) {
                std::string face = match[1].str();
                int layer_num = match[2].length() > 0 ? std::stoi(match[2].str()) : 1;
                move_str += face + std::to_string(layer_num) + " ";
            }
        }
        return move_str.empty() ? "" : move_str.substr(0, move_str.length() - 1); // Remove trailing space
    };
};
Piece::Piece(/* args */) : dimensions(3), position(Vector3(0, 0, 0)) {
    // Initialize the piece with default values
    direction["F"] = Vector3(1, 0, 0);
    direction["B"] = Vector3(-1, 0, 0);
    direction["L"] = Vector3(0, 1, 0);
    direction["R"] = Vector3(0, -1, 0);
    direction["U"] = Vector3(0, 0, 1);
    direction["D"] = Vector3(0, 0, -1);
}
Piece::Piece() : dimensions(3), position(Vector3(0, 0, 0)) {
    // Initialize the piece with default values
    direction["F"] = Vector3(1, 0, 0);
    direction["B"] = Vector3(-1, 0, 0);
    direction["L"] = Vector3(0, 1, 0);
    direction["R"] = Vector3(0, -1, 0);
    direction["U"] = Vector3(0, 0, 1);
    direction["D"] = Vector3(0, 0, -1);
}
int main() {
    // Example usage of the Cube and Piece classes
    Cube cube(3); // Create a 3x3 Rubik's cube
    try {
        cube.makeMove("F", 1, 1); // Perform a move on the front face
        std::vector<std::string> moves = {"F1", "R2", "U3"};
        std::string move_str = cube.moveListToString(moves);
        std::cout << "Move list as string: " << move_str << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}