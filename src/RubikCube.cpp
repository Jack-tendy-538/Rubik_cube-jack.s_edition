#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

// Simple vertex and fragment shader strings (compatible with #version 330 core)
static const char* vertexShaderSrc = R"glsl(
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
)glsl";

static const char* fragmentShaderSrc = R"glsl(
#version 330 core
in vec4 vColor;
out vec4 fragColor;
void main() {
    fragColor = vColor;
}
)glsl";

struct Vertex {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 role;
    glm::vec3 color;
};

// A minimal cube representation that mirrors the provided Python getVertexData behaviour
class SimpleCube {
public:
    SimpleCube() {
        // Initialize face colors similar to the Python version
        faceColors = {
            glm::vec3(1.0f, 0.0f, 0.0f),    // front - red
            glm::vec3(1.0f, 0.5f, 0.0f),    // back - orange
            glm::vec3(0.0f, 0.0f, 1.0f),    // top - blue
            glm::vec3(0.0f, 1.0f, 0.0f),    // bottom - green
            glm::vec3(1.0f, 1.0f, 0.0f),    // right - yellow
            glm::vec3(1.0f, 1.0f, 1.0f)     // left - white
        };

        // positions for 36 vertices (6 faces * 6 vertices)
        positions = {
            // front
            glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f),
            glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f),
            // back
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f),
            glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
            // top
            glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f),
            glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f),
            // bottom
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f),
            glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
            // right
            glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f),
            glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f, -0.5f),
            // left
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f)
        };

        directions = {
            // front
            glm::vec3(0,0,1), glm::vec3(0,0,1), glm::vec3(0,0,1), glm::vec3(0,0,1), glm::vec3(0,0,1), glm::vec3(0,0,1),
            // back
            glm::vec3(0,0,-1), glm::vec3(0,0,-1), glm::vec3(0,0,-1), glm::vec3(0,0,-1), glm::vec3(0,0,-1), glm::vec3(0,0,-1),
            // top
            glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0), glm::vec3(0,1,0),
            // bottom
            glm::vec3(0,-1,0), glm::vec3(0,-1,0), glm::vec3(0,-1,0), glm::vec3(0,-1,0), glm::vec3(0,-1,0), glm::vec3(0,-1,0),
            // right
            glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(1,0,0), glm::vec3(1,0,0),
            // left
            glm::vec3(-1,0,0), glm::vec3(-1,0,0), glm::vec3(-1,0,0), glm::vec3(-1,0,0), glm::vec3(-1,0,0), glm::vec3(-1,0,0)
        };
    }

    void getVertexData(std::vector<Vertex>& out) {
        out.clear();
        out.reserve(36);
        for (int i = 0; i < 36; ++i) {
            Vertex v;
            v.position = positions[i];
            v.direction = directions[i];
            v.role = glm::vec3(0.0f);
            int faceIndex = i / 6;
            v.color = faceColors[faceIndex];
            out.push_back(v);
        }
    }

    void makeMove(const std::string& move) {
        // Placeholder: just print the move. Implement rotation logic here later.
        std::cout << "Making move: " << move << std::endl;
    }

private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> directions;
    std::vector<glm::vec3> faceColors;
};

// Helper: compile shader and link program
static GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[512];
        glGetShaderInfoLog(shader, 512, nullptr, buf);
        std::cerr << "Shader compile error: " << buf << std::endl;
    }
    return shader;
}

static GLuint createProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[512];
        glGetProgramInfoLog(prog, 512, nullptr, buf);
        std::cerr << "Program link error: " << buf << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// Global app state for simplicity
struct App {
    GLFWwindow* window = nullptr;
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    SimpleCube cube;
    float width = 1.0f;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    std::vector<Vertex> vertexData;
} app;

void initGL() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    app.window = glfwCreateWindow(800, 800, "Rubik's Cube (C++)", NULL, NULL);
    if (!app.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(-1);
    }
    glfwMakeContextCurrent(app.window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        std::exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    app.program = createProgram();

    // Setup VAO/VBO
    glGenVertexArrays(1, &app.vao);
    glGenBuffers(1, &app.vbo);
    glBindVertexArray(app.vao);
    glBindBuffer(GL_ARRAY_BUFFER, app.vbo);
    glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // direction
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, direction));
    // role
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, role));
    // color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);

    // default matrices
    app.projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    app.view = glm::lookAt(glm::vec3(3,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    app.model = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(1,0,0));
}

void updateVertexBuffer() {
    app.cube.getVertexData(app.vertexData);
    glBindBuffer(GL_ARRAY_BUFFER, app.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, app.vertexData.size() * sizeof(Vertex), app.vertexData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render() {
    glUseProgram(app.program);
    GLint projLoc = glGetUniformLocation(app.program, "projection");
    GLint viewLoc = glGetUniformLocation(app.program, "view");
    GLint modelLoc = glGetUniformLocation(app.program, "model");
    GLint widthLoc = glGetUniformLocation(app.program, "width");

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(app.projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(app.view));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(app.model));
    glUniform1f(widthLoc, app.width);

    updateVertexBuffer();

    glBindVertexArray(app.vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)app.vertexData.size());
    glBindVertexArray(0);
    glUseProgram(0);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    bool clockwise = !(mods & GLFW_MOD_SHIFT);
    std::string move;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    else if (key == GLFW_KEY_U) move = "U";
    else if (key == GLFW_KEY_D) move = "D";
    else if (key == GLFW_KEY_L) move = "L";
    else if (key == GLFW_KEY_R) move = "R";
    else if (key == GLFW_KEY_F) move = "F";
    else if (key == GLFW_KEY_B) move = "B";
    else if (key == GLFW_KEY_X) move = "X";
    else if (key == GLFW_KEY_Y) move = "Y";
    else if (key == GLFW_KEY_Z) move = "Z";

    if (!move.empty()) {
        if (!clockwise) move += "'";
        app.cube.makeMove(move);
    }
}

int main() {
    initGL();
    glfwSetKeyCallback(app.window, keyCallback);

    while (!glfwWindowShouldClose(app.window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render();
        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    glfwDestroyWindow(app.window);
    glfwTerminate();
    return 0;
}