// Statements to force the use of NVIDIA or AMD discrete GPU
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;       // For NVIDIA
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;         // For AMD
}


#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Game.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"

// GLFW function declarations
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void calculateFPS();

// The Width of the screen
const unsigned int SCREEN_WIDTH = 2048;
// The Height of the screen
const unsigned int SCREEN_HEIGHT = 1152;

static unsigned int fps = 0;
static unsigned int frameCount = 0;
static double previousTime = 0;
static double timeInterval = 0;

static Game GLMan(SCREEN_WIDTH, SCREEN_HEIGHT);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GL-Man", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << '\n';
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // OpenGL configuration
    // --------------------
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // initialize game
    // ---------------
    LoggerManager::Init();
    GLMan.Init();

    // deltaTime variables
    // -------------------
    double deltaTime = 0.0f;
    double lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        // calculate delta time
        // --------------------
        const double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        calculateFPS();
        //LoggerManager::LogInfo("FPS: {}", fps);

        // manage user input
        // -----------------
        GLMan.ProcessInput(deltaTime);

        // update game state
        // -----------------
        GLMan.Update(deltaTime);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLMan.Render(deltaTime);

        glfwSwapBuffers(window);
    }

    // delete all resources as loaded using the resource manager
    // ---------------------------------------------------------
    ResourceManager::Clear();
    LoggerManager::Shutdown();

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, const int key, int scancode, const int action, int mode) {
    // when a user presses the escape key, we set the WindowShouldClose property to true, closing the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            GLMan.keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            GLMan.keys[key] = false;
            GLMan.keysProcessed[key] = false;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void calculateFPS() {
    frameCount++;
    const double currentTime = glfwGetTime();
    timeInterval = currentTime - previousTime;

    // If a second has passed, update the fps variable
    if (timeInterval > 1.0f) {
        fps = frameCount;
        previousTime = currentTime;
        frameCount = 0;
    }
}