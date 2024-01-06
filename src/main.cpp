#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <type_traits>

class GLFWRAII {
public:
    GLFWRAII() noexcept {
        glfwInit();
    }
    ~GLFWRAII() noexcept {
        glfwTerminate();
    }
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texcoord;
};

auto readAll(const char* const fileName) noexcept {
    auto file = std::ifstream(fileName);
    file.seekg(0, std::ios::end);
    auto buffer = std::string(static_cast<size_t>(file.tellg()), '\0');
    file.seekg(0, std::ios::beg);
    buffer.assign((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
    return buffer;
}

auto compileShader(const int shaderFlag,
        const char* const srcName) noexcept {
    const auto shaderId = glCreateShader(shaderFlag);
    auto fileContent = readAll(srcName);
    const GLchar* shadersSrcs[] = { fileContent.data() };
    glShaderSource(shaderId, 1, shadersSrcs, nullptr);
    glCompileShader(shaderId);
    return shaderId;
}

auto showShaderCompileError(const GLint shaderId) noexcept {
    const auto bufferSize = 512;
    char buffer[bufferSize];
    glGetShaderInfoLog(shaderId,
            bufferSize, nullptr, buffer);
    std::cout << "Shader compiling error! Log:\n" <<
            buffer << std::endl;
}

auto loadShader(const int shaderFlag,
        const char* const srcName) noexcept {
    const auto shaderId = compileShader(shaderFlag, srcName);
    GLint success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        showShaderCompileError(shaderId);
        return static_cast<decltype(shaderId)>(-1);
    }
    return shaderId;
}

auto showProgramLinkError(const GLint programId) noexcept {
    const auto bufferSize = 512;
    char buffer[bufferSize];
    glGetProgramInfoLog(programId,
            bufferSize, nullptr, buffer);
    std::cout << "Program linking error! Log:\n" <<
            buffer << std::endl;
}

using RType = std::result_of_t<decltype(glCreateProgram)()>;

auto loadShaders() noexcept {
    // Load

    const auto vertexShaderId = loadShader(
            GL_VERTEX_SHADER, "shaders/vertexcore.glsl");
    if (vertexShaderId == -1) {
        glUseProgram(0u);
        return static_cast<RType>(-1);
    }
    const auto fragmentShaderId = loadShader(
            GL_FRAGMENT_SHADER, "shaders/fragmentcore.glsl");
    if (fragmentShaderId == -1) {
        glDeleteShader(vertexShaderId);
        glUseProgram(0u);
        return static_cast<RType>(-1);
    }

    // Program

    const auto programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);
    
    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        showProgramLinkError(programId);
    }

    // Exit

    glUseProgram(0u);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return programId;
}

auto processWindowInput(GLFWwindow* const window) noexcept {
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

auto frameBufferResizeCallback(GLFWwindow* const window,
        const int frameWidth, const int frameHeight) noexcept {
    glViewport(0, 0, frameWidth, frameHeight);
}

int main() noexcept {
    // Init GLFW

    const auto glfwRAII = GLFWRAII();

    // Create Window

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    constexpr auto width = 640u;
    constexpr auto height = 480u;

    const auto window = glfwCreateWindow(width, height,
            "OpenGL Samples", nullptr, nullptr);

    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

    // GLsizei frameWidth, frameHeight;
    // glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
    // glViewport(0, 0, frameWidth, frameHeight);

    glfwMakeContextCurrent(window);

    // Init GLEW (Needs Window)

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW init failed. Error:\n";
        return 0;
    }

    // OpenGL options

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Init shaders

    const auto programId = loadShaders();

    // Model

    // Vertices

    Vertex vertices[] = {
        glm::vec3(.0f, .5f, .0f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec2(0.f, 1.f),

        glm::vec3(-.5f, -.5f, .0f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec2(0.f, 0.f),

        glm::vec3(.5f, -.5f, .0f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec2(1.f, 0.f)
    };
    constexpr auto verticesCount = sizeof(vertices)/sizeof(vertices[0]);

    GLuint indeces[] = {
        0, 1, 2
    };
    constexpr auto indecesCount = sizeof(indeces)/sizeof(indeces[0]); 

    // VAO, VBO, EBO
    // Gen VAO and Bind

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Gen VBO and Bind and send data

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
            sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Gen EBO and Bind and send data

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(indeces), indeces, GL_STATIC_DRAW);

    // Set vertex attribute pointers and enbale (input assembly)

    // vertex_position

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    // vertex_color

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);

    // vertex_texcoord

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, texcoord)));
    glEnableVertexAttribArray(2);

    // Bind VAO 0
    glBindVertexArray(0u);

    // Main loop

    while (!glfwWindowShouldClose(window)) {
        // Process events

        glfwPollEvents();

        // Process input

        processWindowInput(window);

        // Redraw

        // Clear screen

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Use program

        glUseProgram(programId);

        // Bind vertex array object

        glBindVertexArray(vao);

        // Draw

        glDrawElements(GL_TRIANGLES, indecesCount, GL_UNSIGNED_INT, 0);

        // End draw

        glfwSwapBuffers(window);        
        glFlush(); // Rud
    }

    // End of program

    glfwDestroyWindow(window);
    return 0;
}