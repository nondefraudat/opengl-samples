#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

class GLFWRAII {
public:
    GLFWRAII() noexcept {
        glfwInit();
    }
    ~GLFWRAII() noexcept {
        glfwTerminate();
    }
};

int main() noexcept {
    // Init GLFW

    const auto glfwRAII = GLFWRAII();

    // Create Window

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    constexpr auto width = 640u;
    constexpr auto height = 480u;

    const auto window = glfwCreateWindow(width, height, "OpenGL Samples", nullptr, nullptr);

    GLsizei frameWidth, frameHeight;
    glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
    glViewport(0, 0, frameWidth, frameHeight);

    glfwMakeContextCurrent(window);

    // Init GLEW (Needs Window)

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW init failed. Error:\n";
        return 0;
    }

    // Main loop

    while (!glfwWindowShouldClose(window)) {
        // Process events

        glfwPollEvents();

        // Update

        // Redraw

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glfwSwapBuffers(window);        
        glFlush(); // Rud
    }

    // End of program

    return 0;
}