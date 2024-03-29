#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext.hpp>
#include <SOIL2/SOIL2.h>

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

static auto readAll(const char* const fileName) noexcept {
    auto file = std::ifstream(fileName);
    file.seekg(0, std::ios::end);
    auto buffer = std::string(static_cast<size_t>(file.tellg()), '\0');
    file.seekg(0, std::ios::beg);
    buffer.assign((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
    return buffer;
}

static auto compileShader(const int shaderFlag,
        const char* const srcName) noexcept {
    const auto shaderId = glCreateShader(shaderFlag);
    auto fileContent = readAll(srcName);
    const GLchar* shadersSrcs[] = { fileContent.data() };
    glShaderSource(shaderId, 1, shadersSrcs, nullptr);
    glCompileShader(shaderId);
    return shaderId;
}

static auto showShaderCompileError(const GLint shaderId) noexcept {
    const auto bufferSize = 512;
    char buffer[bufferSize];
    glGetShaderInfoLog(shaderId,
            bufferSize, nullptr, buffer);
    std::cout << "Shader compiling error! Log:\n" <<
            buffer << std::endl;
}

static auto loadShader(const int shaderFlag,
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

static auto showProgramLinkError(const GLint programId) noexcept {
    const auto bufferSize = 512;
    char buffer[bufferSize];
    glGetProgramInfoLog(programId,
            bufferSize, nullptr, buffer);
    std::cout << "Program linking error! Log:\n" <<
            buffer << std::endl;
}

using RType = std::result_of_t<decltype(glCreateProgram)()>;

static auto loadShaders() noexcept {
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

static auto processWindowInput(GLFWwindow* const window) noexcept {
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static auto frameBufferResizeCallback(GLFWwindow* const window,
        const int frameWidth, const int frameHeight) noexcept {
    glViewport(0, 0, frameWidth, frameHeight);
}

static auto loadTexture(const char* const imageName) noexcept {
    auto imageWidth = 0;
    auto imageHeight = 0;
    const auto image = SOIL_load_image(imageName,
            &imageWidth, &imageHeight, nullptr, SOIL_LOAD_RGBA);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if (image) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Texture \"" << imageName << "\" loading failed\n";
    }

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return texture;
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

    auto frameBufferWidth = 0;
    auto frameBufferHeight = 0;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

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
        glm::vec3(.5f, -.5f, .0f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec2(1.f, 0.f),

        glm::vec3(.5f, .5f, .0f),
        glm::vec3(1.f, 1.f, 0.f),
        glm::vec2(1.f, 1.f),

        glm::vec3(-.5f, .5f, .0f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec2(0.f, 1.f),

        glm::vec3(-.5f, -.5f, .0f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec2(0.f, 0.f)
    };
    constexpr auto verticesCount = sizeof(vertices)/sizeof(vertices[0]);

    GLuint indeces[] = {
        0, 1, 2,
        0, 2, 3
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

    // Texture init
    
    const auto ilufanTexture = loadTexture("rsc/ilufan.png");
    const auto boxTexture = loadTexture("rsc/box.png");

    // Init metrics

    auto modelMatrix = glm::mat4(1.f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f));
    //modelMatrix = glm::rotate(modelMatrix,
    //        glm::radians(3.f), glm::vec3(1.f, 0.f, 0.f));
    //modelMatrix = glm::rotate(modelMatrix,
    //        glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
    //modelMatrix = glm::rotate(modelMatrix,
    //        glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(.05f));

    auto camPosition = glm::vec3(0.f, 0.f, 1.f);
    auto worldUp = glm::vec3(0.f, 1.f, 0.f);
    auto camFront = glm::vec3(0.f, 0.f, -1.f);
    auto viewMatrix = glm::mat4(1.f);
    
    viewMatrix = glm::lookAt(camPosition, camPosition + camFront, worldUp);

    const auto fov = 90.f;
    const auto nearPlane = .1f;
    const auto farPlane = 5000.f;
    auto projectionMatrix = glm::perspective(glm::radians(fov),
            static_cast<float>(frameBufferWidth)/frameBufferHeight,
            nearPlane, farPlane);

    glUseProgram(programId);

    glUniformMatrix4fv(glGetUniformLocation(programId, "modelMatrix"),
            1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(programId, "viewMatrix"),
            1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(programId, "projectionMatrix"),
            1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glUseProgram(0);

    // Main loop

    while (!glfwWindowShouldClose(window)) {
        // Process events

        glfwPollEvents();

        // Process input

        processWindowInput(window);

        // Redraw

        // Clear screen

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Use program

        glUseProgram(programId);


        // Update uniforms

        const auto ilufanTextureLocation = glGetUniformLocation(
                programId, "ilufanTexture");
        glUniform1i(ilufanTextureLocation, 0);

        const auto boxTextureLocation = glGetUniformLocation(
            programId, "boxTexture");
        glUniform1i(boxTextureLocation, 1);

        // Move, rotate, scale matrix

        modelMatrix = glm::rotate(modelMatrix,
                glm::radians(1.f), glm::vec3(0.f, 1.f, 0.f));
        //modelMatrix = glm::rotate(modelMatrix,
        //        glm::radians(.5f), glm::vec3(0.f, 1.f, 0.f));
        //modelMatrix = glm::rotate(modelMatrix,
        //        glm::radians(.1f), glm::vec3(0.f, 0.f, 1.f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.001f));
        const auto unipos = glGetUniformLocation(programId, "modelMatrix");
        glUniformMatrix4fv(unipos, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

        projectionMatrix = glm::perspective(glm::radians(fov),
                static_cast<float>(frameBufferWidth) / frameBufferHeight,
                nearPlane, farPlane);

        glUniformMatrix4fv(glGetUniformLocation(programId, "projectionMatrix"),
                1, GL_FALSE, glm::value_ptr(projectionMatrix));

        // Activate texture

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ilufanTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boxTexture);

        // Bind vertex array object

        glBindVertexArray(vao);

        // Draw

        // glDrawArrays(GL_TRIANGLES, 0, verticesCount);
        glDrawElements(GL_TRIANGLES, indecesCount, GL_UNSIGNED_INT, 0);

        // End draw

        glfwSwapBuffers(window);        
        glFlush(); // Rud
    }

    // End of program

    glfwDestroyWindow(window);
    return 0;
}