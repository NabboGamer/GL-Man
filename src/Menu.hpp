#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Filesystem.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "Texture2D.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Menu {

public:

	explicit Menu(GLFWwindow* window, unsigned int width, unsigned int height, bool& showGame);
	~Menu();

	void Init();
	void Render(double deltaTime);

private:

	GLFWwindow*   window;
	unsigned int  width;
	unsigned int  height;
	bool&         showGame;

	GLFWcursor* createCustomCursor(const char* imagePath, int hotspotX, int hotspotY);

};