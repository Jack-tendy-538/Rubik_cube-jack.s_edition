import OpenGL
from OpenGL.GL import *
from OpenGL.GL import shaders
import numpy as np

import sys, os, threading, time, math
import ctypes.CDLL
import ctypes.util

# Load global constants
cubelib = ctypes.CDLL(ctypes.util.find_library('Rubik'))
# Ensure the library is loaded correctly
if not cubelib:
    raise RuntimeError("Could not load the Rubik library. Ensure it is installed and accessible.")
strVS = """
#version 330 core
// Vertex shader for Rubik's Cube
// inputs position, direction, role(A symbol to finite which vertex this is), and color
layout(location = 0) in vec3 position;
in vec3 direction;
in vec3 role;
in vec3 color;
uniform mat4 uProjection;
uniform mat4 uMVP;
uniform int width;
out vec4 vColor;
void main(){
    vec3 faceCenter = position + direction * 0.5;
    vec3 transformedRole = role * direction;
    gl_Position = uMVP * vec4((faceCenter + transformedRole) * width, 1.0);
    vColor = vec4(color, 1.0);
}
"""
strFS = """
#version 330 core
// Fragment shader for Rubik's Cube
in vec4 vColor;
out vec4 fragColor;
void main(){
    fragColor = vColor;
}
"""
# Load the C functions