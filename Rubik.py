import OpenGL 
from OpenGL.GL import *
from OpenGL.GL import shaders 
import numpy as np
import glutils
import glfw
import sys, os, threading, time, math
import ctypes
import ctypes.util

# 尝试加载C库
try:
    cubelib = ctypes.CDLL(ctypes.util.find_library('Rubik'))
    CUBE_LIB_AVAILABLE = True
except:
    print("Warning: Rubik C library not found, using Python implementation")
    cubelib = None
    CUBE_LIB_AVAILABLE = False

# 着色器代码
strVS = """
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 direction;
layout(location = 2) in vec3 role;
layout(location = 3) in vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float width;

out vec4 vColor;

void main() {
    vec3 faceCenter = position + direction * 0.5;
    vec3 transformedRole = role * direction;
    vec4 worldPos = model * vec4(faceCenter + transformedRole * width, 1.0);
    gl_Position = projection * view * worldPos;
    vColor = vec4(color, 1.0);
}
"""

strFS = """
#version 330 core
in vec4 vColor;
out vec4 fragColor;
void main() {
    fragColor = vColor;
}
"""

class CubeRender:
    def __init__(self):
        if CUBE_LIB_AVAILABLE:
            self.cube = cubelib.createCube()
        else:
            # 纯Python实现
            self.cube = PythonCube()
            
        self.width = 1.0
        self.angleX = 30
        self.program = glutils.loadShaders(strVS, strFS)
        
        # 获取uniform位置
        self.projLoc = glGetUniformLocation(self.program, "projection")
        self.viewLoc = glGetUniformLocation(self.program, "view")
        self.modelLoc = glGetUniformLocation(self.program, "model")
        self.widthLoc = glGetUniformLocation(self.program, "width")
        
        # 设置矩阵
        self.projection = glutils.perspective(45, 1, 0.1, 100)
        self.view = glutils.lookAt(np.array([3, 3, 3]), np.array([0, 0, 0]), np.array([0, 1, 0]))
        self.model = glutils.rotate(np.radians(self.angleX), np.array([1, 0, 0]))
        
        # 顶点数据 - 每个面6个顶点，6个面共36个顶点
        num_vertices = 36
        self.VertexData = np.zeros(num_vertices, [
            ("position", np.float32, 3),
            ("direction", np.float32, 3),
            ("role", np.float32, 3),
            ("color", np.float32, 3)
        ])
        
        # 设置VAO和VBO
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        
        self.vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, self.VertexData.nbytes, self.VertexData, GL_DYNAMIC_DRAW)
        
        # 设置顶点属性
        attr_names = ["position", "direction", "role", "color"]
        offset = 0
        for i, name in enumerate(attr_names):
            glEnableVertexAttribArray(i)
            glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 
                                 self.VertexData.strides[0], ctypes.c_void_p(offset))
            offset += self.VertexData.dtype[name].itemsize
            
        glBindVertexArray(0)
        glBindBuffer(GL_ARRAY_BUFFER, 0)

    def render(self):
        glUseProgram(self.program)
        
        # 传递uniform值
        glUniformMatrix4fv(self.projLoc, 1, GL_FALSE, self.projection)
        glUniformMatrix4fv(self.viewLoc, 1, GL_FALSE, self.view)
        glUniformMatrix4fv(self.modelLoc, 1, GL_FALSE, self.model)
        glUniform1f(self.widthLoc, self.width)
        
        # 获取顶点数据
        if CUBE_LIB_AVAILABLE:
            cubelib.getVertexData(self.cube, self.VertexData.ctypes.data_as(ctypes.POINTER(ctypes.c_float)))
        else:
            self.cube.getVertexData(self.VertexData)
        
        # 更新VBO
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferSubData(GL_ARRAY_BUFFER, 0, self.VertexData.nbytes, self.VertexData)
        
        # 渲染
        glBindVertexArray(self.vao)
        glDrawArrays(GL_TRIANGLES, 0, len(self.VertexData))
        
        # 清理
        glBindVertexArray(0)
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glUseProgram(0)

    def rotate(self, face, clockwise=True):
        move_str = face if clockwise else face + "'"
        
        if CUBE_LIB_AVAILABLE:
            cubelib.makeMove(self.cube, move_str.encode('utf-8'))
        else:
            self.cube.makeMove(move_str)

    def turn(self, axis, clockwise=True):
        move_str = axis if clockwise else axis + "'"
        
        if CUBE_LIB_AVAILABLE:
            cubelib.makeMove(self.cube, move_str.encode('utf-8'))
        else:
            self.cube.makeMove(move_str)

# 纯Python魔方实现（备选）
class PythonCube:
    def __init__(self):
        # 简化的魔方状态表示
        self.state = np.ones((6, 3, 3), dtype=np.int32)
        # 为每个面分配不同的颜色
        for i in range(6):
            self.state[i, :, :] = i + 1
            
    def getVertexData(self, vertex_data):
        # 简化实现：生成固定颜色的立方体
        # 实际实现应根据魔方状态生成顶点数据
        positions = [
            # 前面
            [-0.5, -0.5, 0.5], [0.5, -0.5, 0.5], [0.5, 0.5, 0.5],
            [0.5, 0.5, 0.5], [-0.5, 0.5, 0.5], [-0.5, -0.5, 0.5],
            # 后面
            [-0.5, -0.5, -0.5], [-0.5, 0.5, -0.5], [0.5, 0.5, -0.5],
            [0.5, 0.5, -0.5], [0.5, -0.5, -0.5], [-0.5, -0.5, -0.5],
            # 上面
            [-0.5, 0.5, -0.5], [-0.5, 0.5, 0.5], [0.5, 0.5, 0.5],
            [0.5, 0.5, 0.5], [0.5, 0.5, -0.5], [-0.5, 0.5, -0.5],
            # 下面
            [-0.5, -0.5, -0.5], [0.5, -0.5, -0.5], [0.5, -0.5, 0.5],
            [0.5, -0.5, 0.5], [-0.5, -0.5, 0.5], [-0.5, -0.5, -0.5],
            # 右面
            [0.5, -0.5, -0.5], [0.5, 0.5, -0.5], [0.5, 0.5, 0.5],
            [0.5, 0.5, 0.5], [0.5, -0.5, 0.5], [0.5, -0.5, -0.5],
            # 左面
            [-0.5, -0.5, -0.5], [-0.5, -0.5, 0.5], [-0.5, 0.5, 0.5],
            [-0.5, 0.5, 0.5], [-0.5, 0.5, -0.5], [-0.5, -0.5, -0.5]
        ]
        
        # 设置面方向（法线）
        directions = [
            [0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1],  # 前面
            [0, 0, -1], [0, 0, -1], [0, 0, -1], [0, 0, -1], [0, 0, -1], [0, 0, -1],  # 后面
            [0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0], [0, 1, 0],  # 上面
            [0, -1, 0], [0, -1, 0], [0, -1, 0], [0, -1, 0], [0, -1, 0], [0, -1, 0],  # 下面
            [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0],  # 右面
            [-1, 0, 0], [-1, 0, 0], [-1, 0, 0], [-1, 0, 0], [-1, 0, 0], [-1, 0, 0]   # 左面
        ]
        
        # 设置角色（用于着色器中的计算）
        roles = [[0, 0, 0]] * 36
        
        # 设置颜色（每个面不同颜色）
        colors = []
        face_colors = [
            [1, 0, 0],    # 红 - 前面
            [1, 0.5, 0],  # 橙 - 后面
            [0, 0, 1],    # 蓝 - 上面
            [0, 1, 0],    # 绿 - 下面
            [1, 1, 0],    # 黄 - 右面
            [1, 1, 1]     # 白 - 左面
        ]
        
        for i in range(6):
            colors.extend([face_colors[i]] * 6)
        
        # 填充顶点数据
        for i in range(36):
            vertex_data[i]['position'] = positions[i]
            vertex_data[i]['direction'] = directions[i]
            vertex_data[i]['role'] = roles[i]
            vertex_data[i]['color'] = colors[i]
            
    def makeMove(self, move_str):
        # 简化实现：打印移动信息
        print(f"Making move: {move_str}")
        # 实际实现应更新魔方状态

# CubeApp类保持不变，但更新键盘回调
class CubeApp:
    def __init__(self):
        if not glfw.init():
            print("Failed to initialize GLFW")
            sys.exit(-1)
            
        self.window = glfw.create_window(800, 800, "Rubik's Cube", None, None)
        if not self.window:
            print("Failed to create GLFW window")
            glfw.terminate()
            sys.exit(-1)
            
        glfw.make_context_current(self.window)
        glEnable(GL_DEPTH_TEST)
        glClearColor(0.2, 0.3, 0.3, 1.0)
        
        self.cubeRender = CubeRender()
        glfw.set_key_callback(self.window, self.on_key)

    def on_key(self, window, key, scancode, action, mods):
        if action == glfw.PRESS:
            clockwise = not (mods & glfw.MOD_SHIFT)  # Shift键表示逆时针
            
            if key == glfw.KEY_ESCAPE:
                glfw.set_window_should_close(self.window, True)
            elif key == glfw.KEY_U:
                self.cubeRender.rotate('U', clockwise)
            elif key == glfw.KEY_D:
                self.cubeRender.rotate('D', clockwise)
            elif key == glfw.KEY_L:
                self.cubeRender.rotate('L', clockwise)
            elif key == glfw.KEY_R:
                self.cubeRender.rotate('R', clockwise)
            elif key == glfw.KEY_F:
                self.cubeRender.rotate('F', clockwise)
            elif key == glfw.KEY_B:
                self.cubeRender.rotate('B', clockwise)
            elif key == glfw.KEY_X:
                self.cubeRender.turn('X', clockwise)
            elif key == glfw.KEY_Y:
                self.cubeRender.turn('Y', clockwise)
            elif key == glfw.KEY_Z:
                self.cubeRender.turn('Z', clockwise)

    def run(self):
        while not glfw.window_should_close(self.window):
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            self.cubeRender.render()
            glfw.swap_buffers(self.window)
            glfw.poll_events()
            
        glfw.terminate()

if __name__ == "__main__":
    app = CubeApp()
    app.run()